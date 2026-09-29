#ifndef SYMBOL_TABLE_CPP_INCLUDED
#define SYMBOL_TABLE_CPP_INCLUDED

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <algorithm>

using namespace std;

struct SymbolInfo {
    string name;
    string type;

    bool isArray = false;
    int arraySize = 0;

    bool isFunction = false;
    bool defined = false;

    string returnType;
    vector<string> parameterTypes;

    SymbolInfo() = default;

    SymbolInfo(const string& n, const string& t)
        : name(n), type(t) {}
};

class SymbolTable {
    struct Scope {
        string id;
        unordered_map<string, SymbolInfo> symbols;
        Scope *parent = nullptr;

        Scope(const string& scopeId, Scope *p)
            : id(scopeId), parent(p) {}
    };

    vector<unique_ptr<Scope>> scopes;
    Scope *current = nullptr;
    int nextScopeId = 1;

public:
    SymbolTable() {
        current = nullptr;
    }

    void enterScope(const string& requestedId = "") {
        string id = requestedId.empty()
                 ? to_string(nextScopeId++)
                 : requestedId;

        if (current) {
            id = current->id + "." + id;
        }

        scopes.push_back(make_unique<Scope>(id, current));
        current = scopes.back().get();
    }

    void exitScope() {
        if (!current) return;
        current = current->parent;
    }

    bool insert(const SymbolInfo& symbol) {
        if (!current) enterScope();
        if (current->symbols.find(symbol.name) != current->symbols.end())
            return false;

        current->symbols[symbol.name] = symbol;
        return true;
    }

    SymbolInfo* lookupCurrent(const string& name) {
        if (!current) return nullptr;

        auto it = current->symbols.find(name);
        if (it == current->symbols.end())
            return nullptr;

        return &it->second;
    }

    SymbolInfo* lookup(const string& name) {
        Scope *s = current;

        while (s) {
            auto it = s->symbols.find(name);
            if (it != s->symbols.end())
                return &it->second;
            s = s->parent;
        }

        return nullptr;
    }

    void printCurrent(ofstream& out) const {
        if (!current) return;

        out << "\nSymbol Table\n";
        out << "ScopeTable # " << current->id << '\n';

        vector<string> names;
        names.reserve(current->symbols.size());

        for (const auto& p : current->symbols)
            names.push_back(p.first);

        sort(names.begin(), names.end());

        for (const string& name : names) {
            const SymbolInfo& s = current->symbols.at(name);

            out << "< " << s.name << " : " << s.type;

            if (s.isArray)
                out << " , array[" << s.arraySize << "]";

            if (s.isFunction) {
                out << " , function";
                out << " , return=" << s.returnType;
                out << " , parameters=" << s.parameterTypes.size();

                if (!s.parameterTypes.empty()) {
                    out << " [";
                    for (size_t i = 0; i < s.parameterTypes.size(); ++i) {
                        if (i) out << ", ";
                        out << s.parameterTypes[i];
                    }
                    out << "]";
                }

                out << (s.defined ? " , defined" : " , declaration");
            }

            out << " >\n";
        }
        out << '\n';
    }

    void printAll(ofstream& out) const {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            out << "\nSymbol Table\n";
            out << "ScopeTable # " << (*it)->id << '\n';

            vector<string> names;
            for (const auto& p : (*it)->symbols)
                names.push_back(p.first);

            sort(names.begin(), names.end());

            for (const string& name : names) {
                const SymbolInfo& s = (*it)->symbols.at(name);

                out << "< " << s.name << " : " << s.type;

                if (s.isArray)
                    out << " , array[" << s.arraySize << "]";

                if (s.isFunction) {
                    out << " , function"
                        << " , return=" << s.returnType
                        << " , parameters=" << s.parameterTypes.size();

                    if (!s.parameterTypes.empty()) {
                        out << " [";
                        for (size_t i = 0; i < s.parameterTypes.size(); ++i) {
                            if (i) out << ", ";
                            out << s.parameterTypes[i];
                        }
                        out << "]";
                    }
                    out << (s.defined ? " , defined" : " , declaration");
                }

                out << " >\n";
            }
        }
    }

    string currentScopeId() const {
        return current ? current->id : "";
    }
};

#endif
