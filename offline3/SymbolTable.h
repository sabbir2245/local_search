#pragma once

#include <fstream>
#include <vector>
#include "SymbolInfo.h"

class SymbolTable {
public:
    std::vector<SymbolInfo> table;

    bool insert(SymbolInfo info) {
        for (const SymbolInfo &s : table) {
            if (s.name == info.name) {
                return false;
            }
        }
        table.push_back(info);
        return true;
    }

    void print(std::ofstream &out) const {
        if (table.empty()) {
            out << "    (empty)" << std::endl;
            return;
        }
        for (const SymbolInfo &s : table) {
            out << "    Name: " << s.name
                << "  Type: " << s.type
                << "  Array: " << (s.isArray ? "yes" : "no");
            if (s.isArray) {
                out << "  Size: " << s.arraySize;
            }
            if (s.isFunction) {
                out << "  Function: Params(" << s.paramCount << ") [";
                for (size_t i = 0; i < s.paramTypes.size(); i++) {
                    out << s.paramTypes[i];
                    if (i + 1 < s.paramTypes.size()) out << ",";
                }
                out << "] " << (s.defined ? "defined" : "declared");
            }
            out << std::endl;
        }
    }
};