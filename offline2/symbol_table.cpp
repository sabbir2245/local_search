#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <string>
#include <iostream>
#include <vector>
#include "general.h"

using namespace std;

unsigned int sdbmHash(const string& p, int mod) {
    unsigned int hash = 0;
    auto *str = (const unsigned char *)p.c_str();
    int c{};
    while ((c = *str++)) {
        hash = (c + (hash << 6) + (hash << 16) - hash) % mod;
    }
    return hash;
}

class SymbolInfo {
    string name;
    string type;
public:
    SymbolInfo() : name(""), type("") {}
    SymbolInfo(const string& name, const string& type) : name(name), type(type) {}
    string getName() const { return name; }
    string getType() const { return type; }
};

class ScopeTable {
    mylist<SymbolInfo>* buckets;
    int totalBuckets;
    string scopeId;
    ScopeTable* parent;
    int childCount;

public:
    ScopeTable(int nBuckets, const string& id, ScopeTable* p)
        : totalBuckets(nBuckets), scopeId(id), parent(p), childCount(0) {
        buckets = new mylist<SymbolInfo>[totalBuckets];
    }

    string getId() const { return scopeId; }
    ScopeTable* getParent() const { return parent; }
    int getNextChildNum() { return ++childCount; }

    pair<int,int> Insert(const string& name, const string& type) {
        unsigned int idx = sdbmHash(name, totalBuckets);
        int pos = 0;
        for (auto it = buckets[idx].begin(); it != buckets[idx].end(); ++it) {
            pos++;
            if (it->getName() == name) return {-1, -1};
        }
        buckets[idx].push_back(SymbolInfo(name, type));
        return {(int)idx + 1, pos + 1};
    }

    pair<int,int> Lookup(const string& name) {
        unsigned int idx = sdbmHash(name, totalBuckets);
        int pos = 0;
        for (auto it = buckets[idx].begin(); it != buckets[idx].end(); ++it) {
            pos++;
            if (it->getName() == name) return {(int)idx + 1, pos};
        }
        return {-1, -1};
    }

    void PrintNonEmpty(ostream& out) {
        for (int i = 0; i < totalBuckets; i++) {
            bool first = true;
            string bucketLine;
            for (auto it = buckets[i].begin(); it != buckets[i].end(); ++it) {
                if (first) {
                    bucketLine += to_string(i + 1) + " --> < " + it->getName() + " : " + it->getType() + " >";
                    first = false;
                } else {
                    bucketLine += "< " + it->getName() + " : " + it->getType() + " >";
                }
            }
            if (!first) out << bucketLine << endl;
        }
    }

    ~ScopeTable() { delete[] buckets; }
};

class SymbolTable {
    ScopeTable* current;
    int bucketSize;

public:
    SymbolTable(int bs) : current(nullptr), bucketSize(bs) {
        current = new ScopeTable(bucketSize, "1", nullptr);
    }

    void EnterScope() {
        if (!current) return;
        int childNum = current->getNextChildNum();
        string newId = current->getId() + "." + to_string(childNum);
        current = new ScopeTable(bucketSize, newId, current);
    }

    string getCurrentId() const { return current ? current->getId() : "?"; }

    void ExitScope() {
        if (!current) return;
        ScopeTable* temp = current;
        current = current->getParent();
        delete temp;
    }

    pair<int,int> Insert(const string& name, const string& type) {
        if (!current) return {-1, -1};
        return current->Insert(name, type);
    }

    pair<int,int> LookupAll(const string& name) {
        ScopeTable* s = current;
        while (s) {
            auto res = s->Lookup(name);
            if (res.first != -1) return res;
            s = s->getParent();
        }
        return {-1, -1};
    }

    void PrintAll(ostream& out) {
        vector<ScopeTable*> scopes;
        ScopeTable* s = current;
        while (s) {
            scopes.push_back(s);
            s = s->getParent();
        }
        for (auto* sc : scopes) {
            out << "ScopeTable # " << sc->getId() << endl;
            sc->PrintNonEmpty(out);
        }
    }

    ~SymbolTable() {
        while (current) {
            ScopeTable* temp = current;
            current = current->getParent();
            delete temp;
        }
    }
};

#endif
