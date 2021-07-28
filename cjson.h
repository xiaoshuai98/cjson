/**
 * @file cjson.h
 * @author Quan.Dashuai
 * @copyright GNU AFFERO GENERAL PUBLIC LICENSE Version3
 */

#ifndef CJSON_H__
#define CJSON_H__

#include <string.h>

/**
 * @brief Data type in json parser.
 */
typedef enum {
  CJSON_NULL = 0,
  CJSON_TRUE,
  CJSON_FALSE,
  CJSON_NUMBER,
  CJSON_STRING,
  CJSON_ARRAY,
  CJSON_OBJECT
} cjson_type;

typedef struct cjson_value cjson_value;
typedef struct cjson_member cjson_member;

struct cjson_value {
  union {
    struct { cjson_member *members; size_t mem_size; };
    struct { cjson_value *elements; size_t size; };
    struct { char *str; size_t len; };
    double number;
  };
  cjson_type type;
};

struct cjson_member {
  char *key;
  size_t klen;
  cjson_value value;
};

/**
 * @brief The state of parsing json.
 */
enum {
  CJSON_PARSE_OK = 0,
  CJSON_PARSE_EXPECT_VALUE,
  CJSON_PARSE_INVALID_VALUE,
  CJSON_PARSE_ROOT_NOT_SINGULAR,
  CJSON_PARSE_NUMBER_TOO_BIG,
  CJSON_PARSE_MISS_QUOTATION_MARK,
  CJSON_PARSE_INVALID_STRING_ESCAPE,
  CJSON_PARSE_INVALID_STRING_CHAR,
  CJSON_PARSE_INVALID_UNICODE_HEX,
  CJSON_PARSE_INVALID_UNICODE_SURROGATE,
  CJSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
  CJSON_PARSE_MISS_KEY,
  CJSON_PARSE_MISS_COLON,
  CJSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET
};

/**
 * @brief Parse the json string.
 * 
 * @param[in] json The original json string.
 * @param[out] value The result of parsing json, can't be NULL.
 * 
 * @return The state of parsing json.
 */
int cjson_parse(const char *json, cjson_value *value);

/**
 * @brief Free the memory space of [value].
 * 
 * @param[in] value The result of parsing json, can't be NULL.
 * 
 * @note
 * If [value] is not a string or array, then this function does not have any side effects.
 */
void cjson_free_value(cjson_value *value);

#endif  /* CJSON_H__ */
