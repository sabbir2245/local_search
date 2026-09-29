#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <climits>
#include <random>
#include <map>

using namespace std;

struct Edge
{
    int u, v, w;
};

struct Neighbor
{
    int to;
    int weight;
};

struct LocalSearchResult
{
    vector<int> partition;
    int iterations;
};

class Graph
{
    int n, m;
    vector<Edge> edgelist;
    vector<vector<Neighbor>> adj;

public:
    Graph()
    {
        this->n = 0;
        this->m = 0;
    }

    Graph(int n, int m)
    {
        this->n = n;
        this->m = m;
        this->adj.resize(n);
    }

    void addEdge(int u, int v, int w)
    {
        Edge e;
        e.u = u;
        e.v = v;
        e.w = w;
        edgelist.push_back(e);
        adj[u].push_back({v, w});
        adj[v].push_back({u, w});
    }

    int getN() const { return n; }
    int getM() const { return m; }
    const vector<Edge>& getEdges() const { return edgelist; }
    const vector<Neighbor>& getNeighbors(int u) const { return adj[u]; }
};

Graph processFile(const string& path)
{
    ifstream infile(path);
    if (!infile.is_open())
    {
        cerr << "Error: input file not found: " << path << endl;
        return Graph();
    }

    int n, m;
    if (!(infile >> n >> m)) return Graph();

    Graph graph(n, m);

    for (int i = 0; i < m; i++)
    {
        int u, v, w;
        infile >> u >> v >> w;
        u--; // Convert 1-based index to 0-based
        v--;
        graph.addEdge(u, v, w);
    }

    return graph;
}

int computeCutWeight(const Graph& g, const vector<int>& partition)
{
    int weight = 0;
    for (const auto& e : g.getEdges())
    {
        if (partition[e.u] != partition[e.v])
            weight += e.w;
    }
    return weight;
}

// ---------- Randomized Heuristic ----------
double randomizedMaxCut(const Graph& g, mt19937& rng)
{
    int n = g.getN();
    long long totalCutWeight = 0;

    uniform_int_distribution<int> dist(0, 1);
    const auto& edges = g.getEdges();

    vector<int> partition(n);
    for (int t = 0; t < n; t++)
    {
        for (int i = 0; i < n; i++)
            partition[i] = dist(rng); // set either rand 0 or 1 

        int cutWeight = 0;
        for (const auto& e : edges)
            if (partition[e.u] != partition[e.v])
                cutWeight += e.w;

        totalCutWeight += cutWeight;
    }

    return (double)totalCutWeight / n;
}

// ----------Greedy Heuristic ----------
vector<int> greedyMaxCut(const Graph& g)
{
    int n = g.getN();
    vector<int> partition(n, -1);

    int maxW = INT_MIN, maxU = 0, maxV = 0;
    for (const auto& e : g.getEdges())
    {
        if (e.w > maxW)
        {
            maxW = e.w;
            maxU = e.u;
            maxV = e.v;
        }
    }

    if (maxW == INT_MIN)
    {
        fill(partition.begin(), partition.end(), 0);
        return partition;
    }

    partition[maxU] = 0; // Set X
    partition[maxV] = 1; // Set Y

    vector<int> sigX(n, 0), sigY(n, 0);

    for (const auto& nbr : g.getNeighbors(maxU))
        sigX[nbr.to] += nbr.weight;

    for (const auto& nbr : g.getNeighbors(maxV))
        sigY[nbr.to] += nbr.weight;

    for (int z = 0; z < n; z++)
    {
        if (z == maxU || z == maxV) continue;

        int valIfX = sigY[z]; // edge weight to Y
        int valIfY = sigX[z]; // edge weight to X

        int chosenPartition = (valIfY > valIfX) ? 1 : 0;
        partition[z] = chosenPartition;

        const auto& nbrs = g.getNeighbors(z);
        if (chosenPartition == 0)
        {
            for (const auto& nbr : nbrs)
                sigX[nbr.to] += nbr.weight;
        }
        else
        {
            for (const auto& nbr : nbrs)
                sigY[nbr.to] += nbr.weight;
        }
    }

    return partition;
}

// ---------- Algorithm 4: Semi-Greedy Heuristic ----------
vector<int> semiGreedyMaxCut(const Graph& g, double alpha, mt19937& rng)
{
    int n = g.getN();
    vector<int> partition(n, -1);

    int maxW = INT_MIN, maxU = 0, maxV = 0;
    for (const auto& e : g.getEdges())
    {
        if (e.w > maxW)
        {
            maxW = e.w;
            maxU = e.u;
            maxV = e.v;
        }
    }

    if (maxW == INT_MIN)
    {
        fill(partition.begin(), partition.end(), 0);
        return partition;
    }

    partition[maxU] = 0;
    partition[maxV] = 1;

    vector<int> sigX(n, 0), sigY(n, 0);
    vector<int> unassigned;
    unassigned.reserve(n);

    for (const auto& nbr : g.getNeighbors(maxU))
        sigX[nbr.to] += nbr.weight;

    for (const auto& nbr : g.getNeighbors(maxV))
        sigY[nbr.to] += nbr.weight;

    for (int v = 0; v < n; v++)
        if (partition[v] == -1)
            unassigned.push_back(v);

    vector<int> rcl;
    while (!unassigned.empty())
    {
        vector<int> gvals(unassigned.size());
        int wmin = INT_MAX, wmax = INT_MIN;
        for (size_t i = 0; i < unassigned.size(); i++)
        {
            int v = unassigned[i];
            gvals[i] = max(sigX[v], sigY[v]);
            wmin = min(wmin, gvals[i]);
            wmax = max(wmax, gvals[i]);
        }

        double cutoff = wmin + alpha * (wmax - wmin);

        rcl.clear();
        for (size_t i = 0; i < unassigned.size(); i++)
            if (gvals[i] >= cutoff)
                rcl.push_back(unassigned[i]);

        if (rcl.empty()) rcl = unassigned;

        uniform_int_distribution<int> dist(0, rcl.size() - 1);
        int chosenIndex = dist(rng);
        int chosen = rcl[chosenIndex];

        int chosenPartition = (sigX[chosen] >= sigY[chosen]) ? 1 : 0;
        partition[chosen] = chosenPartition;

        for (size_t i = 0; i < unassigned.size(); i++)
        {
            if (unassigned[i] == chosen)
            {
                unassigned[i] = unassigned.back();
                unassigned.pop_back();
                break;
            }
        }

        const auto& nbrs = g.getNeighbors(chosen);
        if (chosenPartition == 0)
        {
            for (const auto& nbr : nbrs)
                sigX[nbr.to] += nbr.weight;
        }
        else
        {
            for (const auto& nbr : nbrs)
                sigY[nbr.to] += nbr.weight;
        }
    }

    return partition;
}

// ---------- Optimized Local Search (Returns Partition & Iteration Count) ----------
LocalSearchResult localSearch(const Graph& g, vector<int> partition)
{
    int n = g.getN();
    vector<int> sigX(n, 0), sigY(n, 0);

    for (int u = 0; u < n; u++)
    {
        for (const auto& nbr : g.getNeighbors(u))
        {
            if (partition[nbr.to] == 0)
                sigX[u] += nbr.weight;
            else if (partition[nbr.to] == 1)
                sigY[u] += nbr.weight;
        }
    }

    int iterations = 0;

    while (true)
    {
        int bestDelta = 0;
        int bestV = -1;

        for (int v = 0; v < n; v++)
        {
            int delta = (partition[v] == 0) ? (sigX[v] - sigY[v]) : (sigY[v] - sigX[v]);

            if (delta > bestDelta)
            {
                bestDelta = delta;
                bestV = v;
            }
        }

        if (bestDelta > 0)
        {
            iterations++;
            int oldP = partition[bestV];
            int newP = 1 - oldP;
            partition[bestV] = newP;

            for (const auto& nbr : g.getNeighbors(bestV))
            {
                int u = nbr.to;
                int w = nbr.weight;
                if (oldP == 0) // bestV moved from X (0) to Y (1)
                {
                    sigX[u] -= w;
                    sigY[u] += w;
                }
                else // bestV moved from Y (1) to X (0)
                {
                    sigY[u] -= w;
                    sigX[u] += w;
                }
            }
        }
        else
        {
            break;
        }
    }

    return {partition, iterations};
}

// ---------- GRASP ----------
vector<int> grasp(const Graph& g, int maxIterations, double alpha, mt19937& rng)
{
    vector<int> bestPartition;
    int bestWeight = -1;

    for (int i = 0; i < maxIterations; i++)
    {
        vector<int> sol = semiGreedyMaxCut(g, alpha, rng);
        LocalSearchResult lsRes = localSearch(g, sol);
        int w = computeCutWeight(g, lsRes.partition);

        if (i == 0 || w > bestWeight)
        {
            bestWeight = w;
            bestPartition = lsRes.partition;
        }
    }

    return bestPartition;
}

int main()
{
    map<int, string> knownBestMap = {
        {1, "12078"}, {2, "12084"}, {3, "12077"},
        {11, "627"},  {12, "621"},  {13, "645"},
        {14, "3187"}, {15, "3169"}, {16, "3172"},
        {22, "14123"}, {23, "14129"}, {24, "14131"},
        {32, "1560"}, {33, "1537"}, {34, "1541"},
        {35, "8000"}, {36, "7996"}, {37, "8009"},
        {43, "7027"}, {44, "7022"}, {45, "7020"},
        {48, "6000"}, {49, "6000"}, {50, "5988"}
    };

    string basePath = "set1/";
    int numFiles = 54;
    int graspIterations = 300;
    double alpha = 0.6;

    ofstream csv("2205040.csv");
    
    csv << "Name,|V| or n,|E| or m,Simple Randomized,Simple Greedy,Semi-greedy (α = 0.6),Simple Local - No. of iterations,Simple Local - Average value,GRASP (300 iterations) - Best value,Known best solution or upper bound" << endl;

    cout << "Name\tn\tm\tRnd\tGreedy\tSemiG\tLS_Iter\tLS_Val\tGRASP_Val\tKnownBest" << endl;

    mt19937 rng(42);

    for (int i = 0; i < numFiles; i++)
    {
        int gNum = i + 1;
        string filename = basePath + "g" + to_string(gNum) + ".rud";
        Graph g = processFile(filename);

        if (g.getN() == 0) continue;

        int n = g.getN();
        int m = g.getM();

        double avgRand = randomizedMaxCut(g, rng);

        vector<int> greedyPart = greedyMaxCut(g);
        int greedyW = computeCutWeight(g, greedyPart);

        vector<int> semiPart = semiGreedyMaxCut(g, alpha, rng);
        int semiW = computeCutWeight(g, semiPart);

        LocalSearchResult lsRes = localSearch(g, greedyPart);
        int lsW = computeCutWeight(g, lsRes.partition);

        vector<int> graspPart = grasp(g, graspIterations, alpha, rng);
        int graspW = computeCutWeight(g, graspPart);

        string kbStr = knownBestMap.count(gNum) ? knownBestMap[gNum] : "";

        cout << "G" << gNum << "\t" << n << "\t" << m << "\t"
             << (int)avgRand << "\t" << greedyW << "\t" << semiW << "\t"
             << lsRes.iterations << "\t" << lsW << "\t"
             << graspW << "\t" << kbStr << endl;

        csv << "G" << gNum << "," << n << "," << m << ","
            << (int)avgRand << "," << greedyW << "," << semiW << ","
            << lsRes.iterations << "," << lsW << ","
            << graspW << "," << kbStr << endl;
    }

    csv.close();
    cout << "Results written to 2205040.csv" << endl;

    return 0;
}
