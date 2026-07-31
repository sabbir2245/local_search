#ifndef SYMBOL_TABLE_CPP
#define SYMBOL_TABLE_CPP

#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <utility>

using namespace std;

class SymbolInfo {
public:
    string name;
    string type;
    SymbolInfo() {}
    SymbolInfo(const string& n, const string& t) : name(n), type(t) {}
};

class ScopeTable {
private:
    int id;
    int bucketCount;
    vector<list<SymbolInfo>> table;
    int totalSymbols;
    ScopeTable* parentScope;

public:
    ScopeTable(int n, int scopeId) {
        bucketCount = n;
        id = scopeId;
        table.resize(bucketCount);
        totalSymbols = 0;
        parentScope = nullptr;
    }

    int getId() const { return id; }

    void setParentScope(ScopeTable* p) { parentScope = p; }
    ScopeTable* getParentScope() const { return parentScope; }

    int hashFunc(const string& name) const {
        int h = 0;
        for (size_t i = 0; i < name.size(); i++) {
            h = (h + name[i]) % bucketCount;
        }
        return h;
    }

    // returns position in the list, -1 if not found
    int Lookup(const string& name) const {
        int idx = hashFunc(name);
        int pos = 0;
        for (auto it = table[idx].begin(); it != table[idx].end(); ++it, ++pos) {
            if (it->name == name) return pos;
        }
        return -1;
    }

    bool Insert(const string& name, const string& type) {
        if (Lookup(name) != -1) return false;
        int idx = hashFunc(name);
        table[idx].push_back(SymbolInfo(name, type));
        totalSymbols++;
        return true;
    }

    bool Remove(const string& name) {
        int idx = hashFunc(name);
        for (auto it = table[idx].begin(); it != table[idx].end(); ++it) {
            if (it->name == name) {
                table[idx].erase(it);
                totalSymbols--;
                return true;
            }
        }
        return false;
    }

    void Print(ostream& out) const {
        out << "ScopeTable # " << id << endl;
        for (int i = 0; i < bucketCount; i++) {
            out << "  " << i << " --> ";
            bool first = true;
            for (auto it = table[i].begin(); it != table[i].end(); ++it) {
                if (!first) out << " --> ";
                out << "< " << it->name << " : " << it->type << " >";
                first = false;
            }
            out << endl;
        }
    }

    int getTotalSymbols() const { return totalSymbols; }
};

class SymbolTable {
private:
    int bucketCount;
    ScopeTable* currentScope;
    int currentScopeId;

public:
    SymbolTable(int n) {
        bucketCount = n;
        currentScopeId = 1;
        currentScope = new ScopeTable(bucketCount, currentScopeId);
    }

    ~SymbolTable() {
        ScopeTable* cur = currentScope;
        while (cur != nullptr) {
            ScopeTable* parent = cur->getParentScope();
            delete cur;
            cur = parent;
        }
    }

    void EnterScope() {
        currentScopeId++;
        ScopeTable* newScope = new ScopeTable(bucketCount, currentScopeId);
        newScope->setParentScope(currentScope);
        currentScope = newScope;
    }

    void ExitScope() {
        if (currentScope->getParentScope() != nullptr) {
            ScopeTable* parent = currentScope->getParentScope();
            delete currentScope;
            currentScope = parent;
        }
    }

    int getCurrentId() const { return currentScopeId; }

    bool Insert(const string& name, const string& type) {
        return currentScope->Insert(name, type);
    }

    // returns pair of (position, scopeId); (-1, -1) if not found
    pair<int, int> LookupAll(const string& name) const {
        ScopeTable* cur = currentScope;
        while (cur != nullptr) {
            int pos = cur->Lookup(name);
            if (pos != -1) return make_pair(pos, cur->getId());
            cur = cur->getParentScope();
        }
        return make_pair(-1, -1);
    }

    void PrintCurrent(ostream& out) const {
        currentScope->Print(out);
    }

    void PrintAll(ostream& out) const {
        vector<ScopeTable*> scopes;
        ScopeTable* cur = currentScope;
        while (cur != nullptr) {
            scopes.push_back(cur);
            cur = cur->getParentScope();
        }
        for (int i = (int)scopes.size() - 1; i >= 0; i--) {
            scopes[i]->Print(out);
        }
    }
};

#endif
