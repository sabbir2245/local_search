MaxCut Code Limitations
Limitations and Fixes — 2205040_on.cpp
Overview

2205040_on.cpp is a Max-Cut solver implementing several heuristics: randomized, greedy, multi-greedy, semi-greedy, local search, and GRASP. The program reads graph files from set1/ and writes results to 2205040.csv. While the overall structure is sound, there are several bugs, inefficiencies, and correctness issues that need to be addressed.
1. MultigreedyMaxCut — Uninitialized Edge List (Critical)
Limitation

In MultigreedyMaxCut, the local edgelist is declared as:
cpp

vector<Edge> edgelist(m);

but it is never populated from the graph. The sorting loop then operates on default-constructed Edge objects (all zeros), and the selection Edge e = edgelist[i]; always picks a zero-weight edge. As a result, maxW == INT_MIN is never true (it's 0), and the algorithm partitions the graph based on an arbitrary edge (0,0).

Additionally, partition is declared outside the for (int i = 0; i < T; i++) loop, so state leaks between iterations.
Fix
cpp

vector<int> MultigreedyMaxCut(const Graph &g, int T)
{
    int n = g.getN();
    int m = g.getM();
    vector<int> bestpartition;
    int bestWeight = -1;

    // Copy edges from graph
    vector<Edge> edgelist = g.getEdges();

    // Sort in descending order of weight (use std::sort)
    sort(edgelist.begin(), edgelist.end(),
         [](const Edge &a, const Edge &b) { return a.w > b.w; });

    int limit = min(T, m);
    for (int i = 0; i < limit; i++)
    {
        vector<int> partition(n, -1);
        Edge e = edgelist[i];

        partition[e.u] = 0;
        partition[e.v] = 1;

        vector<int> sigX(n, 0), sigY(n, 0);
        for (const auto &nbr : g.getNeighbors(e.u)) sigX[nbr.to] += nbr.weight;
        for (const auto &nbr : g.getNeighbors(e.v)) sigY[nbr.to] += nbr.weight;

        for (int z = 0; z < n; z++)
        {
            if (partition[z] != -1) continue;
            int valIfX = sigY[z];
            int valIfY = sigX[z];
            int chosen = (valIfY > valIfX) ? 1 : 0;
            partition[z] = chosen;
            for (const auto &nbr : g.getNeighbors(z))
            {
                if (chosen == 0) sigX[nbr.to] += nbr.weight;
                else             sigY[nbr.to] += nbr.weight;
            }
        }

        int w = computeCutWeight(g, partition);
        if (w > bestWeight) { bestWeight = w; bestpartition = partition; }
    }
    return bestpartition;
}

2. O(n²) Bubble Sort Instead of std::sort
Limitation

The nested loop:
cpp

for (int i = 0; i < m; i++)
    for (int j = 0; j < m; j++)
        if (edgelist[j].w > edgelist[i].w) { swap(...); }

is a hand-rolled bubble/selection sort with O(m²) complexity. For large graphs this is prohibitively slow, and it doesn't even sort correctly (inner loop starts at j = 0 every time).
Fix

Use std::sort with a comparator (already included via <algorithm>):
cpp

sort(edgelist.begin(), edgelist.end(),
     [](const Edge &a, const Edge &b) { return a.w > b.w; });

3. randomizedMaxCut — Incorrect Averaging (Divide by n Instead of t)
Limitation
cpp

for (int t = 0; t < n; t++) { ... totalCutWeight += cutWeight; }
return (double)totalCutWeight / n;

The loop runs n times and divides by n, so the "average" is over n trials. However, the intention is usually a fixed number of trials (e.g., 100). Tying the number of trials to n makes the result non-comparable across graph sizes and may be extremely slow for large n.

Also, dist(rng) is called inside the loop but dist is uniform over {0,1} — fine, but the partition is fully re-randomized each trial. That's OK.
Fix

Use a fixed number of trials, e.g.:
cpp

double randomizedMaxCut(const Graph &g, mt19937 &rng, int trials = 100)
{
    int n = g.getN();
    long long total = 0;
    uniform_int_distribution<int> dist(0, 1);
    const auto &edges = g.getEdges();
    vector<int> partition(n);

    for (int t = 0; t < trials; t++)
    {
        for (int i = 0; i < n; i++) partition[i] = dist(rng);
        int cut = 0;
        for (const auto &e : edges)
            if (partition[e.u] != partition[e.v]) cut += e.w;
        total += cut;
    }
    return (double)total / trials;
}

4. grasp — Local Search Result Discarded When Iteration 0
Limitation
cpp

if (i == 0 || w > bestWeight) { bestWeight = w; bestPartition = lsRes.partition; }

This is correct, but note that bestPartition is never initialized if maxIterations == 0. Add a guard.
Fix
cpp

if (maxIterations <= 0) return vector<int>(g.getN(), 0);

5. localSearch — Correct, but Only First-Improvement Variant
Limitation

The function correctly finds the best single-vertex move, but it's a steepest-ascent local search. That's fine, but the function name and the CSV column "Simple Local - Average value" imply multiple runs may be expected.
Fix (optional)

If "average value" is required, run local search from multiple random starting partitions and average the results. Currently only one run is reported.
cpp

// Example: average over multiple starts
double lsAvg = 0;
int trials = 10;
for (int t = 0; t < trials; t++) {
    vector<int> start(g.getN());
    for (int i = 0; i < g.getN(); i++) start[i] = rng() % 2;
    auto r = localSearch(g, start);
    lsAvg += computeCutWeight(g, r.partition);
}
lsAvg /= trials;

6. main — greedyWM Uninitialized and Overwritten
Limitation
cpp

int greedyWM;
for (auto &val : tarry) {
    vector<int> bestgreedyPart = MultigreedyMaxCut(g, val);
    greedyWM = computeCutWeight(g, bestgreedyPart);
}

Only the last value (T = 25) is kept. If the intention is to take the best across T ∈ {5, 10, 25}, this is wrong.

Also, if tarry were empty, greedyWM would be uninitialized — UB.
Fix
cpp

int greedyWM = -1;
for (int val : {5, 10, 25}) {
    int w = computeCutWeight(g, MultigreedyMaxCut(g, val));
    if (w > greedyWM) greedyWM = w;
}

7. CSV Header Mismatch
Limitation

Header:
text

Name,|V| or n,|E| or m,Simple Randomized,Simple Greedy,Multigreedy , Semi-greedy (α = 0.6),Simple Local - No. of iterations,Simple Local - Average value,GRASP (300 iterations) - Best value,Known best solution or upper bound

Data row writes:
text

G1,n,m,avgRand,greedyW,greedyWM,semiW,lsRes.iterations,lsW,graspW,kbStr

The "Simple Local - Average value" column actually contains lsW (a single value), not an average. Also, the column labeled "Simple Local - No. of iterations" is filled with lsRes.iterations, which matches, but "Average value" should be a separate average across multiple runs.
Fix

Either rename the column to "Simple Local - Value" or compute a true average (see §5).
8. knownBestMap Hardcoded to set1 Indices
Limitation
cpp

map<int, string> knownBestMap = { {1,"12078"}, ..., {50,"5988"} };

Keys 1..50 correspond to g1..g50, but only g1..g6 are processed (since numFiles = 6). The map is harmless but misleading. More importantly, if the file naming changes (set2/g1.rud), the lookup will silently return "".
Fix

Derive the key from the filename or pass a lookup table per set.
9. processFile — No Handling of Duplicate/Invalid Edges
Limitation

No checks for:

    self-loops (u == v),

    invalid vertex indices,

    duplicate edges.

A self-loop contributes 0 to the cut but may be counted incorrectly in adj.
Fix
cpp

if (u == v || u < 0 || v < 0 || u >= n || v >= n) continue;

10. computeCutWeight — O(m) Per Call, Called Repeatedly
Limitation

computeCutWeight is called inside MultigreedyMaxCut and grasp on every iteration, giving O(T·m) per call. For large graphs this is a bottleneck.
Fix

Maintain the cut weight incrementally when flipping a vertex:
cpp

// When moving v from X to Y:
// delta = sigX[v] - sigY[v]
// newCut = oldCut + delta

This is already implicitly done in localSearch via bestDelta, but computeCutWeight is still called separately. Refactor to return the final weight from the search functions.
11. Missing <numeric> / Potential Integer Overflow
Limitation

totalCutWeight is long long, but computeCutWeight returns int. For dense graphs with large weights, int may overflow.
Fix

Change computeCutWeight to return long long and propagate.
cpp

long long computeCutWeight(const Graph &g, const vector<int> &partition)

12. Minor: Unused sigX/sigY in greedyMaxCut for maxU, maxV
Limitation

In greedyMaxCut, after setting partition[maxU] = 0 and partition[maxV] = 1, the loop updates sigX/sigY for all neighbors. However, sigX[maxU] and sigY[maxV] will include the edge between maxU and maxV (if any), which is fine. No bug, but the initial sigX/sigY for maxU/maxV themselves are never used.
Fix

No change needed — just note it.
Summary Table
#	Issue	Severity	Fix
1	edgelist never populated in MultigreedyMaxCut	Critical	Copy from g.getEdges()
2	O(n²) bubble sort	High	Use std::sort
3	Randomized average divides by n not trials	Medium	Fixed trial count
4	grasp uninitialized if maxIterations == 0	Low	Guard clause
5	"Average value" column not averaged	Medium	Multiple starts
6	greedyWM only keeps last T	High	Track max across T
7	CSV header mismatch	Low	Rename or compute average
8	knownBestMap hardcoded to set1	Low	Derive from filename
9	No duplicate/self-loop handling	Low	Skip invalid edges
10	computeCutWeight O(m) repeated	Medium	Incremental updates
11	int overflow in cut weight	Medium	Use long long
12	Unused vars in greedyMaxCut	Cosmetic	—
Recommended Priority Order

    Fix MultigreedyMaxCut (bug #1) — the algorithm currently does nothing useful.

    Replace bubble sort with std::sort (bug #2).

    Fix greedyWM tracking (bug #6).

    Fix randomized averaging (bug #3) and average local search (bug #5).

    Switch to long long (bug #11) and incremental cut weight (bug #10).

    Clean up CSV header and knownBestMap (bugs #7, #8).

    Add edge validation (bug #9).

