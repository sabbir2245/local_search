#ifndef PEEPHOLEOPTIMIZER_H
#define PEEPHOLEOPTIMIZER_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>

using namespace std;

class PeepholeOptimizer {
private:
    vector<string> lines;
    int optimizationsCount;

    string trim(const string& str);
    bool isLabel(const string& line);
    bool isEmptyOrComment(const string& line);
    bool isRedundantMOV(const string& line1, const string& line2);
    bool isRedundantPushPop(const string& line1, const string& line2);
    bool isRedundantArithmetic(const string& line);
    bool isConsecutiveLabels(const string& line1, const string& line2);
    string normalizeInstruction(const string& line);

public:
    PeepholeOptimizer();
    ~PeepholeOptimizer();

    void loadFile(const string& inputFileName);
    void optimize();
    void writeOutput(const string& outputFileName);
    int getOptimizationsCount() const;
};

#endif
