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
    bool isRedundantJmpToNextLabel(const string& line1, const string& line2);
    bool isRedundantLoadAfterStore(const string& line1, const string& line2);
    bool isConstantStoreSimplifiable(const string& line1, const string& line2);
    bool isEmptyLine(const string& line);
    string normalizeInstruction(const string& line);
    string getLabelName(const string& line);
    bool isLabelReferenced(const string& label);

    bool foldPushOverwritePop(const string& movSrcLine, const string& pushLine,
                               const string& movImmLine, const string& popLine,
                               string& replacement);
    bool isDeadRegisterOverwrite(const string& afterPopLine, const string& reg);
    int  findMatchingPop(size_t pushIndex, const string& reg, const vector<bool>& removed);

public:
    PeepholeOptimizer();
    ~PeepholeOptimizer();

    void loadFile(const string& inputFileName);
    void optimize();
    void writeOutput(const string& outputFileName);
    int getOptimizationsCount() const;
};

#endif
