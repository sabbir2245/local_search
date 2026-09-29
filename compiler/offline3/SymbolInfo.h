#pragma once

#include <string>
#include <vector>

class SymbolInfo {
public:
    std::string name;
    std::string type;
    bool isArray;
    int arraySize;
    bool isFunction;
    bool defined;
    int paramCount;
    std::vector<std::string> paramTypes;

    SymbolInfo()
        : name(""), type(""), isArray(false), arraySize(0),
          isFunction(false), defined(false), paramCount(0) {}

    SymbolInfo(const std::string &n, const std::string &t)
        : name(n), type(t), isArray(false), arraySize(0),
          isFunction(false), defined(false), paramCount(0) {}
};