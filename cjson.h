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

typedef struct {
  union {
    struct {
      char *str;
      size_t len;
    };
    double number;
  };
  cjson_type type;
} cjson_value;

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
  CJSON_PARSE_INVALID_STRING_CHAR
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
 * If [value] is not a string, then this function does not have any side effects.
 */
void cjson_free_value(cjson_value *value);

#endif  /* CJSON_H__ */
