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

static const char *parsing_state[] = {
  "CJSON_PARSE_OK",
  "CJSON_PARSE_EXPECT_VALUE",
  "CJSON_PARSE_INVALID_VALUE",
  "CJSON_PARSE_ROOT_NOT_SINGULAR",
  "CJSON_PARSE_NUMBER_TOO_BIG",
  "CJSON_PARSE_MISS_QUOTATION_MARK",
  "CJSON_PARSE_INVALID_STRING_ESCAPE",
  "CJSON_PARSE_INVALID_STRING_CHAR",
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
  } while(0)

#define EXPECT_EQ_STATE(expect, actual) \
        EXPECT_EQ_BASE((expect) == (actual), parsing_state[expect], parsing_state[actual], "%s")

#define EXPECT_EQ_RESULT(expect, actual) \
        EXPECT_EQ_BASE((expect) == (actual), parsing_result[expect], parsing_result[actual], "%s")

#define EXPECT_EQ_DOUBLE(expect, actual) \
        EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")

#define EXPECT_EQ_STRING(expect, actual, length) \
        EXPECT_EQ_BASE(strlen(expect) == length && memcmp(expect, actual, length) == 0, expect, actual, "%s")

#define TEST_ERROR(error, json) \
  do { \
    cjson_value value; \
    EXPECT_EQ_STATE(error, cjson_parse(json, &value)); \
    EXPECT_EQ_RESULT(CJSON_NULL, value.type); \
  } while(0)

#define TEST_NUMBER(expect, json) \
  do { \
    cjson_value value; \
    EXPECT_EQ_STATE(CJSON_PARSE_OK, cjson_parse(json, &value)); \
    EXPECT_EQ_RESULT(CJSON_NUMBER, value.type); \
    assert(value.type == CJSON_NUMBER); \
    EXPECT_EQ_DOUBLE(expect, value.number); \
  } while(0)

#define TEST_STRING(expect, json) \
  do { \
    cjson_value value; \
    EXPECT_EQ_STATE(CJSON_PARSE_OK, cjson_parse(json, &value)); \
    EXPECT_EQ_RESULT(CJSON_STRING, value.type); \
    assert(value.type == CJSON_STRING); \
    EXPECT_EQ_STRING(expect, value.str, value.len); \
    cjson_free_value(&value); \
  } while(0)

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
}

static void test_parse() {
  test_parse_null();
  test_parse_true();
  test_parse_false();
  test_parse_number();
  test_parse_string();
  test_parse_expect_value();
  test_parse_invalid_value();
  test_parse_root_not_singular();
  test_parse_number_too_big();
  test_parse_missing_quotation_mark();
  test_parse_invalid_string_escape();
  test_parse_invalid_string_char();
}

int main() {
  test_parse();
  printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
  return main_ret;
}
