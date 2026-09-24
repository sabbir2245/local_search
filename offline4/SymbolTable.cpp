#include <string>
#include <vector>
#include <iostream>

using namespace std ;



class SymbolInfo {
private:
    string name;
    string type;
    
    
    string identityType; 
    int stackOffset;          
    bool isGlobal;            
    int arraySize;            

public:
    SymbolInfo* next;

    SymbolInfo(const string& name, const string& type)
        : name(name), type(type), identityType("VAR"), stackOffset(0), 
          isGlobal(false), arraySize(0), next(nullptr) {}

    string getName() const { return name; }
    string getType() const { return type; }
    void setName(const string& n) { name = n; }
    void setType(const string& t) { type = t; }

    string getIdentityType() const { return identityType; }
    void setIdentityType(const string& idType) { identityType = idType; }

    int getStackOffset() const { return stackOffset; }
    void setStackOffset(int offset) { stackOffset = offset; }

    bool getIsGlobal() const { return isGlobal; }
    void setIsGlobal(bool global) { isGlobal = global; }

    int getArraySize() const { return arraySize; }
    void setArraySize(int size) { arraySize = size; }
};




class ScopeTable {
private:
    int numBuckets;
    int id;
    vector<SymbolInfo*> table;
    int currentStackOffset; 

    unsigned long hashFunction(const string& name) const {
        unsigned long hash = 0;
        for (char c : name) {
            hash = c + (hash << 6) + (hash << 16) - hash;
        }
        return hash % numBuckets;
    }

public:
    ScopeTable* parentScope;

    ScopeTable(int numBuckets, int id, ScopeTable* parent = nullptr)
        : numBuckets(numBuckets), id(id), currentStackOffset(0), parentScope(parent) {
        table.resize(numBuckets, nullptr);
        if (parent != nullptr) {
            currentStackOffset = parent->getCurrentStackOffset();
        }
    }

    ~ScopeTable() {
        for (int i = 0; i < numBuckets; ++i) {
            SymbolInfo* curr = table[i];
            while (curr != nullptr) {
                SymbolInfo* temp = curr;
                curr = curr->next;
                delete temp;
            }
        }
    }

    int getId() const { return id; }
    int getCurrentStackOffset() const { return currentStackOffset; }
    void setCurrentStackOffset(int offset) { currentStackOffset = offset; }

    int allocateStackMemory(int numBytes = 4) {
        currentStackOffset += numBytes;
        return currentStackOffset;
    }

    bool insert(SymbolInfo* symbol) {
        unsigned long idx = hashFunction(symbol->getName());
        SymbolInfo* curr = table[idx];

        if (curr == nullptr) {
            table[idx] = symbol;
            return true;
        }

        while (curr != nullptr) {
            if (curr->getName() == symbol->getName()) {
                return false;
            }
            if (curr->next == nullptr) break;
            curr = curr->next;
        }
        curr->next = symbol;
        return true;
    }

    SymbolInfo* lookUp(const string& name) {
        unsigned long idx = hashFunction(name);
        SymbolInfo* curr = table[idx];
        while (curr != nullptr) {
            if (curr->getName() == name) {
                return curr;
            }
            curr = curr->next;
        }
        return nullptr;
    }
};




class SymbolTable {
private:
    ScopeTable* currentScope;
    int numBuckets;
    int scopeCount;

public:
    SymbolTable(int numBuckets = 11) : numBuckets(numBuckets), scopeCount(0) {
        currentScope = nullptr;
        enterScope(); 
    }

    ~SymbolTable() {
        while (currentScope != nullptr) {
            exitScope();
        }
    }

    void enterScope() {
        scopeCount++;
        ScopeTable* newScope = new ScopeTable(numBuckets, scopeCount, currentScope);
        currentScope = newScope;
    }

    void exitScope() {
        if (currentScope == nullptr) return;
        ScopeTable* temp = currentScope;
        currentScope = currentScope->parentScope;
        delete temp;
    }

    bool insert(const string& name, const string& type) {
        SymbolInfo* symbol = new SymbolInfo(name, type);
        if (currentScope->getId() == 1) {
            symbol->setIsGlobal(true);
        }
        bool inserted = currentScope->insert(symbol);
        if (!inserted) {
            delete symbol;
        }
        return inserted;
    }

    bool insert(SymbolInfo* symbol) {
        if (currentScope->getId() == 1) {
            symbol->setIsGlobal(true);
        }
        bool inserted = currentScope->insert(symbol);
        if (!inserted) {
            delete symbol;
        }
        return inserted;
    }

    SymbolInfo* lookUp(const string& name) {
        ScopeTable* curr = currentScope;
        while (curr != nullptr) {
            SymbolInfo* found = curr->lookUp(name);
            if (found != nullptr) return found;
            curr = curr->parentScope;
        }
        return nullptr;
    }

    ScopeTable* getCurrentScope() const { return currentScope; }

    int allocateLocalVar(const string& name, const string& type, int sizeInBytes = 4) {
        SymbolInfo* symbol = new SymbolInfo(name, type);
        symbol->setIdentityType("VAR");
        
        if (currentScope->getId() == 1) {
            symbol->setIsGlobal(true);
            symbol->setStackOffset(0);
        } else {
            symbol->setIsGlobal(false);
            int offset = currentScope->allocateStackMemory(sizeInBytes);
            symbol->setStackOffset(-offset);
        }

        if (!currentScope->insert(symbol)) {
            delete symbol;
            return -1;
        }
        return symbol->getStackOffset();
    }

    int allocateLocalArray(const string& name, const string& type, int arrayLength) {
        SymbolInfo* symbol = new SymbolInfo(name, type);
        symbol->setIdentityType("ARRAY");
        symbol->setArraySize(arrayLength);

        if (currentScope->getId() == 1) {
            symbol->setIsGlobal(true);
            symbol->setStackOffset(0);
        } else {
            symbol->setIsGlobal(false);
            int totalBytes = arrayLength * 4;
            int offset = currentScope->allocateStackMemory(totalBytes);
            symbol->setStackOffset(-offset);
        }

        if (!currentScope->insert(symbol)) {
            delete symbol;
            return -1;
        }
        return symbol->getStackOffset();
    }
};
