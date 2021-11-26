# cjson

[![CMake](https://github.com/qdslovelife/cjson/actions/workflows/cmake.yml/badge.svg)](https://github.com/qdslovelife/cjson/actions/workflows/cmake.yml)
[![codecov](https://codecov.io/gh/qdslovelife/cjson/branch/main/graph/badge.svg?token=OP2XQAKIYM)](https://codecov.io/gh/qdslovelife/cjson) 
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/7ad3b1a88d0f4534be7df8abb8e87c20)](https://www.codacy.com/gh/qdslovelife/cjson/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=qdslovelife/cjson&amp;utm_campaign=Badge_Grade)

一个用C编写的JSON解析器和生成器。

代码参考自[json-tutorial](https://github.com/miloyip/json-tutorial)。

## 构建

``` bash
cmake -Bbuild -DCMAKE_BUILD_TYPE=Release
cd build && make
```

## 测试

``` bash
ctest --verbose
```

## 使用

### 反序列化

``` c
// 省略错误处理
char json_buf[512];
cjson_value value;
int json_fd = open("hello.json", O_RDONLY);
read(json_fd, json_buf, sizeof(json_buf));
cjson_parse(json_buf, &value);
```

### 序列化

``` c
cjson_value value
size_t length;
// 写入value
const char *result = cjson_stringfy(&value, &length);
```
