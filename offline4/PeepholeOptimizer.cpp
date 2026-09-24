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
    size_t commentPos = trimmed.find(';');
    if (commentPos != string::npos) {
        trimmed = trimmed.substr(0, commentPos);
    }
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
    transform(result.begin(), result.end(), result.begin(), ::toupper);
    return trim(result);
}


bool PeepholeOptimizer::isRedundantMOV(const string& line1, const string& line2) {
    string n1 = normalizeInstruction(line1);
    string n2 = normalizeInstruction(line2);

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

        if (dest1 == src2 && src1 == dest2) {
            return true;
        }
    }
    return false;
}


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


bool PeepholeOptimizer::isRedundantArithmetic(const string& line) {
    string n = normalizeInstruction(trim(line));

    if (n.substr(0, 4) == "ADD ") {
        string rest = trim(n.substr(4));
        size_t comma = rest.find(',');
        if (comma != string::npos) {
            string operand = trim(rest.substr(comma + 1));
            if (operand == "0") return true;
        }
    }

    if (n.substr(0, 4) == "SUB ") {
        string rest = trim(n.substr(4));
        size_t comma = rest.find(',');
        if (comma != string::npos) {
            string operand = trim(rest.substr(comma + 1));
            if (operand == "0") return true;
        }
    }

    if (n.substr(0, 4) == "MUL ") {
        string rest = trim(n.substr(4));
        size_t comma = rest.find(',');
        if (comma != string::npos) {
            string operand = trim(rest.substr(comma + 1));
            if (operand == "1") return true;
        }
    }

    if (n.substr(0, 4) == "AND ") {
        string rest = trim(n.substr(4));
        size_t comma = rest.find(',');
        if (comma != string::npos) {
            string operand = trim(rest.substr(comma + 1));
            if (operand == "-1" || operand == "0FFFFFFFF" || operand == "0FF") return true;
        }
    }

    
    
    

    return false;
}


bool PeepholeOptimizer::isConsecutiveLabels(const string& line1, const string& line2) {
    return isLabel(line1) && isLabel(line2);
}


bool PeepholeOptimizer::isRedundantJmpToNextLabel(const string& line1, const string& line2) {
    string n1 = normalizeInstruction(trim(line1));
    string n2 = normalizeInstruction(trim(line2));

    if (n1.substr(0, 4) == "JMP ") {
        string target = trim(n1.substr(4));
        string label2 = trim(n2);
        if (!label2.empty() && label2.back() == ':') {
            string labelName = label2.substr(0, label2.size() - 1);
            if (target == labelName) return true;
        }
    }
    return false;
}


bool PeepholeOptimizer::isRedundantLoadAfterStore(const string& line1, const string& line2) {
    string n1 = normalizeInstruction(trim(line1));
    string n2 = normalizeInstruction(trim(line2));

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

        if (dest1[0] == '[' && dest1.back() == ']' && src2[0] == '[' && src2.back() == ']') {
            if (src1 == dest2 && dest1 == src2) return true;
        }
    }
    return false;
}


bool PeepholeOptimizer::isConstantStoreSimplifiable(const string& line1, const string& line2) {
    string n1 = normalizeInstruction(trim(line1));
    string n2 = normalizeInstruction(trim(line2));

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

        if (dest1[0] != '[' && src1[0] != '[' && dest2[0] == '[' && dest2.back() == ']' && src2 == dest1) {
            if (!src1.empty() && (isdigit(src1[0]) || src1[0] == '-' || src1[0] == '+')) {
                return true;
            }
        }
    }
    return false;
}


bool PeepholeOptimizer::foldPushOverwritePop(const string& movSrcLine, const string& pushLine,
                                              const string& movImmLine, const string& popLine,
                                              string& replacement) {
    string n0 = normalizeInstruction(movSrcLine);
    string n1 = normalizeInstruction(pushLine);
    string n2 = normalizeInstruction(movImmLine);
    string n3 = normalizeInstruction(popLine);

    if (n0.substr(0, 4) != "MOV " || n1.substr(0, 5) != "PUSH " ||
        n2.substr(0, 4) != "MOV " || n3.substr(0, 4) != "POP ") return false;

    size_t c0 = n0.substr(4).find(',');
    if (c0 == string::npos) return false;
    string ra0 = trim(n0.substr(4, c0));

    if (trim(n1.substr(5)) != ra0) return false;

    size_t c2 = n2.substr(4).find(',');
    if (c2 == string::npos) return false;
    if (trim(n2.substr(4, c2)) != ra0) return false;

    string rb = trim(n3.substr(4));
    if (rb == ra0) return false;

    
    string origTrimmed = trim(movSrcLine);
    size_t origComma = origTrimmed.find(',');
    if (origComma == string::npos) return false;
    string src = trim(origTrimmed.substr(origComma + 1));

    if (src.find("ESP") != string::npos) return false;
    if (src.find(rb) != string::npos) return false;

    replacement = "    MOV  " + rb + ", " + src;
    return true;
}


int PeepholeOptimizer::findMatchingPop(size_t pushIndex, const string& reg, const vector<bool>& removed) {
    const size_t maxLookahead = 6;
    for (size_t j = pushIndex + 1; j < lines.size() && j <= pushIndex + maxLookahead; j++) {
        if (removed[j]) return -1;
        string n = normalizeInstruction(lines[j]);
        if (n.substr(0, 5) == "PUSH " && trim(n.substr(5)) == reg) return -1;
        if (n.substr(0, 4) == "POP " && trim(n.substr(4)) == reg) return (int)j;
        if (isLabel(lines[j])) return -1;
    }
    return -1;
}



bool PeepholeOptimizer::isDeadRegisterOverwrite(const string& afterPopLine, const string& reg) {
    string n = normalizeInstruction(afterPopLine);
    if (n.substr(0, 4) != "MOV ") return false;
    size_t comma = n.substr(4).find(',');
    if (comma == string::npos) return false;
    string dest = trim(n.substr(4, comma));
    string src = trim(n.substr(4 + comma + 1));
    if (dest != reg) return false;
    return src.find(reg) == string::npos;
}


bool PeepholeOptimizer::isEmptyLine(const string& line) {
    string trimmed = trim(line);
    return trimmed.empty();
}

string PeepholeOptimizer::getLabelName(const string& line) {
    string trimmed = trim(line);
    if (!trimmed.empty() && trimmed.back() == ':') {
        return trimmed.substr(0, trimmed.size() - 1);
    }
    return "";
}

bool PeepholeOptimizer::isLabelReferenced(const string& label) {
    string normLabel = label;
    transform(normLabel.begin(), normLabel.end(), normLabel.begin(), ::toupper);
    for (const string& line : lines) {
        string trimmed = trim(line);
        if (trimmed.empty() || trimmed[0] == ';') continue;
        string norm = normalizeInstruction(trimmed);
        if (norm.empty()) continue;
        string labelDef = normalizeInstruction(label + ":");
        if (norm == labelDef) continue;
        size_t pos = norm.find(normLabel);
        if (pos == string::npos) continue;
        size_t end = pos + normLabel.size();
        bool afterOk = (end >= norm.size()) || (norm[end] == ' ') || (norm[end] == ',') || (norm[end] == ':');
        bool beforeOk = (pos == 0) || (norm[pos - 1] == ' ') || (norm[pos - 1] == ',');
        if (beforeOk && afterOk) return true;
    }
    return false;
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

            
            if (isEmptyLine(lines[i])) {
                removed[i] = true;
                optimizationsCount++;
                changed = true;
                continue;
            }

            
            if (i + 3 < lines.size() && !removed[i + 1] && !removed[i + 2] && !removed[i + 3]) {
                string replacement;
                if (foldPushOverwritePop(lines[i], lines[i + 1], lines[i + 2], lines[i + 3], replacement)) {
                    lines[i] = replacement;
                    removed[i + 1] = true;
                    removed[i + 3] = true;
                    optimizationsCount++;
                    changed = true;
                    continue;
                }
            }

            
            {
                string ni = normalizeInstruction(lines[i]);
                if (ni.substr(0, 5) == "PUSH ") {
                    string reg = trim(ni.substr(5));
                    int popIdx = findMatchingPop(i, reg, removed);
                    if (popIdx != -1) {
                        size_t after = (size_t)popIdx + 1;
                        while (after < lines.size() && (removed[after] || isLabel(lines[after]))) after++;
                        if (after < lines.size() && isDeadRegisterOverwrite(lines[after], reg)) {
                            removed[i] = true;
                            removed[popIdx] = true;
                            optimizationsCount++;
                            changed = true;
                            continue;
                        }
                    }
                }
            }

            
            if (i + 1 < lines.size() && !removed[i + 1]) {
                if (isRedundantMOV(lines[i], lines[i + 1])) {
                    removed[i + 1] = true;
                    optimizationsCount++;
                    changed = true;
                    continue;
                }
            }

            
            if (i + 1 < lines.size() && !removed[i + 1]) {
                if (isRedundantPushPop(lines[i], lines[i + 1])) {
                    removed[i] = true;
                    removed[i + 1] = true;
                    optimizationsCount++;
                    changed = true;
                    continue;
                }
            }

            
            if (isRedundantArithmetic(lines[i])) {
                removed[i] = true;
                optimizationsCount++;
                changed = true;
                continue;
            }

            
            if (i + 1 < lines.size() && !removed[i + 1]) {
                if (isRedundantJmpToNextLabel(lines[i], lines[i + 1])) {
                    removed[i] = true;
                    optimizationsCount++;
                    changed = true;
                    continue;
                }
            }

            
            if (i + 1 < lines.size() && !removed[i + 1]) {
                if (isRedundantLoadAfterStore(lines[i], lines[i + 1])) {
                    removed[i + 1] = true;
                    optimizationsCount++;
                    changed = true;
                    continue;
                }
            }

            
            if (i + 1 < lines.size() && !removed[i + 1]) {
                if (isConstantStoreSimplifiable(lines[i], lines[i + 1])) {
                    string n1 = normalizeInstruction(trim(lines[i]));
                    string rest1 = n1.substr(4);
                    size_t comma1 = rest1.find(',');
                    string src1 = trim(rest1.substr(comma1 + 1));

                    string orig2 = lines[i + 1];
                    size_t origComma = orig2.find(',');
                    if (origComma != string::npos) {
                        string before = orig2.substr(0, origComma);
                        size_t bracketPos = before.find('[');
                        if (bracketPos != string::npos) {
                            before.insert(bracketPos, "dword ");
                        }
                        lines[i + 1] = before + ", " + src1;
                    }
                    removed[i] = true;
                    optimizationsCount++;
                    changed = true;
                    continue;
                }
            }

            
            if (i + 1 < lines.size() && !removed[i + 1]) {
                if (isConsecutiveLabels(lines[i], lines[i + 1])) {
                    size_t j = i + 1;
                    while (j < lines.size() && isLabel(lines[j])) {
                        string lbl = getLabelName(lines[j]);
                        if (!lbl.empty() && !isLabelReferenced(lbl)) {
                            removed[j] = true;
                            optimizationsCount++;
                            changed = true;
                        }
                        j++;
                    }
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
