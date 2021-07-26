/**
 * @file cjson.c
 * @author Quan.Dashuai
 * @copyright GNU AFFERO GENERAL PUBLIC LICENSE Version3
 */

#include "cjson.h"

#include <stdlib.h>
#include <math.h>
#include <errno.h>

#define CJSON_PARSE_STACK_INIT_SIZE 256

#define ISDIGIT(ch)     ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')
#define PUTC(c, ch) \
  do { \
    *(char*)cjson_context_push(c, sizeof(char)) = (ch); \
  } while(0)

/**
 * @brief The context of parsing json.
 */
typedef struct {
  const char *json; /* The original json string */
  char *stack;
  size_t size;      /* The size of the stack */
  size_t top;       /* The index of the stack top */
} cjson_context;

/**
 * @brief Returns a pointer to a free location。
 * 
 * @note
 * The caller can use returned pointer as the destination address of the string copy, 
 * or dereference the pointer to insert a single character.
 */
static void* cjson_context_push(cjson_context *context, size_t size) {
  void *ret;
  if (context->top + size >= context->size) {
    if (context->size == 0) {
      context->size = CJSON_PARSE_STACK_INIT_SIZE;
    }
    while (context->top + size >= context->size) {
      context->size += context->size >> 1;
    }
    context->stack = (char*)realloc(context->stack, context->size);
  }
  ret = context->stack + context->top;
  context->top += size;
  return ret;
}

static void* cjson_context_pop(cjson_context *context, size_t size) {
  return context->stack + (context->top -= size);
}

static int cjson_parse_str(cjson_context *context, cjson_value *value, const char *str, cjson_type type) {
  size_t str_len = strlen(str);
  if (strlen(context->json) < str_len) {
    return CJSON_PARSE_INVALID_VALUE;
  }
  if (!strncmp(context->json, str, str_len)) {
    value->type = type;
    context->json += str_len;
    return CJSON_PARSE_OK;
  } else {
    return CJSON_PARSE_INVALID_VALUE;
  }
}

static void cjson_parse_whitespace(cjson_context *context) {
  const char *p = context->json;
  while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
    p++;
  }
  context->json = p;
}

static int cjson_parse_number(cjson_context *context, cjson_value *value) {
  const char* p = context->json;
  if (*p == '-') p++;
  if (*p == '0') p++;
  else {
    if (!ISDIGIT1TO9(*p)) return CJSON_PARSE_INVALID_VALUE;
    for (p++; ISDIGIT(*p); p++);
  }
  if (*p == '.') {
    p++;
    if (!ISDIGIT(*p)) return CJSON_PARSE_INVALID_VALUE;
    for (p++; ISDIGIT(*p); p++);
  }
  if (*p == 'e' || *p == 'E') {
    p++;
    if (*p == '+' || *p == '-') p++;
    if (!ISDIGIT(*p)) return CJSON_PARSE_INVALID_VALUE;
    for (p++; ISDIGIT(*p); p++);
  }
  value->number = strtod(context->json, NULL);
  if (errno == ERANGE && (value->number == HUGE_VAL || value->number == -HUGE_VAL)) {
    return CJSON_PARSE_NUMBER_TOO_BIG;
  }
  value->type = CJSON_NUMBER;
  context->json = p;
  return CJSON_PARSE_OK;
}

void cjson_set_string(cjson_value *value, const char *src, size_t len) {
  cjson_free_value(value);
  value->str = (char*)malloc(len + 1);
  memcpy(value->str, src, len);
  value->str[len] = '\0';
  value->len = len;
  value->type = CJSON_STRING;
}

static int cjson_parse_string(cjson_context *context, cjson_value *value) {
  size_t head = context->top, len;
  const char* p;
  p = context->json + 1;
  while(1) {
    char ch = *p++;
    switch (ch) {
      case '"': {
        len = context->top - head;
        cjson_set_string(value, (const char*)cjson_context_pop(context, len), len);
        context->json = p;
        return CJSON_PARSE_OK;
      }
      case '\\': {
        switch (*p++) {
          case '"':  PUTC(context, '"'); break;
          case '\\': PUTC(context, '\\'); break;
          case '/':  PUTC(context, '/' ); break;
          case 'b':  PUTC(context, '\b'); break;
          case 'f':  PUTC(context, '\f'); break;
          case 'n':  PUTC(context, '\n'); break;
          case 'r':  PUTC(context, '\r'); break;
          case 't':  PUTC(context, '\t'); break;
          default: {
            context->top = head;
            return CJSON_PARSE_INVALID_STRING_ESCAPE;
          }
        }
        break;
      }
      case '\0': {
        context->top = head;
        return CJSON_PARSE_MISS_QUOTATION_MARK;
      }
      default: {
        if ((unsigned char)ch < 0x20) { 
          context->top = head;
          return CJSON_PARSE_INVALID_STRING_CHAR;
        }
        PUTC(context, ch);
      }
    }
  }
}

static int cjson_parse_value(cjson_context *context, cjson_value *value) {
  switch(*context->json) {
    case 't': return cjson_parse_str(context, value, "true", CJSON_TRUE);
    case 'f': return cjson_parse_str(context, value, "false", CJSON_FALSE);
    case 'n': return cjson_parse_str(context, value, "null", CJSON_NULL);
    case '"': return cjson_parse_string(context, value);
    case '\0': return CJSON_PARSE_EXPECT_VALUE;
    default: return cjson_parse_number(context, value);
  }
}

int cjson_parse(const char *json, cjson_value *value) {
  cjson_context context;
  int state;
  if (!json) {
    state = CJSON_PARSE_EXPECT_VALUE;
    value->type = CJSON_NULL;
  } else {
    context.json = json;
    context.stack = NULL;
    context.size = context.top = 0;
    value->type = CJSON_NULL;
    cjson_parse_whitespace(&context);
    if ((state = cjson_parse_value(&context, value)) == CJSON_PARSE_OK) {
      cjson_parse_whitespace(&context);
      if (*context.json != '\0') {
        state = CJSON_PARSE_ROOT_NOT_SINGULAR;
        value->type = CJSON_NULL;
      } 
    }
    free(context.stack);
  }
  return state;
}

void cjson_free_value(cjson_value *value) {
  if (value->type == CJSON_STRING) {
    free(value->str);
    value->type = CJSON_NULL;
  }
}
