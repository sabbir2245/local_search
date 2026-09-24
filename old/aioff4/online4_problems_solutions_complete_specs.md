# CSE 318 — Online 4: Complete Problem Specifications and Line-by-Line Solution Guide

> **Line-number note:** All referenced line numbers are from the supplied `basecode.cpp` (547 lines). If you insert code, later line numbers in your editor will shift; use the function/class name as the primary anchor.

---

# Problem 1 — Section A1: Adaptive Cooling

## Problem Statement

The geometric cooling `T ← αT` uses a fixed `α`, which can be too fast in some regimes and too slow in others. A common improvement is to adapt `α` based on how often moves are being accepted: if too many moves are being accepted, the temperature is too high (cool faster); if very few moves are being accepted, the temperature is too low (cool slower or hold steady).

### What to implement.

Modify your SA loop so that cooling happens every `W` iterations (not every iteration), and the value of `α` used depends on the acceptance ratio in the last window.

1. Set `W = 5` or `10` (window size).
2. Every `W` iterations, compute

   `r = accepted moves in the last W iterations / W`.

3. Choose the cooling rate for this cooling step as:
   - If `r > 0.5`: `α = 0.90` (too many accepts, cool faster).
   - If `r < 0.1`: `α = 0.999` (too few accepts, cool slower).
   - Otherwise: `α = 0.995` (default).
4. Update `T ← αT` and reset the window’s accept counter.
5. Keep the same `T0`, `Tmin`, iteration budget, neighborhood, and acceptance rule as before.

### What to show in the output.

- Run modified SA 5 times on at least two instances. Report best, average, worst cost.
- Compare against your original SA (fixed `α = 0.995`). Report best/avg/worst for both.

## Solution

### A1.1 — Replace `simulatedAnnealing()`

**Add/replace at `basecode.cpp` lines 212–277**, specifically the main loop at lines 233–261.

Use `W = 5`. The supplied `iterPerT` is retained in the signature for compatibility, but cooling is now controlled by the required W-iteration window.

```cpp
// REPLACE simulatedAnnealing() at lines 212–277
Result simulatedAnnealing(
    const Matrix& cost,
    Arr initialTour,
    double t0,
    double alpha,
    double tMin,
    int iterPerT,
    long long maxIter,
    mt19937& rng
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

    // ADD: adaptive-cooling window
    const int W = 5;
    int windowIterations = 0;
    int windowAccepted = 0;

    uniform_real_distribution<double> unif(0.0, 1.0);

    while (T > tMin && iterCount < maxIter) {

        // One SA iteration
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
            windowAccepted++;

            if (currentCost < bestCost) {
                bestCost = currentCost;
                best = current;
            }
        }

        iterCount++;
        windowIterations++;

        // ADD: cool every W iterations
        if (windowIterations == W) {
            double acceptanceRatio =
                static_cast<double>(windowAccepted) / W;

            double currentAlpha;

            if (acceptanceRatio > 0.5) {
                currentAlpha = 0.90;
            } else if (acceptanceRatio < 0.1) {
                currentAlpha = 0.999;
            } else {
                currentAlpha = 0.995;
            }

            T *= currentAlpha;

            // Reset window
            windowIterations = 0;
            windowAccepted = 0;
        }
    }

    auto endTime = chrono::high_resolution_clock::now();

    double elapsed =
        chrono::duration<double>(endTime - startTime).count();

    Result res;
    res.name = "ADAPTIVE SIMULATED ANNEALING";
    res.tour = best;
    res.initTour = initialTour;
    res.cost = bestCost;
    res.timeSec = elapsed;
    res.isSA = true;
    res.initCost = initCost;
    res.initTemp = t0;
    res.coolingRate = 0.0; // adaptive
    res.totalIter = iterCount;
    res.acceptedMoves = accepted;
    res.worseMovesAccepted = worseAccepted;

    return res;
}
```

### A1.2 — Keep the original neighborhood and acceptance rule

**Do not modify `generateNeighbor()` at lines 200–210 for A1.**

The assignment explicitly says the neighborhood and acceptance rule stay the same.

The original acceptance code is at **lines 238–248**:

```cpp
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
```

### A1.3 — Run 5 times on at least two instances

**Base-code location:** `Experiment::runExp1()` at **lines 419–458**.

The supplied code already has:

```cpp
const int SA_RUNS = 5;
```

at line 435 and different seeds at line 438.

Add a cost vector around **lines 435–443**:

```cpp
vector<double> adaptiveCosts;

for (int i = 0; i < 5; i++) {
    Annealing sa(
        1000.0, 0.995, 0.001, 100, 100000,
        InitType::RANDOM, 42 + i
    );

    Result r = sa.solve(g);
    adaptiveCosts.push_back(r.cost);
}
```

Calculate the required statistics:

```cpp
double best =
    *min_element(adaptiveCosts.begin(), adaptiveCosts.end());

double avg =
    accumulate(adaptiveCosts.begin(), adaptiveCosts.end(), 0.0)
    / adaptiveCosts.size();

double worst =
    *max_element(adaptiveCosts.begin(), adaptiveCosts.end());

cout << "Adaptive SA Best: " << best << "\n";
cout << "Adaptive SA Average: " << avg << "\n";
cout << "Adaptive SA Worst: " << worst << "\n";
```

### A1.4 — Compare with original fixed-alpha SA

**Base-code location:** `runExp1()` lines 419–458.

You need two five-run batches:

```cpp
vector<double> originalCosts;
vector<double> adaptiveCosts;

for (int i = 0; i < 5; i++) {

    // ORIGINAL: fixed alpha = 0.995
    Annealing original(
        1000.0, 0.995, 0.001, 100, 100000,
        InitType::RANDOM, 42 + i
    );

    Result oldResult = original.solve(g);
    originalCosts.push_back(oldResult.cost);

    // MODIFIED: adaptive alpha
    Annealing adaptive(
        1000.0, 0.995, 0.001, 100, 100000,
        InitType::RANDOM, 42 + i
    );

    Result newResult = adaptive.solve(g);
    adaptiveCosts.push_back(newResult.cost);
}
```

For each batch report:

```cpp
double best =
    *min_element(costs.begin(), costs.end());

double avg =
    accumulate(costs.begin(), costs.end(), 0.0)
    / costs.size();

double worst =
    *max_element(costs.begin(), costs.end());
```

Report **best/avg/worst for both original and modified SA**, and do this on **at least two instances**.

---

# Problem 2 — Section B1: 3-Opt Neighbor

## Problem Statement

You have generated neighbiors by reversing one segment of the tour, disconnecting and reconnecting two edges. Update that portion so that it cuts the tour at three positions and reconnects the three segments in a new way. There are several reconnection variants; you will implement one specific and simple variant below.

### What to implement.

Write `generateNeighbor3Opt(tour)` that returns one neighbor tour by the following 3-cut move. Let `N` be the number of cities. City 0 is at position 0 and stays there.

1. Pick three positions `i < j < k` uniformly at random with `1 ≤ i < j < k ≤ N − 1`.
2. Split the tour into four parts:
   - `A = tour[0 ...i-1]`
   - `B = tour[i ...j-1]`
   - `C = tour[j ...k-1]`
   - `D = tour[k ...N-1]`
3. Return the concatenation

   `A + reverse(B) + reverse(C) + D`.

   (Both middle segments are reversed.)

Plug this in place of your current neighbor generator (or use it as an additional option — but pick one and be consistent). Keep everything else (initial solution, temperature, cooling, acceptance) identical to your original SA.

### Output:

- Run modified SA 5 times on at least two instances. Report best, average, worst cost.
- Report the total number of accepted moves and worse-accepted moves.

## Solution

### B1.1 — Add `generateNeighbor3Opt()`

**Base-code location:** Current `generateNeighbor()` is at **lines 200–210**.

Add the following function **immediately after line 210**:

```cpp
// ADD after basecode.cpp line 210
Arr generateNeighbor3Opt(const Arr& tour, mt19937& rng) {
    int n = static_cast<int>(tour.size());

    if (n < 4)
        return tour;

    // 1 <= i < j < k <= n - 1
    uniform_int_distribution<int> dist(1, n - 1);

    int i, j, k;

    do {
        i = dist(rng);
        j = dist(rng);
        k = dist(rng);
    } while (!(i < j && j < k));

    Arr neighbor;
    neighbor.reserve(n);

    // A = tour[0 ... i-1]
    for (int p = 0; p < i; p++)
        neighbor.push_back(tour[p]);

    // reverse(B), B = tour[i ... j-1]
    for (int p = j - 1; p >= i; p--)
        neighbor.push_back(tour[p]);

    // reverse(C), C = tour[j ... k-1]
    for (int p = k - 1; p >= j; p--)
        neighbor.push_back(tour[p]);

    // D = tour[k ... n-1]
    for (int p = k; p < n; p++)
        neighbor.push_back(tour[p]);

    return neighbor;
}
```

This exactly constructs:

`A + reverse(B) + reverse(C) + D`

and keeps city 0 at position 0.

### B1.2 — Use the 3-opt neighbor

**Base-code location:** line **235** inside `simulatedAnnealing()`.

Change:

```cpp
Arr neighbor = generateNeighbor(current, rng);
```

to:

```cpp
// CHANGE basecode.cpp line 235
Arr neighbor = generateNeighbor3Opt(current, rng);
```

Pick this option and use it consistently for the B1 experiment.

### B1.3 — Keep all other SA behavior unchanged

Do not change:

- Initial solution
- Temperature
- Cooling
- `Tmin`
- Iteration budget
- Acceptance rule

The assignment specifically requires everything except the neighborhood operator to remain identical.

### B1.4 — Run 5 times on at least two instances

**Base-code location:** experiment code around `runExp1()` **lines 419–458**.

Use:

```cpp
vector<double> costs;

for (int run = 0; run < 5; run++) {
    Annealing sa(
        1000.0, 0.995, 0.001, 100, 100000,
        InitType::RANDOM, 42 + run
    );

    Result r = sa.solve(g);
    costs.push_back(r.cost);
}

double best = *min_element(costs.begin(), costs.end());
double average =
    accumulate(costs.begin(), costs.end(), 0.0) / costs.size();
double worst = *max_element(costs.begin(), costs.end());

cout << "Best: " << best << "\n";
cout << "Average: " << average << "\n";
cout << "Worst: " << worst << "\n";
```

Repeat this for **at least two instances**.

### B1.5 — Report total accepted and worse-accepted moves

`Result` already contains these fields at **lines 124–125**:

```cpp
long long acceptedMoves = 0;
long long worseMovesAccepted = 0;
```

The counters are already updated in `simulatedAnnealing()`.

Accumulate them across the five runs:

```cpp
long long totalAccepted = 0;
long long totalWorseAccepted = 0;

for (int run = 0; run < 5; run++) {
    // ...
    Result r = sa.solve(g);

    totalAccepted += r.acceptedMoves;
    totalWorseAccepted += r.worseMovesAccepted;
}

cout << "Total Accepted Moves: "
     << totalAccepted << "\n";

cout << "Total Worse-Accepted Moves: "
     << totalWorseAccepted << "\n";
```

---

# Problem 3 — Section C1: Reheating

## Problem Statement

Once the temperature has dropped, SA essentially becomes a pure hill-climber and gets trapped in the nearest local optimum. A common fix is reheating: if the best-known cost has not improved for a while, jump the temperature back up so the algorithm can escape and try a different region.

### What to implement:

Modify your `simulatedAnnealing()` main loop as follows:

1. Keep track of `bestCost` (already done) and add a counter `stagnation` that counts the number of consecutive iterations in which `bestCost` did not improve.
2. Whenever a strictly better solution is found, reset `stagnation` to 0.
3. If `stagnation` reaches a threshold `S` (default `S = 5`), reheat: set `T ← T0/2`, reset `stagnation` to 0, and increment a counter `reheatCount`.
4. Cap the number of reheats at `R = 3` (after that, do no more reheats but continue until the iteration budget runs out).
5. Everything else (neighbor operator, cooling schedule, acceptance rule) stays identical.

### Output

- Run modified SA 5 times on a test case and report best, average, worst cost.
- Report the number of reheats that actually occurred in each run.

## Solution

### C1.1 — Add `reheatCount` to `Result`

**Base-code location:** `Result` is at **lines 113–126**.

Add after line 125:

```cpp
// ADD after line 125
long long reheatCount = 0;
```

### C1.2 — Add stagnation variables

**Base-code location:** `simulatedAnnealing()` lines **229–231** currently contain the counters.

Add:

```cpp
// ADD after the existing counters around lines 229–231
const int S = 5;
const int MAX_REHEATS = 3;

int stagnation = 0;
int reheatCount = 0;
```

### C1.3 — Track whether the best solution improved

Inside the per-iteration loop, immediately after the loop begins, add:

```cpp
// ADD near the start of each SA iteration
bool improvedBest = false;
```

The existing best-update block is at **lines 253–256**. Change it to:

```cpp
// MODIFY lines 253–256
if (currentCost < bestCost) {
    bestCost = currentCost;
    best = current;

    improvedBest = true;
}
```

### C1.4 — Update stagnation

After the iteration counter is incremented at **line 258**, add:

```cpp
// ADD after line 258
if (improvedBest) {
    stagnation = 0;
} else {
    stagnation++;
}
```

This counts consecutive iterations in which there was no strictly better best solution.

### C1.5 — Reheat at S = 5, with maximum R = 3

Immediately after the stagnation update, add:

```cpp
// ADD after the stagnation update
if (stagnation >= S && reheatCount < MAX_REHEATS) {
    T = t0 / 2.0;
    stagnation = 0;
    reheatCount++;
}
```

This implements all required C1 behavior:

- threshold `S = 5`
- `T ← T0/2`
- reset stagnation
- increment `reheatCount`
- maximum 3 reheats

After 3 reheats, the condition becomes false, so SA continues until the iteration budget runs out without further reheating.

### C1.6 — Store the reheat count in `Result`

**Base-code location:** result construction at **lines 264–277**.

Add before `return res;`:

```cpp
// ADD before return res; around line 276
res.reheatCount = reheatCount;
```

### C1.7 — Print reheats

**Base-code location:** `printResults()` **lines 283–305**.

After line 301, add:

```cpp
// ADD after line 301
cout << "Reheat Count: "
     << r.reheatCount << "\n";
```

### C1.8 — Run 5 times and report best/average/worst

Add a C1 experiment in the `Experiment` class, around **line 405 onward**, or create a dedicated function.

```cpp
vector<double> costs;

for (int run = 0; run < 5; run++) {
    Annealing sa(
        1000.0, 0.995, 0.001, 100, 100000,
        InitType::RANDOM, 42 + run
    );

    Result r = sa.solve(g);

    costs.push_back(r.cost);

    cout << "Run " << run + 1
         << ": Cost = " << r.cost
         << ", Reheats = " << r.reheatCount
         << "\n";
}

double best = *min_element(costs.begin(), costs.end());
double average =
    accumulate(costs.begin(), costs.end(), 0.0) / costs.size();
double worst = *max_element(costs.begin(), costs.end());

cout << "Best: " << best << "\n";
cout << "Average: " << average << "\n";
cout << "Worst: " << worst << "\n";
```

This reports the required best/average/worst cost and the actual reheat count for every run.

---

# Problem 4 — Section C2: Multi-Start Semi-Greedy

## Problem Statement

Your current greedy always picks the single nearest unvisited city. That is fully deterministic, so it can get stuck in one bad tour with no way to escape. In this task you will add a small amount of randomness to the greedy choice and run the construction several times, keeping the best tour.

### What to implement.

1. Write a new function `semiGreedyTSP(k)` that behaves like the original `greedyTSP()` except:
   - At each step from the current city, sort the unvisited cities by cost and keep the `k` nearest ones (Restricted Candidate List, or RCL). If fewer than `k` unvisited cities remain, use all of them.
   - Pick one city from the RCL uniformly at random as the next city.
2. Write a driver `multiStartSemiGreedy(k, R)` that runs `semiGreedyTSP(k)` `R` times (with a different random seed each time) and returns the best tour among the `R` runs.
3. Use default parameters `k = 7` and `R = 13`. (yes, you may be lucky and unlucky at the same time!)
4. The tour must still start and end at city 0 (that part is unchanged).

### What to show in the output.

- The best, average, and worst tour cost across the `R` runs.
- The best tour itself, printed as `0 -> ... -> 0`.
- A comparison line: original deterministic Nearest Neighbor cost vs. your best semi-greedy cost on the same instance.
- Run the experiment on at least two of your test instances.

## Solution

### C2.1 — Add `<limits>` if needed

**Base-code location:** headers **lines 81–92**.

If you use `numeric_limits<double>::infinity()`, add:

```cpp
// ADD with the headers around lines 81–92
#include <limits>
```

### C2.2 — Add `semiGreedyTSP(k)`

**Base-code location:** original `greedyTSP()` is at **lines 166–187**.

Add this immediately after line 187:

```cpp
// ADD after basecode.cpp line 187
Arr semiGreedyTSP(
    const Matrix& cost,
    int k,
    mt19937& rng,
    int start = 0
) {
    int n = static_cast<int>(cost.size());

    vector<bool> visited(n, false);
    Arr tour;
    tour.reserve(n);

    int current = start;
    visited[current] = true;
    tour.push_back(current);

    for (int step = 1; step < n; step++) {

        vector<pair<double, int>> candidates;

        // Collect all unvisited cities.
        for (int city = 0; city < n; city++) {
            if (!visited[city]) {
                candidates.push_back({
                    cost[current][city],
                    city
                });
            }
        }

        // Sort by cost: nearest first.
        sort(candidates.begin(), candidates.end());

        // Keep k nearest, or all if fewer than k remain.
        int rclSize =
            min(k, static_cast<int>(candidates.size()));

        // Uniformly choose one candidate from RCL.
        uniform_int_distribution<int> pick(0, rclSize - 1);

        int chosenIndex = pick(rng);
        int nextCity = candidates[chosenIndex].second;

        visited[nextCity] = true;
        tour.push_back(nextCity);
        current = nextCity;
    }

    return tour;
}
```

### C2.3 — Add `multiStartSemiGreedy(k, R)`

**Base-code location:** Add this immediately after `semiGreedyTSP()`.

Use different seeds for each run:

```cpp
// ADD after semiGreedyTSP()
Arr multiStartSemiGreedy(
    const Matrix& cost,
    int k = 7,
    int R = 13,
    unsigned seed = 42
) {
    Arr bestTour;
    double bestCost =
        numeric_limits<double>::infinity();

    for (int run = 0; run < R; run++) {

        // Different random seed for every run.
        mt19937 runRng(seed + run);

        Arr tour =
            semiGreedyTSP(cost, k, runRng, 0);

        double tourCost =
            calculateTourCost(cost, tour);

        if (tourCost < bestCost) {
            bestCost = tourCost;
            bestTour = tour;
        }
    }

    return bestTour;
}
```

### C2.4 — Collect best, average, worst across R = 13 runs

The assignment requires statistics **across the R runs**, so keep the individual costs in the experiment.

```cpp
const int K = 7;
const int R = 13;

vector<double> costs;
vector<Arr> tours;

for (int run = 0; run < R; run++) {
    mt19937 runRng(42 + run);

    Arr tour =
        semiGreedyTSP(g.matrix(), K, runRng, 0);

    double tourCost =
        calculateTourCost(g.matrix(), tour);

    costs.push_back(tourCost);
    tours.push_back(tour);
}

int bestIndex =
    static_cast<int>(
        min_element(costs.begin(), costs.end())
        - costs.begin()
    );

double best = costs[bestIndex];

double average =
    accumulate(costs.begin(), costs.end(), 0.0)
    / costs.size();

double worst =
    *max_element(costs.begin(), costs.end());

cout << "Best Cost: " << best << "\n";
cout << "Average Cost: " << average << "\n";
cout << "Worst Cost: " << worst << "\n";

cout << "Best Tour:\n";
printTour(tours[bestIndex]);
```

### C2.5 — Print the best tour as `0 -> ... -> 0`

The existing `printTour()` is at **lines 279–281**:

```cpp
static void printTour(const Arr& tour) {
    for (size_t i = 0; i < tour.size(); i++)
        cout << tour[i] << " -> ";

    cout << tour[0] << "\n";
}
```

Do not change it. It already prints the return to city 0.

### C2.6 — Compare against deterministic Nearest Neighbor

The original deterministic method is `greedyTSP()` at **lines 166–187**.

Add in the C2 experiment:

```cpp
Arr nnTour = greedyTSP(g.matrix(), 0);

double nnCost =
    calculateTourCost(g.matrix(), nnTour);

cout << "Comparison: Nearest Neighbor = "
     << nnCost
     << ", Best Semi-Greedy = "
     << best
     << "\n";
```

This is the required comparison on the same instance.

### C2.7 — Run on at least two test instances

The supplied `all` mode creates five test instances at **lines 602–615**:

```cpp
{"Test1_10", 10},
{"Test2_20", 20},
{"Test3_50", 50},
{"Test4_100", 100},
{"Test5_200", 200}
```

For C2, run the experiment on at least two, for example:

```cpp
// ADD in the C2 experiment
for (int instanceIndex = 0; instanceIndex < 2; instanceIndex++) {

    const string& name =
        instances[instanceIndex].first;

    const Graph& g =
        instances[instanceIndex].second;

    cout << "\n===== "
         << name
         << " : SEMI-GREEDY =====\n";

    // Run the K=7, R=13 experiment here.
}
```

---

# Complete Change Map

| Problem | Exact original location | Required change |
|---|---:|---|
| A1 | `simulatedAnnealing()` lines 212–277 | Replace cooling loop with W-iteration adaptive cooling |
| A1 | SA loop lines 233–261 | Cool every W=5 iterations and select alpha from acceptance ratio |
| A1 | `runExp1()` lines 419–458 | 5 runs, ≥2 instances, best/avg/worst; compare fixed 0.995 vs adaptive |
| B1 | `generateNeighbor()` lines 200–210 | Add `generateNeighbor3Opt()` immediately after line 210 |
| B1 | `simulatedAnnealing()` line 235 | Change neighbor call to `generateNeighbor3Opt()` |
| B1 | experiment around lines 419–458 | 5 runs, ≥2 instances, best/avg/worst, total accepted and worse-accepted |
| C1 | `Result` lines 113–126 | Add `reheatCount` after line 125 |
| C1 | `simulatedAnnealing()` lines 229–258 | Add stagnation counter and improvement tracking |
| C1 | after iteration update around line 258 | Reheat at S=5 with `T=T0/2`, maximum 3 times |
| C1 | result creation lines 264–277 | Store `reheatCount` |
| C1 | `printResults()` lines 283–305 | Print reheat count |
| C1 | `Experiment` around line 405 onward | 5 runs, best/avg/worst, reheats per run |
| C2 | headers lines 81–92 | Add `<limits>` if using `numeric_limits` |
| C2 | `greedyTSP()` lines 166–187 | Add `semiGreedyTSP()` after line 187 |
| C2 | after `semiGreedyTSP()` | Add `multiStartSemiGreedy(k,R)` |
| C2 | new experiment | R=13: best/avg/worst, best tour, NN comparison, ≥2 instances |

---

# Important Requirement Checklist

## A1
- [x] W = 5 or 10
- [x] Cooling every W iterations
- [x] Acceptance ratio uses last W iterations
- [x] `r > 0.5` → `alpha = 0.90`
- [x] `r < 0.1` → `alpha = 0.999`
- [x] Otherwise → `alpha = 0.995`
- [x] Update `T ← alpha*T`
- [x] Reset window acceptance counter
- [x] Keep T0
- [x] Keep Tmin
- [x] Keep iteration budget
- [x] Keep neighborhood
- [x] Keep acceptance rule
- [x] Run 5 times
- [x] Use at least 2 instances
- [x] Report best/average/worst
- [x] Compare with original fixed alpha = 0.995
- [x] Report best/avg/worst for both

## B1
- [x] Implement `generateNeighbor3Opt(tour)`
- [x] Pick `i < j < k` uniformly
- [x] `1 ≤ i < j < k ≤ N−1`
- [x] City 0 remains position 0
- [x] Split into A/B/C/D exactly as specified
- [x] Return `A + reverse(B) + reverse(C) + D`
- [x] Both middle segments reversed
- [x] Replace current neighbor or use consistently as an option
- [x] Keep initial solution identical
- [x] Keep temperature identical
- [x] Keep cooling identical
- [x] Keep acceptance identical
- [x] Run 5 times
- [x] Use at least 2 instances
- [x] Report best/average/worst
- [x] Report total accepted moves
- [x] Report total worse-accepted moves

## C1
- [x] Keep bestCost
- [x] Add stagnation counter
- [x] Count consecutive iterations without bestCost improvement
- [x] Reset stagnation to 0 after strictly better solution
- [x] S = 5
- [x] Reheat at S
- [x] Set `T ← T0/2`
- [x] Reset stagnation after reheat
- [x] Increment reheatCount
- [x] Maximum R = 3 reheats
- [x] After 3, continue until iteration budget runs out
- [x] Keep neighbor unchanged
- [x] Keep cooling unchanged
- [x] Keep acceptance unchanged
- [x] Run 5 times
- [x] Report best/average/worst
- [x] Report actual reheats for every run

## C2
- [x] Implement `semiGreedyTSP(k)`
- [x] Behaves like original greedy except for randomized selection
- [x] Sort unvisited cities by cost
- [x] Keep k nearest cities
- [x] If fewer than k remain, use all
- [x] RCL selection is uniform
- [x] Implement `multiStartSemiGreedy(k,R)`
- [x] Run semi-greedy R times
- [x] Different random seed each time
- [x] Return best tour
- [x] Default k = 7
- [x] Default R = 13
- [x] Tour starts at city 0
- [x] Tour returns to city 0
- [x] Report best cost
- [x] Report average cost
- [x] Report worst cost
- [x] Print best tour as `0 -> ... -> 0`
- [x] Compare deterministic Nearest Neighbor cost vs best semi-greedy cost
- [x] Run on at least 2 test instances
