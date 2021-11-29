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
  CJSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET,
  CJSON_PARSE_NULL
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

/**
 * @brief Convert the internal data structure of the json parser into a C-style string.
 * 
 * @param[in] value The internal data structure of the json parser.
 * @param[out] length  The length of the converted C-style string, can be NULL.
 * 
 * @return A pointer to the converted C-style string, or NULL if stringify failed.
 * 
 * @note
 * This function uses dynamic memory to store the converted string, 
 * so the caller is responsible for freeing the pointer returned by this function.
 */
char* cjson_stringify(const cjson_value *value, size_t *length);

/**
 * @brief Find the index corresponding to a key of the json object.
 * 
 * @param[in] value The result of parsing json string.
 * @param[in] key The key to find.
 * @param[in] klen The length of the key to find.
 * 
 * @return The index of the target key in the json object, or -1 if not find.
 */
int cjson_find_key_index(const cjson_value *value, const char *key, size_t klen);

/**
 * @brief Find the value corresponding to a key in the json object.
 * 
 * @param[in] value The result of parsing json string.
 * @param[in] key The key to find.
 * @param[in] klen The length of the key to find.
 * 
 * @return The value corresponding to the target key in the json object, or NULL if not find.
 */
const cjson_value* cjson_find_key_value(const cjson_value *value, const char *key, size_t klen);

/**
 * @brief Compare two cjson_value. 
 * 
 * @param[in] lhs The object of comparison.
 * @param[in] rhs The object of comparison.
 * 
 * @return 0 means equal, -1 means not euqal.
 */
int cjson_is_equal(const cjson_value *lhs, const cjson_value *rhs);

#endif  /* CJSON_H__ */
