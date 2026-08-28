#include "PeepholeOptimizer.h"
#include <algorithm>
#include <sstream>
#include <cctype>

PeepholeOptimizer::PeepholeOptimizer() : optimizationsCount(0) {}

PeepholeOptimizer::~PeepholeOptimizer() {}

string PeepholeOptimizer::trim(const string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

bool PeepholeOptimizer::isLabel(const string& line) {
    string trimmed = trim(line);
    return !trimmed.empty() && trimmed.back() == ':';
}

bool PeepholeOptimizer::isEmptyOrComment(const string& line) {
    string trimmed = trim(line);
    return trimmed.empty() || trimmed[0] == ';';
}

string PeepholeOptimizer::normalizeInstruction(const string& line) {
    string trimmed = trim(line);
    // Remove comments
    size_t commentPos = trimmed.find(';');
    if (commentPos != string::npos) {
        trimmed = trimmed.substr(0, commentPos);
    }
    // Normalize whitespace
    string result;
    bool lastWasSpace = false;
    for (char c : trimmed) {
        if (c == ' ' || c == '\t') {
            if (!lastWasSpace) {
                result += ' ';
                lastWasSpace = true;
            }
        } else {
            result += c;
            lastWasSpace = false;
        }
    }
    // Convert to uppercase for comparison
    transform(result.begin(), result.end(), result.begin(), ::toupper);
    return trim(result);
}

// Rule (i): Remove redundant consecutive MOV instructions
// MOV AX, a followed by MOV a, AX is redundant
bool PeepholeOptimizer::isRedundantMOV(const string& line1, const string& line2) {
    string n1 = normalizeInstruction(line1);
    string n2 = normalizeInstruction(line2);

    // Check for: MOV dest, src  /  MOV src, dest  pattern
    // Pattern: MOV X, Y / MOV Y, X
    if (n1.substr(0, 4) == "MOV " && n2.substr(0, 4) == "MOV ") {
        string rest1 = n1.substr(4);
        string rest2 = n2.substr(4);

        size_t comma1 = rest1.find(',');
        size_t comma2 = rest2.find(',');
        if (comma1 == string::npos || comma2 == string::npos) return false;

        string dest1 = trim(rest1.substr(0, comma1));
        string src1 = trim(rest1.substr(comma1 + 1));
        string dest2 = trim(rest2.substr(0, comma2));
        string src2 = trim(rest2.substr(comma2 + 1));

        // MOV X, Y followed by MOV Y, X
        if (dest1 == src2 && src1 == dest2) {
            return true;
        }
    }
    return false;
}

// Rule (ii): Remove redundant consecutive PUSH and POP of the same register
bool PeepholeOptimizer::isRedundantPushPop(const string& line1, const string& line2) {
    string n1 = normalizeInstruction(line1);
    string n2 = normalizeInstruction(line2);

    if (n1.substr(0, 5) == "PUSH " && n2.substr(0, 4) == "POP ") {
        string src = trim(n1.substr(5));
        string dest = trim(n2.substr(4));
        return src == dest;
    }
    return false;
}

// Rule (iii): Remove redundant arithmetic operations
// ADD X, 0 or MUL X, 1
bool PeepholeOptimizer::isRedundantArithmetic(const string& line) {
    string n = normalizeInstruction(trim(line));

    // ADD reg, 0
    if (n.substr(0, 4) == "ADD ") {
        string rest = trim(n.substr(4));
        size_t comma = rest.find(',');
        if (comma != string::npos) {
            string operand = trim(rest.substr(comma + 1));
            if (operand == "0") return true;
        }
    }

    // SUB reg, 0
    if (n.substr(0, 4) == "SUB ") {
        string rest = trim(n.substr(4));
        size_t comma = rest.find(',');
        if (comma != string::npos) {
            string operand = trim(rest.substr(comma + 1));
            if (operand == "0") return true;
        }
    }

    // MUL reg, 1 or IMUL reg, 1
    if (n.substr(0, 4) == "MUL ") {
        string rest = trim(n.substr(4));
        size_t comma = rest.find(',');
        if (comma != string::npos) {
            string operand = trim(rest.substr(comma + 1));
            if (operand == "1") return true;
        }
    }

    // AND reg, -1 (all bits set, no-op)
    if (n.substr(0, 4) == "AND ") {
        string rest = trim(n.substr(4));
        size_t comma = rest.find(',');
        if (comma != string::npos) {
            string operand = trim(rest.substr(comma + 1));
            if (operand == "-1" || operand == "0FFFFFFFF" || operand == "0FF") return true;
        }
    }

    // XOR reg, 0
    if (n.substr(0, 4) == "XOR ") {
        string rest = trim(n.substr(4));
        size_t comma = rest.find(',');
        if (comma != string::npos) {
            string op1 = trim(rest.substr(0, comma));
            string op2 = trim(rest.substr(comma + 1));
            if (op1 == op2) return true; // XOR reg, reg (zeroing is intentional but let's keep)
        }
    }

    return false;
}

// Rule (iv): Remove redundant consecutive labels
bool PeepholeOptimizer::isConsecutiveLabels(const string& line1, const string& line2) {
    return isLabel(line1) && isLabel(line2);
}

void PeepholeOptimizer::loadFile(const string& inputFileName) {
    ifstream inFile(inputFileName);
    if (!inFile.is_open()) {
        cerr << "Error: Cannot open input file " << inputFileName << endl;
        return;
    }
    string line;
    while (getline(inFile, line)) {
        lines.push_back(line);
    }
    inFile.close();
}

void PeepholeOptimizer::optimize() {
    optimizationsCount = 0;
    bool changed = true;

    while (changed) {
        changed = false;
        vector<string> newLines;
        vector<bool> removed(lines.size(), false);

        for (size_t i = 0; i < lines.size(); i++) {
            if (removed[i]) continue;

            // Rule (i): Redundant MOV
            if (i + 1 < lines.size() && !removed[i + 1]) {
                if (isRedundantMOV(lines[i], lines[i + 1])) {
                    removed[i + 1] = true;
                    optimizationsCount++;
                    changed = true;
                    continue;
                }
            }

            // Rule (ii): Redundant PUSH/POP
            if (i + 1 < lines.size() && !removed[i + 1]) {
                if (isRedundantPushPop(lines[i], lines[i + 1])) {
                    removed[i] = true;
                    removed[i + 1] = true;
                    optimizationsCount++;
                    changed = true;
                    continue;
                }
            }

            // Rule (iii): Redundant arithmetic
            if (isRedundantArithmetic(lines[i])) {
                removed[i] = true;
                optimizationsCount++;
                changed = true;
                continue;
            }

            // Rule (iv): Consecutive labels (keep first, remove rest)
            if (i + 1 < lines.size() && !removed[i + 1]) {
                if (isConsecutiveLabels(lines[i], lines[i + 1])) {
                    // Remove the second label (and any more consecutive labels)
                    size_t j = i + 1;
                    while (j < lines.size() && isLabel(lines[j])) {
                        removed[j] = true;
                        optimizationsCount++;
                        j++;
                    }
                    changed = true;
                    continue;
                }
            }
        }

        for (size_t i = 0; i < lines.size(); i++) {
            if (!removed[i]) {
                newLines.push_back(lines[i]);
            }
        }
        lines = newLines;
    }
}

void PeepholeOptimizer::writeOutput(const string& outputFileName) {
    ofstream outFile(outputFileName);
    if (!outFile.is_open()) {
        cerr << "Error: Cannot open output file " << outputFileName << endl;
        return;
    }
    for (const string& line : lines) {
        outFile << line << endl;
    }
    outFile.close();
}

int PeepholeOptimizer::getOptimizationsCount() const {
    return optimizationsCount;
}
