#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "2205040_general.h"

using namespace std;

ostringstream out;

class SymbolInfo {
    string name;
    string type;
    string extraInfo;
    SymbolInfo* next;

public:
    SymbolInfo(string name, string type, string extraInfo = "")
        : name(name), type(type), extraInfo(extraInfo), next(nullptr) {}

    string getName() { return name; }
    string getType() { return type; }
    string getExtraInfo() { return extraInfo; }
    SymbolInfo* getNext() { return next; }

    void setNext(SymbolInfo* n) { next = n; }
};

class ScopeTable {
    mylist<SymbolInfo>* table;
    int total_buckets;
    int scope_id;
    ScopeTable* parentScope;

    unsigned int SDBMHash(const string& str) {
        unsigned int h = 0;
        for (unsigned int i = 0; i < str.length(); i++) {
            h = ((str[i]) + (h << 6) + (h << 16) - h) % total_buckets;
        }
        return h;
    }

    mylist<SymbolInfo>& getBucket(const string& key) {
        return table[SDBMHash(key)];
    }

    mylist<SymbolInfo>& getBucketByIndex(int i) {
        return table[i];
    }

    int getBucketIndex(const string& key) {
        return SDBMHash(key);
    }

    int getTotalBuckets() {
        return total_buckets;
    }

public:
    ScopeTable(int buckets, int id, ScopeTable* parent = nullptr)
        : total_buckets(buckets), scope_id(id), parentScope(parent) {
        table = new mylist<SymbolInfo>[buckets];
        out << "\tScopeTable# " << scope_id << " created" << endl;
    }

    int getScopeId() { return scope_id; }
    ScopeTable* getParentScope() { return parentScope; }

    bool Insert(string name, string type, string extra = "") {
        auto& bucket = getBucket(name);
        int bucketIdx = getBucketIndex(name) + 1;

        int pos = 0;
        bool found = false;

        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            pos++;
            if (it->getName() == name) {
                found = true;
                break;
            }
        }

        if (found) {
            out << "\t'" << name << "' already exists in the current ScopeTable" << endl;
            return false;
        }

        bucket.push_back(SymbolInfo(name, type, extra));
        out << "\tInserted in ScopeTable# " << scope_id
            << " at position " << bucketIdx << ", " << pos + 1 << endl;
        return true;
    }

    SymbolInfo* Lookup(string name, int& outBucket, int& outPos) {
        auto& bucket = getBucket(name);
        outBucket = getBucketIndex(name) + 1;
        outPos = 1;

        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->getName() == name) {
                return &(*it);
            }
            outPos++;
        }

        return nullptr;
    }

    bool Delete(string name) {
        auto& bucket = getBucket(name);
        int bucketIdx = getBucketIndex(name) + 1;
        int pos = 0;

        auto remover = [&](SymbolInfo& si) {
            pos++;
            return si.getName() == name;
        };

        if (bucket.remove_if(remover)) {
            out << "\tDeleted '" << name << "' from ScopeTable# " << scope_id << " at position " << bucketIdx << ", " << pos << endl;
            return true;
        }

        out << "\tNot found in the current ScopeTable" << endl;
        return false;
    }

    void Print(int depth, bool showAll) {
        for (int j = 0; j < depth; j++) out << "\t";
        out << "ScopeTable# " << scope_id << endl;

        for (int i = 0; i < getTotalBuckets(); i++) {
            for (int j = 0; j < depth; j++) out << "\t";
            out << (i + 1) << "--> ";

            getBucketByIndex(i).for_each([&](SymbolInfo& si) {
                out << "<" << si.getName() << "," << si.getType();
                if (!si.getExtraInfo().empty()) out << "," << si.getExtraInfo();
                out << "> ";
            });

            out << endl;
        }

        if (showAll && parentScope) {
            parentScope->Print(depth + 1, true);
        }
    }

    ~ScopeTable() {
        delete[] table;
        out << "\tScopeTable# " << scope_id << " removed" << endl;
    }
};

class SymbolTable {
    ScopeTable* currentScope;
    int bucketSize;
    int scopeCounter;

public:
    SymbolTable(int bucketSize) : currentScope(nullptr), bucketSize(bucketSize), scopeCounter(0) {
        EnterScope();
    }

    void EnterScope() {
        scopeCounter++;
        currentScope = new ScopeTable(bucketSize, scopeCounter, currentScope);
    }

    void ExitScope() {
        if (!currentScope) return;
        ScopeTable* temp = currentScope;
        currentScope = currentScope->getParentScope();
        delete temp;
    }

    bool Insert(string name, string type, string extra = "") {
        if (!currentScope) return false;
        return currentScope->Insert(name, type, extra);
    }

    bool Remove(string name) {
        if (!currentScope) return false;
        return currentScope->Delete(name);
    }

    SymbolInfo* Lookup(string name) {
        ScopeTable* sc = currentScope;
        int bucket = 0, pos = 0;
        while (sc) {
            SymbolInfo* found = sc->Lookup(name, bucket, pos);
            if (found) {
                out << "\t'" << name << "' found in ScopeTable# " << sc->getScopeId() << " at position " << bucket << ", " << pos << endl;
                return found;
            }
            sc = sc->getParentScope();
        }
        out << "\t'" << name << "' not found in any of the ScopeTables" << endl;
        return nullptr;
    }

    bool isRootScope() {
        return currentScope && !currentScope->getParentScope();
    }

    void PrintCurrent() {
        if (currentScope) currentScope->Print(1, false);
    }

    void PrintAll() {
        if (currentScope) currentScope->Print(1, true);
    }

    ~SymbolTable() {
        while (currentScope) ExitScope();
    }
};

void processCommand(string line, SymbolTable& st) {
    stringstream ss(line);
    string cmd, token1, token2;
    ss >> cmd;

    if (cmd == "I") {
        ss >> token1 >> token2;
        string remaining, word;
        while (ss >> word) {
            if (!remaining.empty()) remaining += ",";
            remaining += word;
        }
        if (token2 == "FUNCTION") {
            if (!remaining.empty()) {
                size_t first_comma = remaining.find(',');
                string retType = remaining.substr(0, first_comma);
                string params = remaining.substr(first_comma + 1);
                string formattedParams = "(";
                stringstream pss(params);
                string p;
                while (getline(pss, p, ',')) {
                    if (formattedParams != "(") formattedParams += ",";
                    formattedParams += p;
                }
                formattedParams += ")";
                st.Insert(token1, token2, retType + "<==" + formattedParams);
                return;
            }
        } else if (token2 == "STRUCT" || token2 == "UNION") {
            string formatted = "{";
            stringstream pss(remaining);
            string type, name;
            bool first = true;
            while (getline(pss, type, ',') && getline(pss, name, ',')) {
                if (!first) formatted += ",";
                formatted += "(" + type + "," + name + ")";
                first = false;
            }
            formatted += "}";
            st.Insert(token1, token2, formatted);
            return;
        }
        st.Insert(token1, token2, remaining);
    } else if (cmd == "L") {
        ss >> token1;
        if (ss >> token2) {
            out << "\tNumber of parameters mismatch for the command L" << endl;
        } else {
            st.Lookup(token1);
        }
    } else if (cmd == "D") {
        if (!(ss >> token1)) {
            out << "\tNumber of parameters mismatch for the command D" << endl;
        } else {
            st.Remove(token1);
        }
    } else if (cmd == "S") {
        st.EnterScope();
    } else if (cmd == "E") {
        st.ExitScope();
    } else if (cmd == "P") {
        ss >> token1;
        if (token1 == "C") st.PrintCurrent();
        else if (token1 == "A") st.PrintAll();
    }
}

bool check_diff(string file1, string file2) {
    ifstream f1(file1), f2(file2);
    if (!f1.is_open() || !f2.is_open()) {
        cout << "\tCould not open one or both files" << endl;
        return true;
    }

    string l1, l2;
    int lineNum = 1;
    bool diffFound = false;

    while (getline(f1, l1) && getline(f2, l2)) {
        if (l1 != l2) {
            cout << "\tDifference at line " << lineNum << ":" << endl;
            cout << "\tFile1: " << l1 << endl;
            cout << "\tFile2: " << l2 << endl;
            diffFound = true;
        }
        lineNum++;
    }

    while (getline(f1, l1)) {
        cout << "\tExtra line in " << file1 << " at line " << lineNum << ": " << l1 << endl;
        diffFound = true;
        lineNum++;
    }

    while (getline(f2, l2)) {
        cout << "\tExtra line in " << file2 << " at line " << lineNum << ": " << l2 << endl;
        diffFound = true;
        lineNum++;
    }

    if (!diffFound) {
        cout << "\tFiles are identical (" << (lineNum - 1) << " lines)" << endl;
    }

    return diffFound;
}

int main(int argc, char* argv[]) {
    string inputFile = (argc >= 2) ? argv[1] : "sample_input.txt";
    string outputFile = (argc >= 3) ? argv[2] : "new_output.txt";

    ifstream inFile(inputFile);
    if (!inFile) {
        cerr << "Unable to open " << inputFile << endl;
        return 1;
    }

    string line;
    getline(inFile, line);
    int bucketSize = stoi(line);

    {
        SymbolTable st(bucketSize);
        int cmdCount = 1;

        while (getline(inFile, line)) {
            if (line.empty()) continue;
            size_t end = line.find_last_not_of(" \t\n\r");
            if (end != string::npos) line = line.substr(0, end + 1);
            if (line[0] == 'Q') {
                out << "Cmd " << cmdCount << ": Q" << endl;
                break;
            }
            if (line[0] == 'E' && st.isRootScope()) continue;

            stringstream ss(line);
            string cmd;
            ss >> cmd;
            if (cmd == "P") {
                string sub;
                ss >> sub;
                if (sub != "C" && sub != "A") continue;
            } else if (cmd != "I" && cmd != "L" && cmd != "D" && cmd != "S" && cmd != "E") {
                continue;
            }

            out << "Cmd " << cmdCount << ": " << line << endl;
            processCommand(line, st);
            cmdCount++;
        }
    }

    ofstream fout(outputFile);
    fout << out.str();
    fout.close();

    //cout << "--- Comparing with sample output ---" << endl;
   // check_diff("sample_output.txt", outputFile);

    inFile.close();
    return 0;
}
