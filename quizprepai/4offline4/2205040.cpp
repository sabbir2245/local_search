#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <chrono>
#include <random>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <numeric>
#include <map>



//   g++ -O2 -std=c++17 -o tsp tsp.cpp
//
// Usage:
//   ./tsp solve <inputfile> [--t0 V] [--alpha V] [--tmin V] [--iterT N]
//                           [--maxIter N] [--init random|greedy] [--seed N]
//   ./tsp gen <N> <outputfile> [--seed N]          (generate a random instance)
//   ./tsp exp1 <file1> <file2> ...                 (Experiment 1: Greedy vs SA)
//   ./tsp exp2 <inputfile>                         (Experiment 2: vary T0)
//   ./tsp exp3 <inputfile>                         (Experiment 3: vary alpha)
//   ./tsp exp4 <inputfile>                         (Experiment 4: random vs greedy init)
//   ./tsp all                                      (generate 5 instances + run all experiments)
// ============================================================================


using namespace std;
using Arr    = vector<int>;
using Matrix = vector<vector<double>>;
struct Result {
    string name;
    Arr    tour;
    Arr    initTour;
    double cost    = 0.0;
    double timeSec = 0.0;
    bool   isSA               = false;
    double initCost           = 0.0;
    double initTemp           = 0.0;
    double coolingRate        = 0.0;
    long long totalIter       = 0;
    long long acceptedMoves   = 0;
    long long worseMovesAccepted = 0;
};
bool readInput(istream& in, Matrix& cost, int& n) {
    if (!(in >> n)) return false;
    cost.assign(n, vector<double>(n, 0.0));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (!(in >> cost[i][j])) return false;
    return true;
}
bool readInput(const string& filename, Matrix& cost, int& n) {
    ifstream f(filename);
    if (!f.is_open()) {
        cerr << "Error: cannot open input file: " << filename << "\n";
        return false;
    }
    return readInput(f, cost, n);
}
double calculateTourCost(const Matrix& cost, const Arr& tour) {
    double total = 0.0;
    int n = (int)tour.size();
    for (int i = 0; i < n; i++) {
        int u = tour[i];
        int v = tour[(i + 1) % n];
        total += cost[u][v];
    }
    return total;
}
bool isValidTour(const Matrix& cost, const Arr& tour, int start = 0) {
    int n = (int)cost.size();
    if ((int)tour.size() != n) return false;
    if (tour.empty() || tour[0] != start) return false;
    vector<bool> seen(n, false);
    for (int c : tour) {
        if (c < 0 || c >= n) return false;
        if (seen[c]) return false;
        seen[c] = true;
    }
    for (bool b : seen) if (!b) return false;
    return true;
}
Arr greedyTSP(const Matrix& cost, int start = 0) {
    int n = (int)cost.size();
    vector<bool> visited(n, false);
    Arr tour;
    tour.reserve(n);
    int current = start;
    visited[current] = true;
    tour.push_back(current);
    for (int step = 1; step < n; step++) {
        int best = -1;
        double bestCost = -1.0;
        for (int j = 0; j < n; j++) {
            if (!visited[j] && (best == -1 || cost[current][j] < bestCost)) {
                best = j;
                bestCost = cost[current][j];
            }
        }
        visited[best] = true;
        tour.push_back(best);
        current = best;
    }
    return tour;
}
Arr generateRandomTour(int n, int start, mt19937& rng) {
    Arr tour;
    tour.reserve(n);
    tour.push_back(start);
    Arr rest;
    rest.reserve(n - 1);
    for (int i = 0; i < n; i++) if (i != start) rest.push_back(i);
    shuffle(rest.begin(), rest.end(), rng);
    for (int c : rest) tour.push_back(c);
    return tour;
}
Arr generateNeighbor(const Arr& tour, mt19937& rng) {
    int n = (int)tour.size();
    Arr neighbor = tour;
    if (n <= 3) return neighbor;
    uniform_int_distribution<int> dist(1, n - 1);
    int i = dist(rng);
    int j = dist(rng);
    if (i == j) return neighbor;
    if (i > j) swap(i, j);
    reverse(neighbor.begin() + i, neighbor.begin() + j + 1);
    return neighbor;
}
Result simulatedAnnealing(
    const Matrix& cost,
    Arr            initialTour,
    double         t0,
    double         alpha,
    double         tMin,
    int            iterPerT,
    long long      maxIter,
    mt19937&       rng
) {
    auto startTime = chrono::high_resolution_clock::now();
    Arr current = initialTour;
    double currentCost = calculateTourCost(cost, current);
    Arr best = current;
    double bestCost = currentCost;
    double initCost = currentCost;
    double T = t0;
    long long iterCount = 0;
    long long accepted = 0;
    long long worseAccepted = 0;
    uniform_real_distribution<double> unif(0.0, 1.0);
    while (T > tMin && iterCount < maxIter) {
        for (int k = 0; k < iterPerT && iterCount < maxIter; k++) {
            Arr neighbor = generateNeighbor(current, rng);
            double neighborCost = calculateTourCost(cost, neighbor);
            double delta = neighborCost - currentCost;
            bool doAccept = false;
            if (delta <= 0.0) {
                doAccept = true;
            } else {
                double p = exp(-delta / T);
                double r = unif(rng);
                if (r < p) {
                    doAccept = true;
                    worseAccepted++;
                }
            }
            if (doAccept) {
                current = neighbor;
                currentCost = neighborCost;
                accepted++;
                if (currentCost < bestCost) {
                    bestCost = currentCost;
                    best = current;
                }
            }
            iterCount++;
        }
        T *= alpha;
    }
    auto endTime = chrono::high_resolution_clock::now();
    double elapsed = chrono::duration<double>(endTime - startTime).count();
    Result res;
    res.name    = "SIMULATED ANNEALING";
    res.tour    = best;
    res.initTour = initialTour;
    res.cost    = bestCost;
    res.timeSec = elapsed;
    res.isSA    = true;
    res.initCost    = initCost;
    res.initTemp    = t0;
    res.coolingRate = alpha;
    res.totalIter   = iterCount;
    res.acceptedMoves       = accepted;
    res.worseMovesAccepted  = worseAccepted;
    return res;
}
static void printTour(const Arr& tour) {
    for (size_t i = 0; i < tour.size(); i++) cout << tour[i] << " -> ";
    cout << tour[0] << "\n";
}
void printResults(const Result& r) {
    cout << "---------- " << r.name << " ----------\n";
    if (!r.isSA) {
        cout << "Tour:\n";
        printTour(r.tour);
        cout << "Total Cost: " << r.cost << "\n";
        cout << "Execution Time: " << fixed << setprecision(6) << r.timeSec << " seconds\n";
    } else {
        cout << "Initial Solution:\n";
        printTour(r.initTour);
        cout << "Initial Cost: " << r.initCost << "\n";
        cout << "Best Tour Found:\n";
        printTour(r.tour);
        cout << "Best Cost: " << r.cost << "\n";
        cout << "Initial Temperature: " << r.initTemp << "\n";
        cout << "Cooling Rate: " << r.coolingRate << "\n";
        cout << "Total Iterations: " << r.totalIter << "\n";
        cout << "Accepted Moves: " << r.acceptedMoves << "\n";
        cout << "Worse Moves Accepted: " << r.worseMovesAccepted << "\n";
        cout << "Execution Time: " << fixed << setprecision(6) << r.timeSec << " seconds\n";
    }
    cout << "\n";
}
Matrix generateEuclideanInstance(int n, mt19937& rng, double maxCoord = 1000.0) {
    uniform_real_distribution<double> dist(0.0, maxCoord);
    vector<pair<double,double>> pts(n);
    for (int i = 0; i < n; i++) pts[i] = { dist(rng), dist(rng) };
    Matrix cost(n, vector<double>(n, 0.0));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (i != j)
                cost[i][j] = round(hypot(pts[i].first - pts[j].first,
                                          pts[i].second - pts[j].second));
    return cost;
}
void writeInstance(const string& filename, const Matrix& cost) {
    ofstream f(filename);
    int n = (int)cost.size();
    f << n << "\n";
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            f << (long long)cost[i][j];
            if (j + 1 < n) f << " ";
        }
        f << "\n";
    }
}




class Graph {
private:
    int n = 0;
    Matrix cost;
public:
    Graph() = default;
    bool readInput(const string& filename) {
        return ::readInput(filename, cost, n);
    }
    void setMatrix(const Matrix& m) { cost = m; n = (int)m.size(); }
    double getCost(const Arr& tour) const { return calculateTourCost(cost, tour); }
    bool   isValid(const Arr& tour) const { return isValidTour(cost, tour); }
    int    size() const { return n; }
    double getEdge(int u, int v) const { return cost[u][v]; }
    const Matrix& matrix() const { return cost; }
};
class Strategy {
public:
    virtual ~Strategy() = default;
    virtual Result solve(const Graph& g) = 0;
};
class Greedy : public Strategy {
private:
    int start;
public:
    explicit Greedy(int startingCity = 0) : start(startingCity) {}
    Result solve(const Graph& g) override {
        auto t0 = chrono::high_resolution_clock::now();
        Arr tour = greedyTSP(g.matrix(), start);
        auto t1 = chrono::high_resolution_clock::now();
        Result r;
        r.name    = "GREEDY METHOD";
        r.tour    = tour;
        r.cost    = g.getCost(tour);
        r.timeSec = chrono::duration<double>(t1 - t0).count();
        r.isSA    = false;
        return r;
    }
};


enum class InitType { RANDOM, GREEDY_INIT };
class Annealing : public Strategy {
private:
    double t0, alpha, tMin;
    int iterPerT;
    long long maxIter;
    InitType initType;
    unsigned seed;
public:
    Annealing(
        double startingTemperature      = 1000.0,
        double coolingRateAlpha         = 0.995,
        double minimumTemperature       = 0.001,
        int    iterationsPerTemperature = 100,
        long long maximumIterations     = 100000,
        InitType initialSolutionType    = InitType::RANDOM,
        unsigned rngSeed                = 42
    ) : t0(startingTemperature), alpha(coolingRateAlpha), tMin(minimumTemperature),
        iterPerT(iterationsPerTemperature), maxIter(maximumIterations),
        initType(initialSolutionType), seed(rngSeed) {}
    Result solve(const Graph& g) override {
        mt19937 rng(seed);
        Arr initial = (initType == InitType::RANDOM)
                        ? generateRandomTour(g.size(), 0, rng)
                        : greedyTSP(g.matrix(), 0);
        return simulatedAnnealing(g.matrix(), initial, t0, alpha, tMin, iterPerT, maxIter, rng);
    }
};


class Experiment {
public:
    static void print(const Result& res) { printResults(res); }
    static void compare(const Result& gRes, const Result& aRes) {
        cout << "---------- COMPARISON ----------\n";
        cout << "Greedy Cost: " << gRes.cost << "\n";
        cout << "Simulated Annealing Cost: " << aRes.cost << "\n";
        double improvement = (gRes.cost > 0) ? (gRes.cost - aRes.cost) / gRes.cost * 100.0 : 0.0;
        cout << "Improvement: " << fixed << setprecision(2) << improvement << "%\n";
        if (aRes.cost < gRes.cost - 1e-9)      cout << "Best Method: Simulated Annealing\n";
        else if (gRes.cost < aRes.cost - 1e-9) cout << "Best Method: Greedy\n";
        else                                   cout << "Best Method: Both methods produced the same cost.\n";
        cout << "========================================\n\n";
    }
    static void runExp1(const vector<pair<string, Graph>>& instances) {
        cout << "\n===================== EXPERIMENT 1: GREEDY vs SIMULATED ANNEALING =====================\n";
        cout << left  << setw(14) << "Instance"
             << right << setw(8)  << "Cities"
             << right << setw(12) << "Greedy"
             << right << setw(12) << "SA_Best"
             << right << setw(12) << "SA_Avg"
             << right << setw(12) << "Improve%"
             << right << setw(16) << "GreedyTime(s)"
             << right << setw(14) << "SATime(s)" << "\n";
        for (auto& pr : instances) {
            const string& name = pr.first;
            const Graph& g = pr.second;
            Greedy greedy(0);
            Result gr = greedy.solve(g);

            const int SA_RUNS = 5;
            double saBest = 1e18, saSum = 0, saTimeSum = 0;
            for (int i = 0; i < SA_RUNS; i++) {
                Annealing sa(1000.0, 0.995, 0.001, 100, 100000, InitType::RANDOM, 42 + i);
                Result ar = sa.solve(g);
                saSum += ar.cost;
                saTimeSum += ar.timeSec;
                if (ar.cost < saBest) saBest = ar.cost;
            }
            double saAvg = saSum / SA_RUNS;
            double saTimeAvg = saTimeSum / SA_RUNS;
            double improvement = (gr.cost - saBest) / gr.cost * 100.0;

            cout << left  << setw(14) << name
                 << right << setw(8)  << g.size()
                 << right << setw(12) << (long long)llround(gr.cost)
                 << right << setw(12) << (long long)llround(saBest)
                 << right << setw(12) << (long long)llround(saAvg)
                 << right << setw(12) << fixed << setprecision(2) << improvement
                 << right << setw(16) << fixed << setprecision(6) << gr.timeSec
                 << right << setw(14) << fixed << setprecision(6) << saTimeAvg << "\n";
        }
        cout << "=========================================================================================\n";
    }
    static void runExp2(const Graph& g) {
        cout << "\n===================== EXPERIMENT 2: VARYING INITIAL TEMPERATURE (T0) =====================\n";
        vector<double> temps = {100.0, 1000.0, 5000.0};
        cout << left << setw(10) << "T0" << right << setw(14) << "BestCost"
             << setw(16) << "AcceptedMoves" << setw(18) << "WorseAccepted"
             << setw(14) << "Time(s)" << "\n";
        for (double t0 : temps) {
            Annealing sa(t0, 0.995, 0.001, 100, 100000, InitType::RANDOM, 7);
            Result r = sa.solve(g);
            cout << left << setw(10) << t0 << right << setw(14) << (long long)llround(r.cost)
                 << setw(16) << r.acceptedMoves << setw(18) << r.worseMovesAccepted
                 << setw(14) << fixed << setprecision(6) << r.timeSec << "\n";
        }
        cout << "=============================================================================================\n";
        
    }
    static void runExp3(const Graph& g) {
        cout << "\n===================== EXPERIMENT 3: VARYING COOLING RATE (alpha) =====================\n";
        vector<double> alphas = {0.90, 0.95, 0.995};
        cout << left << setw(10) << "Alpha" << right << setw(14) << "BestCost"
             << setw(18) << "TempStepsToMin" << setw(14) << "Time(s)" << "\n";
        for (double a : alphas) {
            Annealing sa(1000.0, a, 0.001, 100, 100000, InitType::RANDOM, 7);
            Result r = sa.solve(g);
            int steps = (int)ceil(log(0.001 / 1000.0) / log(a));
            cout << left << setw(10) << a << right << setw(14) << r.cost
                 << setw(18) << steps << setw(14) << fixed << setprecision(6) << r.timeSec << "\n";
        }
        cout << "=========================================================================================\n";
        
    }
    static void runExp4(const Graph& g) {
        cout << "\n===================== EXPERIMENT 4: RANDOM vs GREEDY INITIALIZATION =====================\n";
        const int RUNS = 5;
        auto runBatch = [&](InitType type, const string& label) {
            vector<double> costs;
            vector<double> times;
            for (int i = 0; i < RUNS; i++) {
                Annealing sa(1000.0, 0.995, 0.001, 100, 100000, type, 1000 + i);
                Result r = sa.solve(g);
                costs.push_back(r.cost);
                times.push_back(r.timeSec);
            }
            double best = *min_element(costs.begin(), costs.end());
            double worst = *max_element(costs.begin(), costs.end());
            double avg = accumulate(costs.begin(), costs.end(), 0.0) / RUNS;
            double avgTime = accumulate(times.begin(), times.end(), 0.0) / RUNS;
            cout << left << setw(16) << label
                 << right << setw(12) << best
                 << setw(12) << fixed << setprecision(1) << avg
                 << setw(12) << worst
                 << setw(14) << fixed << setprecision(6) << avgTime << "\n";
        };
        cout << left << setw(16) << "InitMethod" << right << setw(12) << "Best"
             << setw(12) << "Avg" << setw(12) << "Worst" << setw(14) << "AvgTime(s)" << "\n";
        runBatch(InitType::RANDOM,      "Random");
        runBatch(InitType::GREEDY_INIT, "Greedy");
        cout << "===========================================================================================\n";
        
    }
};
void runSingle(const string& inputFile, double t0, double alpha, double tMin,
               int iterPerT, long long maxIter, InitType initType, unsigned seed) {
    Graph g;
    if (!g.readInput(inputFile)) {
        cerr << "Failed to read input file: " << inputFile << "\n";
        return;
    }
    cout << "========================================\n";
    cout << "TRAVELLING SALESMAN PROBLEM\n";
    cout << "========================================\n";
    cout << "Number of Cities: " << g.size() << "\n\n";
    Greedy greedy(0);
    Result gRes = greedy.solve(g);
    if (!g.isValid(gRes.tour)) cerr << "WARNING: greedy tour failed validation!\n";
    Experiment::print(gRes);
    Annealing sa(t0, alpha, tMin, iterPerT, maxIter, initType, seed);
    Result aRes = sa.solve(g);
    if (!g.isValid(aRes.tour)) cerr << "WARNING: SA tour failed validation!\n";
    Experiment::print(aRes);
    Experiment::compare(gRes, aRes);
}
string getOpt(int argc, char** argv, const string& flag, const string& def) {
    for (int i = 0; i < argc - 1; i++)
        if (flag == argv[i]) return argv[i + 1];
    return def;
}
int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Usage:\n"
             << "  " << argv[0] << " solve <inputfile> [--t0 V] [--alpha V] [--tmin V]\n"
             << "                 [--iterT N] [--maxIter N] [--init random|greedy] [--seed N]\n"
             << "  " << argv[0] << " gen <N> <outputfile> [--seed N]\n"
             << "  " << argv[0] << " exp1 <file1> [file2 ...]\n"
             << "  " << argv[0] << " exp2 <inputfile>\n"
             << "  " << argv[0] << " exp3 <inputfile>\n"
             << "  " << argv[0] << " exp4 <inputfile>\n"
             << "  " << argv[0] << " all\n";
        return 1;
    }
    string mode = argv[1];
    if (mode == "solve") {
        if (argc < 3) { cerr << "Missing input file.\n"; return 1; }
        string inputFile = argv[2];
        double t0     = stod(getOpt(argc, argv, "--t0", "1000"));
        double alpha  = stod(getOpt(argc, argv, "--alpha", "0.995"));
        double tMin   = stod(getOpt(argc, argv, "--tmin", "0.001"));
        int iterPerT  = stoi(getOpt(argc, argv, "--iterT", "100"));
        long long maxIter = stoll(getOpt(argc, argv, "--maxIter", "100000"));
        string initStr = getOpt(argc, argv, "--init", "random");
        InitType initType = (initStr == "greedy") ? InitType::GREEDY_INIT : InitType::RANDOM;
        unsigned seed = (unsigned)stoul(getOpt(argc, argv, "--seed", "42"));
        runSingle(inputFile, t0, alpha, tMin, iterPerT, maxIter, initType, seed);
    } else if (mode == "gen") {
        if (argc < 4) { cerr << "Usage: gen <N> <outputfile> [--seed N]\n"; return 1; }
        int n = stoi(argv[2]);
        string outFile = argv[3];
        unsigned seed = (unsigned)stoul(getOpt(argc, argv, "--seed", "42"));
        mt19937 rng(seed);
        Matrix m = generateEuclideanInstance(n, rng);
        writeInstance(outFile, m);
        cout << "Generated " << n << "-city instance -> " << outFile << "\n";
    } else if (mode == "exp1") {
        if (argc < 3) { cerr << "Provide at least one input file.\n"; return 1; }
        vector<pair<string, Graph>> instances;
        for (int i = 2; i < argc; i++) {
            Graph g;
            if (!g.readInput(argv[i])) continue;
            instances.push_back({ argv[i], g });
        }
        Experiment::runExp1(instances);
    } else if (mode == "exp2") {
        if (argc < 3) { cerr << "Missing input file.\n"; return 1; }
        Graph g; g.readInput(argv[2]);
        Experiment::runExp2(g);
    } else if (mode == "exp3") {
        if (argc < 3) { cerr << "Missing input file.\n"; return 1; }
        Graph g; g.readInput(argv[2]);
        Experiment::runExp3(g);
    } else if (mode == "exp4") {
        if (argc < 3) { cerr << "Missing input file.\n"; return 1; }
        Graph g; g.readInput(argv[2]);
        Experiment::runExp4(g);
    } else if (mode == "all") {
        vector<pair<string,int>> sizes = {
            {"Test1_10", 10}, {"Test2_20", 20}, {"Test3_50", 50},
            {"Test4_100", 100}, {"Test5_200", 200}
        };
        vector<pair<string, Graph>> instances;
        for (auto& [name, n] : sizes) {
            mt19937 rng(1234 + n);
            Matrix m = generateEuclideanInstance(n, rng);
            string fname = name + ".txt";
            writeInstance(fname, m);
            Graph g; g.setMatrix(m);
            instances.push_back({ name, g });
        }
        Experiment::runExp1(instances);
        const Graph& rep = instances[2].second;
        cout << "\n(Experiments 2-4 use the 50-city instance as the representative case)\n";
        Experiment::runExp2(rep);
        Experiment::runExp3(rep);
        Experiment::runExp4(rep);
    } else {
        cerr << "Unknown mode: " << mode << "\n";
        return 1;
    }
    return 0;
}
