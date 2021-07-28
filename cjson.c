/**
 * @file cjson.c
 * @author Quan.Dashuai
 * @copyright GNU AFFERO GENERAL PUBLIC LICENSE Version3
 */

#include "cjson.h"

#include <stdlib.h>
#include <math.h>
#include <errno.h>

#define CJSON_PARSE_STACK_INIT_SIZE 32

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

static const char* cjson_parse_hex4(const char* p, unsigned* u) {
  int i;
  *u = 0;
  for (i = 0; i < 4; i++) {
    char ch = *p++;
    *u <<= 4;
    if      (ch >= '0' && ch <= '9')  *u |= ch - '0';
    else if (ch >= 'A' && ch <= 'F')  *u |= ch - ('A' - 10);
    else if (ch >= 'a' && ch <= 'f')  *u |= ch - ('a' - 10);
    else return NULL;
  }
  return p;
}

static void cjson_encode_utf8(cjson_context* context, unsigned u) {
  if (u <= 0x7F) {
    PUTC(context, u & 0xFF);
  } else if (u <= 0x7FF) {
    PUTC(context, 0xC0 | ((u >> 6) & 0xFF));
    PUTC(context, 0x80 | ( u       & 0x3F));
  } else if (u <= 0xFFFF) {
    PUTC(context, 0xE0 | ((u >> 12) & 0xFF));
    PUTC(context, 0x80 | ((u >>  6) & 0x3F));
    PUTC(context, 0x80 | ( u        & 0x3F));
  } else {
    PUTC(context, 0xF0 | ((u >> 18) & 0xFF));
    PUTC(context, 0x80 | ((u >> 12) & 0x3F));
    PUTC(context, 0x80 | ((u >>  6) & 0x3F));
    PUTC(context, 0x80 | ( u        & 0x3F));
  }
}

static inline int cjson_parse_error(cjson_context *context, size_t head, int error) {
  context->top = head;
  return error;
}

static void cjson_set_string(cjson_value *value, const char *src, size_t len) {
  cjson_free_value(value);
  value->str = (char*)malloc(len + 1);
  memcpy(value->str, src, len);
  value->str[len] = '\0';
  value->len = len;
  value->type = CJSON_STRING;
}

static int cjson_parse_string_raw(cjson_context *context, char **str, size_t *len) {
  size_t head = context->top;
  const char* p;
  unsigned u, u2;
  p = context->json + 1;
  while(1) {
    char ch = *p++;
    switch (ch) {
      case '"': {
        *len = context->top - head;
        *str = cjson_context_pop(context, *len);
        context->json = p;
        return CJSON_PARSE_OK;
      }
      case '\\': {
        switch (*p++) {
          case '"':  PUTC(context, '"');  break;
          case '\\': PUTC(context, '\\'); break;
          case '/':  PUTC(context, '/' ); break;
          case 'b':  PUTC(context, '\b'); break;
          case 'f':  PUTC(context, '\f'); break;
          case 'n':  PUTC(context, '\n'); break;
          case 'r':  PUTC(context, '\r'); break;
          case 't':  PUTC(context, '\t'); break;
          case 'u': {
            if (!(p = cjson_parse_hex4(p, &u))) {
              return cjson_parse_error(context, head, CJSON_PARSE_INVALID_UNICODE_HEX);
            } 
            if (u >= 0xD800 && u <= 0xDBFF) { /* surrogate pair */
              if (*p++ != '\\')
                return cjson_parse_error(context, head, CJSON_PARSE_INVALID_UNICODE_SURROGATE);
              if (*p++ != 'u')
                return cjson_parse_error(context, head, CJSON_PARSE_INVALID_UNICODE_SURROGATE);
              if (!(p = cjson_parse_hex4(p, &u2)))
                return cjson_parse_error(context, head, CJSON_PARSE_INVALID_UNICODE_HEX);
              if (u2 < 0xDC00 || u2 > 0xDFFF)
                return cjson_parse_error(context, head, CJSON_PARSE_INVALID_UNICODE_SURROGATE);
              u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
            }
            cjson_encode_utf8(context, u);
            break;
          }
          default: {
            return cjson_parse_error(context, head, CJSON_PARSE_INVALID_STRING_ESCAPE);
          }
        }
        break;
      }
      case '\0': {
        return cjson_parse_error(context, head, CJSON_PARSE_MISS_QUOTATION_MARK);
      }
      default: {
        if ((unsigned char)ch < 0x20) { 
          return cjson_parse_error(context, head, CJSON_PARSE_INVALID_STRING_CHAR);
        }
        PUTC(context, ch);
      }
    }
  }
}

static int cjson_parse_string(cjson_context *context, cjson_value *value) {
  int state;
  char *str;
  size_t len;
  if ((state = cjson_parse_string_raw(context, &str, &len)) == CJSON_PARSE_OK) {
    cjson_set_string(value, str, len);
  }
  return state;
}

/* Forward declaration */
static int cjson_parse_value(cjson_context *context, cjson_value *value);

static int cjson_parse_array(cjson_context *context, cjson_value *value) {
  size_t size = 0;
  int ret;
  context->json++;
  cjson_parse_whitespace(context);
  if (*context->json == ']') {
    value->type = CJSON_ARRAY;
    value->elements = NULL;
    value->size = 0;
    context->json++;
    return CJSON_PARSE_OK;
  }
  while (1) {
    cjson_value element;
    if ((ret = cjson_parse_value(context, &element)) != CJSON_PARSE_OK) {
      break;
    }
    memcpy(cjson_context_push(context, sizeof(cjson_value)), &element, sizeof(cjson_value));
    size++;
    cjson_parse_whitespace(context);
    if (*context->json == ',') {
      context->json++;
      cjson_parse_whitespace(context);
    } else if (*context->json == ']') {
      value->type = CJSON_ARRAY;
      value->size = size;
      context->json++;
      size *= sizeof(cjson_value);
      memcpy(value->elements = (cjson_value*)malloc(size), cjson_context_pop(context, size), size);
      return CJSON_PARSE_OK;
    } else {
      ret = CJSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
      break;
    }
  }

  for (size_t i = 0; i < size; i++) {
    cjson_free_value((cjson_value*)cjson_context_pop(context, sizeof(cjson_value)));
  }
  return ret;
}

static int cjson_parse_object(cjson_context *context, cjson_value *value) {
  size_t mem_size;
  int state;
  context->json++;
  mem_size = 0;
  cjson_parse_whitespace(context);
  if (*context->json == '}') {
    value->type = CJSON_OBJECT;
    value->members = NULL;
    value->mem_size = 0;
    context->json++;
    return CJSON_PARSE_OK;
  }
  while (1) {
    cjson_member member;
    char *str;
    if (*context->json != '"') {
      state = CJSON_PARSE_MISS_KEY;
      break;
    }
    if ((state = cjson_parse_string_raw(context, &str, &member.klen)) != CJSON_PARSE_OK) {
      break;
    }
    memcpy(member.key = (char*)malloc(member.klen + 1), str, member.klen);
    member.key[member.klen] = '\0';
    cjson_parse_whitespace(context);
    if (*context->json != ':') {
      state = CJSON_PARSE_MISS_COLON;
      free(member.key);
      break;
    }
    context->json++;
    cjson_parse_whitespace(context);
    if ((state = cjson_parse_value(context, &member.value)) != CJSON_PARSE_OK) {
      free(member.key);
      break;
    }
    memcpy(cjson_context_push(context, sizeof(cjson_member)), &member, sizeof(member));
    mem_size++;
    cjson_parse_whitespace(context);
    if (*context->json == ',') {
      context->json++;
      cjson_parse_whitespace(context);
    } else if (*context->json == '}') {
      value->type = CJSON_OBJECT;
      value->mem_size = mem_size;
      context->json++;
      mem_size *= sizeof(cjson_member);
      memcpy(value->members = (cjson_member*)malloc(mem_size), cjson_context_pop(context, mem_size), mem_size);
      return CJSON_PARSE_OK;
    } else {
      state = CJSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
      break;
    }
  }
  for (size_t i = 0; i < mem_size; i++) {
    cjson_member *member = cjson_context_pop(context, sizeof(cjson_member));
    free(member->key);
    cjson_free_value(&member->value);
  }
  value->type = CJSON_NULL;
  return state;
}

static int cjson_parse_value(cjson_context *context, cjson_value *value) {
  switch(*context->json) {
    case 't': return cjson_parse_str(context, value, "true", CJSON_TRUE);
    case 'f': return cjson_parse_str(context, value, "false", CJSON_FALSE);
    case 'n': return cjson_parse_str(context, value, "null", CJSON_NULL);
    case '"': return cjson_parse_string(context, value);
    case '[': return cjson_parse_array(context, value);
    case '{': return cjson_parse_object(context, value);
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
  switch (value->type) {
    case CJSON_STRING: {
      free(value->str);
      break;
    }
    case CJSON_ARRAY: {
      for (size_t i = 0; i < value->size; i++) {
        cjson_free_value(&(value->elements[i]));
      }
      free(value->elements);
      break;
    }
    case CJSON_OBJECT: {
      for (size_t i = 0; i < value->mem_size; i++) {
        free(value->members[i].key);
        cjson_free_value(&value->members[i].value);
      }
      free(value->members);
      break;
    }
    default:
      return;
  }
  value->type = CJSON_NULL;
}
