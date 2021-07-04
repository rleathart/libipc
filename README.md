# libipc

A simple multi-platform, async ipc library. Uses named pipes on Win32 and Unix
domain sockets otherwise.

## Building

Compile with
```
cmake -B build -G Ninja
cmake --build build
```

The following options can be passed to cmake at the configure stage:

|Option|Description|
|---|---|
|`-DCMAKE_BUILD_TYPE=(Release\|Debug)`| Sets the build type|
|`-DBUILD_SHARED_LIBS=(ON\|OFF)`| Builds the library as static or shared (static by default)|
|`-DIPC_BUILD_TESTS=(ON\|OFF)` | Builds the tests as well as the library.<br>Requires [Check](https://github.com/libcheck/check) to be locatable by cmake|
