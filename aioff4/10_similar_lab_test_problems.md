# CSE 318 Lab-Test Practice — 10 Similar Problems

> These problems are designed in the same style as the supplied TSP / Simulated Annealing lab questions.
>
> **Line-number note:** Line numbers refer to the supplied `basecode.cpp` structure. If your lab starter code differs, use the function name as the primary anchor and adjust the line number.

---

# Problem 1 — Adaptive Cooling

## Problem Statement

The original Simulated Annealing algorithm uses a fixed cooling rate. Modify the SA algorithm so that the cooling rate changes according to the acceptance ratio in a recent window.

### What to implement

1. Set `W = 10`.
2. Every `W` iterations calculate:
   `r = accepted moves in the last W iterations / W`.
3. If `r > 0.6`, use `alpha = 0.90`.
4. If `r < 0.2`, use `alpha = 0.998`.
5. Otherwise use `alpha = 0.995`.
6. Update `T = alpha * T`.
7. Reset the window acceptance counter.
8. Keep `T0`, `Tmin`, iteration budget, neighborhood and acceptance rule unchanged.
9. Run 5 times on at least two instances.
10. Report best, average and worst cost.
11. Compare against the original fixed-alpha SA.

## Solution

### Add/change `simulatedAnnealing()`

**Location:** `simulatedAnnealing()` around **lines 212–277**. Add the window variables around **lines 229–231**:

```cpp
const int W = 10;
int windowIterations = 0;
int windowAccepted = 0;
```

After an accepted move, count it:

```cpp
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
```

After each iteration:

```cpp
iterCount++;
windowIterations++;

if (windowIterations == W) {

    double r =
        static_cast<double>(windowAccepted) / W;

    double currentAlpha;

    if (r > 0.6)
        currentAlpha = 0.90;
    else if (r < 0.2)
        currentAlpha = 0.998;
    else
        currentAlpha = 0.995;

    T *= currentAlpha;

    windowIterations = 0;
    windowAccepted = 0;
}
```

### Run the experiment

**Location:** experiment section around **lines 419–458**.

```cpp
vector<double> costs;

for (int run = 0; run < 5; run++) {

    Annealing sa(
        1000.0, 0.995, 0.001,
        100, 100000,
        InitType::RANDOM,
        42 + run
    );

    Result r = sa.solve(g);
    costs.push_back(r.cost);
}

double best = *min_element(costs.begin(), costs.end());

double average =
    accumulate(costs.begin(), costs.end(), 0.0)
    / costs.size();

double worst = *max_element(costs.begin(), costs.end());
```

---

# Problem 2 — 2-Opt Neighborhood

## Problem Statement

The current neighbor operator reverses a segment of the tour. Implement a separate 2-opt neighbor.

### What to implement

1. Write `generateNeighbor2Opt(tour)`.
2. Choose two positions `i < j`.
3. Require `1 <= i < j <= N-1`.
4. Do not choose adjacent positions.
5. Reverse `tour[i ... j]`.
6. City 0 must remain at position 0.
7. Keep the original SA parameters and acceptance rule.
8. Run 5 times on at least two instances.
9. Report best, average and worst.

## Solution

### Add the function

**Location:** Immediately after `generateNeighbor()`, around **line 210**.

```cpp
Arr generateNeighbor2Opt(
    const Arr& tour,
    mt19937& rng
) {
    int n = static_cast<int>(tour.size());

    if (n < 4)
        return tour;

    uniform_int_distribution<int> dist(1, n - 1);

    int i, j;

    do {
        i = dist(rng);
        j = dist(rng);

        if (i > j)
            swap(i, j);

    } while (j - i <= 1);

    Arr neighbor = tour;

    reverse(
        neighbor.begin() + i,
        neighbor.begin() + j + 1
    );

    return neighbor;
}
```

### Replace the neighbor call

**Location:** `simulatedAnnealing()`, around **line 235**.

```cpp
Arr neighbor =
    generateNeighbor2Opt(current, rng);
```

Do not change anything else.

---

# Problem 3 — Stagnation-Based Restart

## Problem Statement

SA may remain around the same solution for many iterations. Add a stagnation counter.

### What to implement

1. Add `stagnation`.
2. Set `S = 20`.
3. Reset stagnation when `bestCost` strictly improves.
4. Otherwise increment it.
5. When stagnation reaches `S`, replace the current solution by the best solution found so far.
6. Set `currentCost = bestCost`.
7. Reset stagnation.
8. Do not change temperature.
9. Keep the original acceptance rule.
10. Run 5 times and report best, average and worst.

## Solution

### Add the counter

**Location:** `simulatedAnnealing()`, around **lines 229–231**.

```cpp
const int S = 20;
int stagnation = 0;
```

At the beginning of each iteration around **line 234**:

```cpp
bool improvedBest = false;
```

Modify the best-update block around **lines 253–256**:

```cpp
if (currentCost < bestCost) {
    bestCost = currentCost;
    best = current;

    improvedBest = true;
}
```

After `iterCount++`:

```cpp
if (improvedBest)
    stagnation = 0;
else
    stagnation++;
```

Then:

```cpp
if (stagnation >= S) {

    current = best;
    currentCost = bestCost;

    stagnation = 0;
}
```

---

# Problem 4 — Reheating with Maximum Two Reheats

## Problem Statement

Modify SA so that the temperature is increased when no improvement has occurred for several iterations.

### What to implement

1. Set stagnation threshold `S = 15`.
2. When stagnation reaches 15, set `T = 0.4 * T0`.
3. Reset stagnation.
4. Increment `reheatCount`.
5. Allow at most `R = 2` reheats.
6. After two reheats, continue normally without reheating.
7. Keep neighborhood, cooling schedule and acceptance rule unchanged.
8. Store `reheatCount` in `Result`.
9. Run 5 times.
10. Report best, average, worst and reheats per run.

## Solution

### Add result field

**Location:** `Result`, around **lines 113–126**.

```cpp
long long reheatCount = 0;
```

### Add counters

**Location:** `simulatedAnnealing()`, around **lines 229–231**.

```cpp
const int S = 15;
const int MAX_REHEATS = 2;

int stagnation = 0;
int reheatCount = 0;
```

At the beginning of each iteration:

```cpp
bool improvedBest = false;
```

Modify the best update:

```cpp
if (currentCost < bestCost) {
    bestCost = currentCost;
    best = current;

    improvedBest = true;
}
```

After the iteration:

```cpp
if (improvedBest)
    stagnation = 0;
else
    stagnation++;
```

Reheat:

```cpp
if (stagnation >= S &&
    reheatCount < MAX_REHEATS) {

    T = 0.4 * t0;

    stagnation = 0;
    reheatCount++;
}
```

Before `return res;`, around **line 276**:

```cpp
res.reheatCount = reheatCount;
```

---

# Problem 5 — Equal-Cost Acceptance

## Problem Statement

Modify the SA acceptance rule.

### What to implement

1. Better solutions are always accepted.
2. Equal-cost solutions are accepted with probability `0.5`.
3. Worse solutions use the normal Metropolis probability.
4. Keep temperature, cooling and neighborhood unchanged.
5. Run 5 times.
6. Report best, average and worst.

## Solution

### Replace acceptance logic

**Location:** `simulatedAnnealing()`, around **lines 238–248**.

```cpp
bool doAccept = false;

if (delta < 0.0) {

    doAccept = true;

}
else if (delta == 0.0) {

    doAccept = (unif(rng) < 0.5);

}
else {

    double p = exp(-delta / T);

    if (unif(rng) < p) {
        doAccept = true;
        worseAccepted++;
    }
}
```

No other SA component needs to change.

---

# Problem 6 — Randomized Restricted Candidate List Greedy

## Problem Statement

The deterministic nearest-neighbor algorithm always selects the closest unvisited city. Add controlled randomness using an RCL.

### What to implement

1. Implement `rclGreedyTSP(k)`.
2. At every step sort unvisited cities by cost.
3. Keep the `k` nearest cities.
4. If fewer than `k` remain, keep all of them.
5. Choose one RCL city uniformly.
6. Start from city 0.
7. Complete the tour.
8. Run 10 times.
9. Report best, average and worst.
10. Print the best tour.
11. Compare against deterministic nearest neighbor.

## Solution

### Add the function

**Location:** Immediately after `greedyTSP()`, around **line 187**.

```cpp
Arr rclGreedyTSP(
    const Matrix& cost,
    int k,
    mt19937& rng
) {
    int n = static_cast<int>(cost.size());

    vector<bool> visited(n, false);
    Arr tour;

    tour.push_back(0);
    visited[0] = true;

    int current = 0;

    while (static_cast<int>(tour.size()) < n) {

        vector<pair<double, int>> candidates;

        for (int city = 0; city < n; city++) {
            if (!visited[city]) {
                candidates.push_back({
                    cost[current][city],
                    city
                });
            }
        }

        sort(candidates.begin(), candidates.end());

        int rclSize =
            min(k, static_cast<int>(candidates.size()));

        uniform_int_distribution<int> pick(
            0, rclSize - 1
        );

        int next =
            candidates[pick(rng)].second;

        visited[next] = true;
        tour.push_back(next);
        current = next;
    }

    return tour;
}
```

### Run 10 times

```cpp
const int K = 5;
const int RUNS = 10;

vector<double> costs;
vector<Arr> tours;

for (int run = 0; run < RUNS; run++) {

    mt19937 rng(100 + run);

    Arr tour =
        rclGreedyTSP(g.matrix(), K, rng);

    double c =
        calculateTourCost(g.matrix(), tour);

    costs.push_back(c);
    tours.push_back(tour);
}
```

Use `min_element`, `accumulate`, and `max_element` for best/average/worst.

---

# Problem 7 — Alternating 2-Opt and 3-Opt SA

## Problem Statement

Use two neighborhood operators in the same SA run.

### What to implement

1. On even iterations use the normal neighbor.
2. On odd iterations use `generateNeighbor3Opt()`.
3. Keep temperature unchanged.
4. Keep cooling unchanged.
5. Keep acceptance unchanged.
6. Keep iteration budget unchanged.
7. Run 5 times.
8. Report best, average, worst.
9. Report total accepted and worse-accepted moves.

## Solution

### Change neighbor selection

**Location:** `simulatedAnnealing()`, around **line 235**.

```cpp
Arr neighbor;

if (iterCount % 2 == 0) {

    neighbor =
        generateNeighbor(current, rng);

}
else {

    neighbor =
        generateNeighbor3Opt(current, rng);
}
```

Accumulate counters across runs:

```cpp
long long totalAccepted = 0;
long long totalWorseAccepted = 0;

for (int run = 0; run < 5; run++) {

    Result r = sa.solve(g);

    totalAccepted += r.acceptedMoves;
    totalWorseAccepted +=
        r.worseMovesAccepted;
}
```

---

# Problem 8 — Temperature-Dependent Neighborhood

## Problem Statement

Use a larger neighborhood while the algorithm is hot and a smaller neighborhood while it is cold.

### What to implement

1. If `T > 0.5*T0`, use 3-opt.
2. Otherwise use the original neighborhood.
3. Keep the original cooling schedule.
4. Keep the original acceptance rule.
5. Keep the original iteration budget.
6. Run 5 times.
7. Report best, average and worst.

## Solution

### Change neighbor generation

**Location:** `simulatedAnnealing()`, around **line 235**.

```cpp
Arr neighbor;

if (T > 0.5 * t0) {

    neighbor =
        generateNeighbor3Opt(current, rng);

}
else {

    neighbor =
        generateNeighbor(current, rng);
}
```

No other SA code changes.

---

# Problem 9 — Adaptive Cooling Using Worse Accepted Moves

## Problem Statement

Instead of using all accepted moves to control cooling, use only worse moves that were accepted by the Metropolis rule.

### What to implement

1. Set `W = 10`.
2. Count worse-accepted moves during the last 10 iterations.
3. Calculate:
   `r = worse accepted moves / W`.
4. If `r > 0.3`, use `alpha = 0.90`.
5. If `r < 0.05`, use `alpha = 0.999`.
6. Otherwise use `alpha = 0.995`.
7. Cool every 10 iterations.
8. Reset the window.
9. Keep everything else unchanged.
10. Run 5 times.
11. Report best, average and worst.

## Solution

### Add counters

**Location:** `simulatedAnnealing()`, around **lines 229–231**.

```cpp
const int W = 10;

int windowIterations = 0;
int windowWorseAccepted = 0;
```

Inside the worse-acceptance block around **lines 243–248**:

```cpp
if (r < p) {

    doAccept = true;

    worseAccepted++;
    windowWorseAccepted++;
}
```

After `iterCount++`:

```cpp
windowIterations++;

if (windowIterations == W) {

    double r =
        static_cast<double>(
            windowWorseAccepted
        ) / W;

    double currentAlpha;

    if (r > 0.3)
        currentAlpha = 0.90;
    else if (r < 0.05)
        currentAlpha = 0.999;
    else
        currentAlpha = 0.995;

    T *= currentAlpha;

    windowIterations = 0;
    windowWorseAccepted = 0;
}
```

---

# Problem 10 — Multi-Start Simulated Annealing

## Problem Statement

A single SA run can produce a poor result because of its random initialization and search path. Implement a multi-start version.

### What to implement

1. Run SA `R = 10` times.
2. Use a different seed for every run.
3. Generate a fresh random initial solution each time.
4. Keep the best tour among all runs.
5. Report best cost.
6. Report average cost.
7. Report worst cost.
8. Print the best tour.
9. Report total accepted moves.
10. Report total worse-accepted moves.
11. Run on at least two instances.
12. Do not change the internal SA algorithm.

## Solution

### Add the multi-start driver

**Location:** Add to the experiment section around **line 405 onward**.

```cpp
void multiStartSA(
    const Graph& g,
    int R = 10,
    unsigned seed = 42
) {
    vector<double> costs;

    Arr bestTour;
    double bestCost =
        numeric_limits<double>::infinity();

    long long totalAccepted = 0;
    long long totalWorseAccepted = 0;

    for (int run = 0; run < R; run++) {

        Annealing sa(
            1000.0,
            0.995,
            0.001,
            100,
            100000,
            InitType::RANDOM,
            seed + run
        );

        Result r = sa.solve(g);

        costs.push_back(r.cost);

        totalAccepted +=
            r.acceptedMoves;

        totalWorseAccepted +=
            r.worseMovesAccepted;

        if (r.cost < bestCost) {
            bestCost = r.cost;
            bestTour = r.tour;
        }
    }

    double average =
        accumulate(
            costs.begin(),
            costs.end(),
            0.0
        ) / costs.size();

    double worst =
        *max_element(
            costs.begin(),
            costs.end()
        );

    cout << "Best Cost: "
         << bestCost << "\n";

    cout << "Average Cost: "
         << average << "\n";

    cout << "Worst Cost: "
         << worst << "\n";

    cout << "Best Tour:\n";
    printTour(bestTour);

    cout << "Total Accepted Moves: "
         << totalAccepted << "\n";

    cout << "Total Worse-Accepted Moves: "
         << totalWorseAccepted << "\n";
}
```

If needed, add `<limits>` with the headers around **lines 81–92**:

```cpp
#include <limits>
```

Run on two instances:

```cpp
multiStartSA(g1, 10, 100);
multiStartSA(g2, 10, 200);
```

---

# Lab-Test Quick Revision Sheet

## 1. Counter

```cpp
int counter = 0;

counter++;

if (counter >= S) {
    // action
    counter = 0;
}
```

## 2. Detect best improvement

```cpp
bool improvedBest = false;

if (currentCost < bestCost) {
    bestCost = currentCost;
    best = current;
    improvedBest = true;
}
```

## 3. Adaptive cooling

```cpp
double r =
    static_cast<double>(acceptedInWindow) / W;

if (r > HIGH)
    alpha = FAST;
else if (r < LOW)
    alpha = SLOW;
else
    alpha = DEFAULT;

T *= alpha;
```

## 4. Reheating

```cpp
if (stagnation >= S &&
    reheatCount < MAX_REHEATS) {

    T = t0 / 2.0;
    stagnation = 0;
    reheatCount++;
}
```

## 5. Best / Average / Worst

```cpp
double best =
    *min_element(costs.begin(), costs.end());

double average =
    accumulate(
        costs.begin(),
        costs.end(),
        0.0
    ) / costs.size();

double worst =
    *max_element(costs.begin(), costs.end());
```

## 6. Different random seed

```cpp
mt19937 rng(seed + run);
```

## 7. RCL

```cpp
sort(candidates.begin(), candidates.end());

int rclSize =
    min(k, static_cast<int>(candidates.size()));

uniform_int_distribution<int> pick(
    0, rclSize - 1
);

int next =
    candidates[pick(rng)].second;
```

## 8. 3-Opt structure

```text
A + reverse(B) + reverse(C) + D
```

with:

```text
1 <= i < j < k <= N-1
```

## 9. Multi-start

```cpp
for (int run = 0; run < R; run++) {

    mt19937 rng(seed + run);

    // generate/run solution

    if (cost < bestCost) {
        bestCost = cost;
        bestTour = tour;
    }
}
```

## 10. Most important lab rule

If the question says **"keep everything else unchanged"**, modify only the requested component.

If it asks for **5 runs**, use 5 independent seeds.

If it asks for **best, average, worst**, calculate all three.

If it asks for **at least two instances**, test at least two.

If it asks for **total accepted moves**, sum the counters across the required runs.

