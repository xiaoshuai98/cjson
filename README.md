# cjson

[![linux](https://github.com/qdslovelife/cjson/actions/workflows/cmake.yml/badge.svg)](https://github.com/qdslovelife/cjson/actions/workflows/cmake.yml)
[![windows](https://github.com/qdslovelife/cjson/actions/workflows/windows.yml/badge.svg)](https://github.com/qdslovelife/cjson/actions/workflows/windows.yml)
[![codecov](https://codecov.io/gh/xiaoshuai98/cjson/branch/main/graph/badge.svg?token=M4RL924FZK)](https://codecov.io/gh/xiaoshuai98/cjson)

A JSON parser and generator written in C.

## Build

### Linux

``` bash
cmake -Bbuild -DCMAKE_BUILD_TYPE=Debug
cd build && make
```

### Windows

```
./packages.bat
```

Use visual studio to open the folder, then you're done.

Note: The version of visual studio I am using is [Visual Studio Community 2022 preview](https://visualstudio.microsoft.com/zh-hans/vs/)

## Test

``` bash
ctest
```

## Using

### Deserialization

``` c
// omit error handling
char json_buf[512];
cjson_value value;
int json_fd = open("hello.json", O_RDONLY);
read(json_fd, json_buf, sizeof(json_buf));
cjson_parse(json_buf, &value);
```

### Serialization

``` c
cjson_value value
size_t length;
// modify 'value'
const char *result = cjson_stringfy(&value, &length);
```
