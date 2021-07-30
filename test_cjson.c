/**
 * @file test_json.c
 * @author Quan.Dashuai
 * @copyright GNU AFFERO GENERAL PUBLIC LICENSE Version3
 */

#include "cjson.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define PARSE_WRONG_TYPE 128

static const char *parsing_state[] = {
  "CJSON_PARSE_OK",
  "CJSON_PARSE_EXPECT_VALUE",
  "CJSON_PARSE_INVALID_VALUE",
  "CJSON_PARSE_ROOT_NOT_SINGULAR",
  "CJSON_PARSE_NUMBER_TOO_BIG",
  "CJSON_PARSE_MISS_QUOTATION_MARK",
  "CJSON_PARSE_INVALID_STRING_ESCAPE",
  "CJSON_PARSE_INVALID_STRING_CHAR",
  "CJSON_PARSE_INVALID_UNICODE_HEX",
  "CJSON_PARSE_INVALID_UNICODE_SURROGATE",
  "CJSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET",
  "CJSON_PARSE_MISS_KEY",
  "CJSON_PARSE_MISS_COLON",
  "CJSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET",
};

static const char *parsing_result[] = {
  "CJSON_NULL",
  "CJSON_TRUE",
  "CJSON_FALSE",
  "CJSON_NUMBER",
  "CJSON_STRING",
  "CJSON_ARRAY",
  "CJSON_OBJECT",
};

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
  do { \
    test_count++; \
    if (equality) { \
      test_pass++; \
    } else { \
      fprintf(stderr, "%s:%d expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual); \
      main_ret = 1; \
    } \
  } while (0)

#define EXPECT_EQ_STATE(expect, actual) \
        EXPECT_EQ_BASE((expect) == (actual), parsing_state[expect], parsing_state[actual], "%s")

#define EXPECT_EQ_RESULT(expect, actual) \
        EXPECT_EQ_BASE((expect) == (actual), parsing_result[expect], parsing_result[actual], "%s")

#define EXPECT_EQ_DOUBLE(expect, actual) \
        EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")

#define EXPECT_EQ_SIZE_T(expect, actual) \
        EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%ld")

/**
 * @details
 * The reason for using sizeof instead of strlen here is that there may be null characters in the json string, 
 * and strlen will stop when it encounters a null character. 
 * 
 * C language considers a string literal value as an array of characters, 
 * and sizeof can be used to get the number of bytes used in this array 
 * (including the null character at the end of the array).
 */
#define EXPECT_EQ_STRING(expect, actual, length) \
        EXPECT_EQ_BASE(sizeof(expect) - 1 == length && memcmp(expect, actual, length) == 0, expect, actual, "%s")

#define TEST_ERROR(error, json) \
  do { \
    cjson_value value; \
    EXPECT_EQ_STATE(error, cjson_parse(json, &value)); \
    EXPECT_EQ_RESULT(CJSON_NULL, value.type); \
  } while (0)

#define TEST_NUMBER(expect, json) \
  do { \
    cjson_value value; \
    EXPECT_EQ_STATE(CJSON_PARSE_OK, cjson_parse(json, &value)); \
    EXPECT_EQ_RESULT(CJSON_NUMBER, value.type); \
    assert(value.type == CJSON_NUMBER); \
    EXPECT_EQ_DOUBLE(expect, value.number); \
  } while (0)

/**
 * @note
 * [expect] must be a string literal, 
 * so that the macro can correctly calculate the length of the string. 
 * Otherwise, the length of all strings will be calculated as 4 or 8 bytes (depending on your machine).
 */
#define TEST_STRING(expect, json) \
  do { \
    cjson_value value; \
    EXPECT_EQ_STATE(CJSON_PARSE_OK, cjson_parse(json, &value)); \
    EXPECT_EQ_RESULT(CJSON_STRING, value.type); \
    assert(value.type == CJSON_STRING); \
    EXPECT_EQ_STRING(expect, value.str, value.len); \
    cjson_free_value(&value); \
  } while (0)

#define TEST_ROUNDTRIP(json) \
  do { \
    cjson_value value;  \
    char *json2;  \
    size_t length;  \
    EXPECT_EQ_STATE(CJSON_PARSE_OK, cjson_parse(json, &value)); \
    json2 = cjson_stringify(&value, &length); \
    EXPECT_EQ_STRING(json, json2, length);  \
    cjson_free_value(&value); \
    free(json2);  \
  } while (0)

static void test_parse_null() {
  cjson_value value;
  EXPECT_EQ_STATE(CJSON_PARSE_OK, cjson_parse("null", &value));
  EXPECT_EQ_RESULT(CJSON_NULL, value.type);
}

static void test_parse_true() {
  cjson_value value;
  EXPECT_EQ_STATE(CJSON_PARSE_OK, cjson_parse("true", &value));
  EXPECT_EQ_RESULT(CJSON_TRUE, value.type);

  EXPECT_EQ_STATE(CJSON_PARSE_OK, cjson_parse("true\n", &value));
  EXPECT_EQ_RESULT(CJSON_TRUE, value.type);

  EXPECT_EQ_STATE(CJSON_PARSE_OK, cjson_parse("true\r", &value));
  EXPECT_EQ_RESULT(CJSON_TRUE, value.type);
}

static void test_parse_false() {
  cjson_value value;
  EXPECT_EQ_STATE(CJSON_PARSE_OK, cjson_parse("false", &value));
  EXPECT_EQ_RESULT(CJSON_FALSE, value.type);
}

static void test_parse_number() {
  TEST_NUMBER(0.0, "0");
  TEST_NUMBER(0.0, "-0");
  TEST_NUMBER(0.0, "-0.0");
  TEST_NUMBER(1.0, "1");
  TEST_NUMBER(-1.0, "-1");
  TEST_NUMBER(1.5, "1.5");
  TEST_NUMBER(-1.5, "-1.5");
  TEST_NUMBER(3.1416, "3.1416");
  TEST_NUMBER(31.416, "31.416");
  TEST_NUMBER(1E10, "1E10");
  TEST_NUMBER(1e10, "1e10");
  TEST_NUMBER(1E+10, "1E+10");
  TEST_NUMBER(1E-10, "1E-10");
  TEST_NUMBER(-1E10, "-1E10");
  TEST_NUMBER(-1e10, "-1e10");
  TEST_NUMBER(-1E+10, "-1E+10");
  TEST_NUMBER(-1E-10, "-1E-10");
  TEST_NUMBER(1.234E+10, "1.234E+10");
  TEST_NUMBER(1.234E-10, "1.234E-10");
  TEST_NUMBER(0.0, "1e-10000"); /* must underflow */

  TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
  TEST_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
  TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
  TEST_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
  TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
  TEST_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
  TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
  TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
  TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

static void test_parse_string() {
  TEST_STRING("", "\"\"");
  TEST_STRING("Hello", "\"Hello\"");
  TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
  TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
  TEST_STRING("Hello\0World", "\"Hello\\u0000World\"");
  TEST_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
  TEST_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
  TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
  TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
  TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
}

static void test_parse_array() {
  cjson_value value;

  EXPECT_EQ_STATE(CJSON_PARSE_OK, cjson_parse("[ ]", &value));
  EXPECT_EQ_RESULT(CJSON_ARRAY, value.type);
  EXPECT_EQ_SIZE_T(0, value.size);
  cjson_free_value(&value);

  EXPECT_EQ_STATE(CJSON_PARSE_OK, cjson_parse("[ null , false , true , 123 , \"abc\" ]", &value));
  EXPECT_EQ_RESULT(CJSON_ARRAY, value.type);
  EXPECT_EQ_SIZE_T(5, value.size);
  EXPECT_EQ_STATE(CJSON_NULL, value.elements[0].type);
  EXPECT_EQ_STATE(CJSON_FALSE, value.elements[1].type);
  EXPECT_EQ_STATE(CJSON_TRUE, value.elements[2].type);
  EXPECT_EQ_STATE(CJSON_NUMBER, value.elements[3].type);
  EXPECT_EQ_STATE(CJSON_STRING, value.elements[4].type);
  EXPECT_EQ_DOUBLE(123.0, value.elements[3].number);
  EXPECT_EQ_STRING("abc", value.elements[4].str, value.elements[4].len);
  cjson_free_value(&value);

  EXPECT_EQ_STATE(CJSON_PARSE_OK, cjson_parse("[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]", &value));
  EXPECT_EQ_RESULT(CJSON_ARRAY, value.type);
  EXPECT_EQ_SIZE_T(4, value.size);
  for (int i = 0; i < 4; i++) {
    cjson_value *element = &value.elements[i];
    EXPECT_EQ_RESULT(CJSON_ARRAY, element->type);
    EXPECT_EQ_SIZE_T((size_t)i, element->size);
    for (int j = 0; j < i; j++) {
      cjson_value *num = &element->elements[j];
      EXPECT_EQ_RESULT(CJSON_NUMBER, num->type);
      EXPECT_EQ_DOUBLE((double)j, num->number);
    }
  }
  cjson_free_value(&value);
}

static void test_parse_object() {
  cjson_value value;

  EXPECT_EQ_STATE(CJSON_PARSE_OK, cjson_parse(" { } ", &value));
  EXPECT_EQ_RESULT(CJSON_OBJECT, value.type);
  EXPECT_EQ_SIZE_T(0, value.mem_size);
  cjson_free_value(&value);

  EXPECT_EQ_STATE(CJSON_PARSE_OK, cjson_parse(
    " { "
    "\"n\" : null , "
    "\"f\" : false , "
    "\"t\" : true , "
    "\"i\" : 123 , "
    "\"s\" : \"abc\", "
    "\"a\" : [ 1, 2, 3 ],"
    "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
    " } "
    , &value));
  EXPECT_EQ_RESULT(CJSON_OBJECT, value.type);
  EXPECT_EQ_SIZE_T(7, value.mem_size);
  EXPECT_EQ_STRING("n", value.members[0].key, value.members[0].klen);
  EXPECT_EQ_RESULT(CJSON_NULL, value.members[0].value.type);
  EXPECT_EQ_STRING("f", value.members[1].key, value.members[1].klen);
  EXPECT_EQ_RESULT(CJSON_FALSE, value.members[1].value.type);
  EXPECT_EQ_STRING("t", value.members[2].key, value.members[2].klen);
  EXPECT_EQ_RESULT(CJSON_TRUE, value.members[2].value.type);
  EXPECT_EQ_STRING("i", value.members[3].key, value.members[3].klen);
  EXPECT_EQ_RESULT(CJSON_NUMBER, value.members[3].value.type);
  EXPECT_EQ_DOUBLE(123.0, value.members[3].value.number);
  EXPECT_EQ_STRING("s", value.members[4].key, value.members[4].klen);
  EXPECT_EQ_RESULT(CJSON_STRING, value.members[4].value.type);
  EXPECT_EQ_STRING("abc", value.members[4].value.str, value.members[4].value.len);
  EXPECT_EQ_STRING("a", value.members[5].key, value.members[5].klen);
  EXPECT_EQ_RESULT(CJSON_ARRAY, value.members[5].value.type);
  EXPECT_EQ_SIZE_T(3, value.members[5].value.size);
  for (int i = 1; i < 4; i++) {
    cjson_value *temp = &value.members[5].value.elements[i - 1];
    EXPECT_EQ_RESULT(CJSON_NUMBER, temp->type);
    EXPECT_EQ_DOUBLE((double)i, temp->number);
  }
  EXPECT_EQ_STRING("o", value.members[6].key, value.members[6].klen);
  cjson_value *temp = &value.members[6].value;
  EXPECT_EQ_RESULT(CJSON_OBJECT, temp->type);
  EXPECT_EQ_SIZE_T(3, temp->mem_size);
  for (int i = 1; i < 4; i++) {
    EXPECT_EQ_RESULT(CJSON_NUMBER, temp->members[i - 1].value.type);
    EXPECT_EQ_DOUBLE((double)i, temp->members[i - 1].value.number);
  }
  cjson_free_value(&value);
}

static void test_parse_expect_value() {
  TEST_ERROR(CJSON_PARSE_EXPECT_VALUE, NULL);
  TEST_ERROR(CJSON_PARSE_EXPECT_VALUE, "");
  TEST_ERROR(CJSON_PARSE_EXPECT_VALUE, " ");
}

static void test_parse_invalid_value() {
  TEST_ERROR(CJSON_PARSE_INVALID_VALUE, "nul");
  TEST_ERROR(CJSON_PARSE_INVALID_VALUE, "falss");
  TEST_ERROR(CJSON_PARSE_INVALID_VALUE, "?");

  /* invalid number */
  TEST_ERROR(CJSON_PARSE_INVALID_VALUE, "+0");
  TEST_ERROR(CJSON_PARSE_INVALID_VALUE, "+1");
  TEST_ERROR(CJSON_PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
  TEST_ERROR(CJSON_PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
  TEST_ERROR(CJSON_PARSE_INVALID_VALUE, "1em");
  TEST_ERROR(CJSON_PARSE_INVALID_VALUE, "INF");
  TEST_ERROR(CJSON_PARSE_INVALID_VALUE, "inf");
  TEST_ERROR(CJSON_PARSE_INVALID_VALUE, "NAN");
  TEST_ERROR(CJSON_PARSE_INVALID_VALUE, "nan");

  /* invalid array */
  TEST_ERROR(CJSON_PARSE_INVALID_VALUE, "[1,]");
  TEST_ERROR(CJSON_PARSE_INVALID_VALUE, "[\"a\", nul]");

  /* invalid object */
  TEST_ERROR(CJSON_PARSE_INVALID_VALUE, "{\"1\": tru}");
}

static void test_parse_root_not_singular() {
  TEST_ERROR(CJSON_PARSE_ROOT_NOT_SINGULAR, "true ?");

  /* invalid number */
  TEST_ERROR(CJSON_PARSE_ROOT_NOT_SINGULAR, "0123"); /* after zero should be '.' or nothing */
  TEST_ERROR(CJSON_PARSE_ROOT_NOT_SINGULAR, "0x0");
  TEST_ERROR(CJSON_PARSE_ROOT_NOT_SINGULAR, "0x123");
}

static void test_parse_number_too_big() {
  TEST_ERROR(CJSON_PARSE_NUMBER_TOO_BIG, "1e309");
  TEST_ERROR(CJSON_PARSE_NUMBER_TOO_BIG, "-1e309");
}

static void test_parse_missing_quotation_mark() {
  TEST_ERROR(CJSON_PARSE_MISS_QUOTATION_MARK, "\"");
  TEST_ERROR(CJSON_PARSE_MISS_QUOTATION_MARK, "\"abc");
  TEST_ERROR(CJSON_PARSE_MISS_QUOTATION_MARK, "{\"1\": \"123}");
}

static void test_parse_invalid_string_escape() {
  TEST_ERROR(CJSON_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
  TEST_ERROR(CJSON_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
  TEST_ERROR(CJSON_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
  TEST_ERROR(CJSON_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

static void test_parse_invalid_string_char() {
  TEST_ERROR(CJSON_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
  TEST_ERROR(CJSON_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
  TEST_ERROR(CJSON_PARSE_INVALID_STRING_CHAR, "{\"\a\": \"123\"}");
}

static void test_parse_invalid_unicode_hex() {
  TEST_ERROR(CJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
  TEST_ERROR(CJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
  TEST_ERROR(CJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
  TEST_ERROR(CJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
  TEST_ERROR(CJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
  TEST_ERROR(CJSON_PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
  TEST_ERROR(CJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
  TEST_ERROR(CJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
  TEST_ERROR(CJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u00/0\"");
  TEST_ERROR(CJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
  TEST_ERROR(CJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
  TEST_ERROR(CJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
  TEST_ERROR(CJSON_PARSE_INVALID_UNICODE_HEX, "\"\\u 123\"");
  TEST_ERROR(CJSON_PARSE_INVALID_UNICODE_HEX, "\"\\uD800\\uG000\"");
}

static void test_parse_invalid_unicode_surrogate() {
  TEST_ERROR(CJSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
  TEST_ERROR(CJSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
  TEST_ERROR(CJSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
  TEST_ERROR(CJSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
  TEST_ERROR(CJSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

static void test_parse_miss_comma_or_square_bracket() {
  TEST_ERROR(CJSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
  TEST_ERROR(CJSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
  TEST_ERROR(CJSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
  TEST_ERROR(CJSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
}

static void test_parse_miss_key() {
  TEST_ERROR(CJSON_PARSE_MISS_KEY, "{:1,");
  TEST_ERROR(CJSON_PARSE_MISS_KEY, "{1:1,");
  TEST_ERROR(CJSON_PARSE_MISS_KEY, "{true:1,");
  TEST_ERROR(CJSON_PARSE_MISS_KEY, "{false:1,");
  TEST_ERROR(CJSON_PARSE_MISS_KEY, "{null:1,");
  TEST_ERROR(CJSON_PARSE_MISS_KEY, "{[]:1,");
  TEST_ERROR(CJSON_PARSE_MISS_KEY, "{{}:1,");
  TEST_ERROR(CJSON_PARSE_MISS_KEY, "{\"a\":1,");
}

static void test_parse_miss_colon() {
  TEST_ERROR(CJSON_PARSE_MISS_COLON, "{\"a\"}");
  TEST_ERROR(CJSON_PARSE_MISS_COLON, "{\"a\",\"b\"}");
}

static void test_parse_miss_comma_or_curly_bracket() {
  TEST_ERROR(CJSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1");
  TEST_ERROR(CJSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1]");
  TEST_ERROR(CJSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1 \"b\"");
  TEST_ERROR(CJSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":{}");
}

static void test_parse() {
  test_parse_null();
  test_parse_true();
  test_parse_false();
  test_parse_number();
  test_parse_string();
  test_parse_array();
  test_parse_object();
  test_parse_expect_value();
  test_parse_invalid_value();
  test_parse_root_not_singular();
  test_parse_number_too_big();
  test_parse_missing_quotation_mark();
  test_parse_invalid_string_escape();
  test_parse_invalid_string_char();
  test_parse_invalid_unicode_hex();
  test_parse_invalid_unicode_surrogate();
  test_parse_miss_comma_or_square_bracket();
  test_parse_miss_key();
  test_parse_miss_colon();
  test_parse_miss_comma_or_curly_bracket();
}

static void test_stringify_number() {
  TEST_ROUNDTRIP("0");
  TEST_ROUNDTRIP("-0");
  TEST_ROUNDTRIP("1");
  TEST_ROUNDTRIP("-1");
  TEST_ROUNDTRIP("1.5");
  TEST_ROUNDTRIP("-1.5");
  TEST_ROUNDTRIP("3.25");
  TEST_ROUNDTRIP("1e+20");
  TEST_ROUNDTRIP("1.234e+20");
  TEST_ROUNDTRIP("1.234e-20");

  TEST_ROUNDTRIP("1.0000000000000002"); /* the smallest number > 1 */
  TEST_ROUNDTRIP("4.9406564584124654e-324"); /* minimum denormal */
  TEST_ROUNDTRIP("-4.9406564584124654e-324");
  TEST_ROUNDTRIP("2.2250738585072009e-308");  /* Max subnormal double */
  TEST_ROUNDTRIP("-2.2250738585072009e-308");
  TEST_ROUNDTRIP("2.2250738585072014e-308");  /* Min normal positive double */
  TEST_ROUNDTRIP("-2.2250738585072014e-308");
  TEST_ROUNDTRIP("1.7976931348623157e+308");  /* Max double */
  TEST_ROUNDTRIP("-1.7976931348623157e+308");
}

static void test_stringify_string() {
  TEST_ROUNDTRIP("\"\"");
  TEST_ROUNDTRIP("\"Hello\"");
  TEST_ROUNDTRIP("\"Hello\\nWorld\"");
  TEST_ROUNDTRIP("\"\\\" \\\\ / \\b \\f \\n \\r \\t\"");
  TEST_ROUNDTRIP("\"Hello\\u0000World\"");
}

static void test_stringify_array() {
  TEST_ROUNDTRIP("[]");
  TEST_ROUNDTRIP("[null,false,true,123,\"abc\",[1,2,3]]");
}

static void test_stringify_object() {
  TEST_ROUNDTRIP("{}");
  TEST_ROUNDTRIP("{\"n\":null,\"f\":false,\"t\":true,\"i\":123,\"s\":\"abc\",\"a\":[1,2,3],\"o\":{\"1\":1,\"2\":2,\"3\":3}}");
}

static void test_stringify() {
  TEST_ROUNDTRIP("null");
  TEST_ROUNDTRIP("false");
  TEST_ROUNDTRIP("true");
  test_stringify_number();
  test_stringify_string();
  test_stringify_array();
  test_stringify_object();
  
  cjson_value value;
  value.type = PARSE_WRONG_TYPE;
  assert(!cjson_stringify(&value, NULL));

  cjson_parse("null", &value);
  char *json = cjson_stringify(&value, NULL);
  cjson_free_value(&value);
  free(json);
}

int main() {
  test_parse();
  test_stringify();
  printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
  return main_ret;
}
