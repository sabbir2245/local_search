# CSE 318 --- Online 4: `explain.md`

This guide is based on the supplied `basecode.cpp` (547 lines) and the
assignment requirements. The snippets show only the parts that need to
be added or changed.

## 1. What the base program does

The program solves TSP in two ways:

-   **Greedy / Nearest Neighbor**
-   **Simulated Annealing (SA)**

Important locations in `basecode.cpp`:

  ---------------------------------------------------------------------------
                         Lines Code                     Purpose
  ---------------------------- ------------------------ ---------------------
                        42--55 `Result`                 Stores solution and
                                                        SA statistics

                        56--71 `readInput()`            Reads `N` and the
                                                        cost matrix

                        72--80 `calculateTourCost()`    Calculates tour cost
                                                        including return to
                                                        city 0

                        82--93 `isValidTour()`          Checks tour
                                                        correctness

                       95--116 `greedyTSP()`            Nearest-neighbor
                                                        construction

                      118--127 `generateRandomTour()`   Random initial
                                                        solution

                      129--139 `generateNeighbor()`     Current
                                                        segment-reversal
                                                        neighbor

                      141--207 `simulatedAnnealing()`   Main SA algorithm

                      212--234 `printResults()`         Prints results

                      263--278 `Graph`                  TSP instance wrapper

                      284--301 `Greedy`                 Greedy strategy

                      304--331 `Annealing`              SA strategy

                      334--448 `Experiment`             Experiments

                      449--469 `runSingle()`            Single-instance run

                      475--556 `main()`                 Command-line control
  ---------------------------------------------------------------------------

The base code's main SA loop is around lines **162--190**. It generates
a neighbor, calculates its cost, applies the acceptance rule, updates
the current/best solution, counts iterations, and then performs fixed
cooling.

## 2. TSP rules

A valid tour must:

1.  Start at the designated starting city; the assignment uses **city
    0**.
2.  Visit every city exactly once.
3.  Return to the starting city.
4.  Minimize total cost.

`calculateTourCost()` at lines 72--80 uses:

``` cpp
int v = tour[(i + 1) % n];
```

so the final edge `last city -> city 0` is automatically included.

`isValidTour()` at lines 82--93 checks size, starting city, range,
duplicates, and that every city appears.

## 3. Greedy algorithm

The required greedy technique is Nearest Neighbor:

``` text
start at city 0
    ↓
choose cheapest unvisited city
    ↓
move there
    ↓
repeat until all cities are visited
    ↓
return to city 0
```

The existing implementation at lines 95--116 already satisfies this.

Greedy is deterministic and only considers the immediate cheapest edge,
so it is **not guaranteed to find the globally optimal tour**.

------------------------------------------------------------------------

# Part A1 --- Adaptive Cooling

## Problem statement

The original SA uses fixed geometric cooling:

``` text
T ← alpha × T
```

Modify it so cooling happens every **W iterations**, with `W = 5` or
`10`.

For the most recent W iterations:

``` text
r = accepted moves / W
```

Choose:

``` text
r > 0.5  → alpha = 0.90
r < 0.1  → alpha = 0.999
otherwise → alpha = 0.995
```

Then:

``` text
T ← alpha × T
```

Reset the window acceptance counter.

Keep the original `T0`, `Tmin`, iteration budget, neighborhood, and
acceptance rule unchanged.

Run 5 times on at least 2 instances. Report best/average/worst and
compare with original fixed `alpha = 0.995`.

## A1 solution

**Modify `simulatedAnnealing()` at lines 141--207.**

### Step 1 --- Add counters

Near the existing counters around lines 158--161:

``` cpp
const int W = 5;
int windowIterations = 0;
int windowAccepted = 0;
```

### Step 2 --- Count accepted moves

Inside the existing:

``` cpp
if (doAccept) {
```

add:

``` cpp
windowAccepted++;
```

So the important part becomes:

``` cpp
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

### Step 3 --- Count iterations

After every SA iteration:

``` cpp
iterCount++;
windowIterations++;
```

### Step 4 --- Replace fixed cooling

The original code has:

``` cpp
T *= alpha;
```

around line 189. Replace that cooling behavior with:

``` cpp
if (windowIterations == W) {

    double r =
        static_cast<double>(windowAccepted) / W;

    double currentAlpha;

    if (r > 0.5)
        currentAlpha = 0.90;
    else if (r < 0.1)
        currentAlpha = 0.999;
    else
        currentAlpha = 0.995;

    T *= currentAlpha;

    windowIterations = 0;
    windowAccepted = 0;
}
```

### A1 memory trick

``` text
W iterations
     ↓
count accepted
     ↓
r = accepted / W
     ↓
high acceptance → .90
low acceptance  → .999
otherwise       → .995
     ↓
T *= alpha
     ↓
reset
```

Do **not** change the Metropolis acceptance rule for A1.

------------------------------------------------------------------------

# Part B1 --- 3-Opt Neighbor

## Problem statement

The current neighbor at lines 129--139 reverses one segment.

Replace it with the required 3-cut neighbor:

1.  Pick `1 <= i < j < k <= N-1` uniformly.
2.  Split:

``` text
A = tour[0 ... i-1]
B = tour[i ... j-1]
C = tour[j ... k-1]
D = tour[k ... N-1]
```

3.  Return:

``` text
A + reverse(B) + reverse(C) + D
```

City 0 must stay at position 0.

Keep initialization, temperature, cooling, acceptance rule, and
iteration budget unchanged.

Run 5 times on at least 2 instances and report best/average/worst,
accepted moves, and worse-accepted moves.

## B1 solution

**Replace `generateNeighbor()` at lines 129--139.**

``` cpp
Arr generateNeighbor(const Arr& tour, mt19937& rng) {
    int n = (int)tour.size();

    if (n <= 4)
        return tour;

    uniform_int_distribution<int> dist(1, n - 1);

    int i = dist(rng);
    int j = dist(rng);
    int k = dist(rng);

    while (j == i)
        j = dist(rng);

    while (k == i || k == j)
        k = dist(rng);

    // Make i < j < k.
    if (i > j) swap(i, j);
    if (j > k) swap(j, k);
    if (i > j) swap(i, j);

    Arr neighbor;

    // A
    neighbor.insert(
        neighbor.end(),
        tour.begin(),
        tour.begin() + i
    );

    // reverse(B)
    for (int p = j - 1; p >= i; --p)
        neighbor.push_back(tour[p]);

    // reverse(C)
    for (int p = k - 1; p >= j; --p)
        neighbor.push_back(tour[p]);

    // D
    neighbor.insert(
        neighbor.end(),
        tour.begin() + k,
        tour.end()
    );

    return neighbor;
}
```

### Why city 0 stays fixed

All cut positions are selected from:

``` cpp
1 ... n-1
```

Therefore position 0 is never cut/reordered.

### Example

``` text
tour = 0 1 2 3 4 5 6
i=1, j=3, k=5

A = 0
B = 1 2
C = 3 4
D = 5 6

result = 0 2 1 4 3 5 6
```

------------------------------------------------------------------------

# Part C1 --- Reheating

## Problem statement

When SA becomes cold, it can get trapped in a local optimum.

Track:

``` text
stagnation = consecutive iterations without improving bestCost
```

Requirements:

-   Strict improvement resets stagnation to 0.
-   Use `S = 5`.
-   When stagnation reaches 5:
    -   `T = T0 / 2`
    -   reset stagnation
    -   increment `reheatCount`
-   Maximum reheats `R = 3`.
-   After 3 reheats, continue normally but never reheat again.
-   Everything else remains unchanged.
-   Run 5 times and report best/average/worst plus reheats per run.

## C1 solution

### Step 1 --- Modify `Result`

`Result` is at lines 42--55.

Add:

``` cpp
int reheatCount = 0;
```

### Step 2 --- Add counters

Inside `simulatedAnnealing()` around lines 158--161:

``` cpp
int stagnation = 0;
int reheatCount = 0;

const int S = 5;
const int R = 3;
```

### Step 3 --- Update stagnation

Replace the existing best update:

``` cpp
if (currentCost < bestCost) {
    bestCost = currentCost;
    best = current;
}
```

with:

``` cpp
if (currentCost < bestCost) {
    bestCost = currentCost;
    best = current;

    stagnation = 0;
}
else {
    stagnation++;
}
```

Important: use `<`, not `<=`, because the requirement says **strictly
better**.

### Step 4 --- Reheat

Immediately after the stagnation logic:

``` cpp
if (stagnation >= S && reheatCount < R) {

    T = t0 / 2.0;

    stagnation = 0;
    reheatCount++;
}
```

### Step 5 --- Store result

Before returning `res` around lines 193--206:

``` cpp
res.reheatCount = reheatCount;
```

### Step 6 --- Print it

Inside the SA section of `printResults()` around lines 219--231:

``` cpp
cout << "Reheats: "
     << r.reheatCount << "\n";
```

### C1 memory trick

``` text
better?
  YES → stagnation = 0
  NO  → stagnation++

stagnation >= 5 and reheats < 3?
  YES → T = T0/2
        stagnation = 0
        reheats++
```

The temperature must be assigned:

``` cpp
T = t0 / 2.0;
```

not merely doubled.

------------------------------------------------------------------------

# Part C2 --- Multi-Start Semi-Greedy

## Problem statement

The original greedy always chooses the single nearest unvisited city.

Implement:

``` cpp
semiGreedyTSP(k)
```

At each step:

1.  Sort unvisited cities by cost.
2.  Keep the `k` nearest cities.
3.  If fewer than `k` remain, use all remaining cities.
4.  Choose uniformly at random from those candidates.

Then implement:

``` cpp
multiStartSemiGreedy(k, R)
```

Run the constructor `R` times with different seeds and keep the best.

Required defaults:

``` text
k = 7
R = 13
```

Start at city 0.

Report:

-   best cost
-   average cost
-   worst cost
-   best tour
-   deterministic nearest-neighbor cost
-   best semi-greedy cost

Run on at least 2 instances.

## C2 solution

Do **not** delete `greedyTSP()`. Add the new function after it, around
line 117.

``` cpp
Arr semiGreedyTSP(
    const Matrix& cost,
    int k,
    mt19937& rng,
    int start = 0
) {
    int n = (int)cost.size();

    vector<bool> visited(n, false);
    Arr tour;
    tour.reserve(n);

    int current = start;
    visited[current] = true;
    tour.push_back(current);

    for (int step = 1; step < n; ++step) {

        vector<pair<double, int>> candidates;

        for (int city = 0; city < n; ++city) {
            if (!visited[city]) {
                candidates.push_back({
                    cost[current][city],
                    city
                });
            }
        }

        sort(candidates.begin(), candidates.end());

        int rclSize =
            min(k, (int)candidates.size());

        uniform_int_distribution<int> pick(
            0, rclSize - 1
        );

        int selected = pick(rng);

        int nextCity =
            candidates[selected].second;

        visited[nextCity] = true;
        tour.push_back(nextCity);
        current = nextCity;
    }

    return tour;
}
```

If using `numeric_limits`, add:

``` cpp
#include <limits>
```

near the existing headers around lines 10--21.

## Multi-start statistics

A simple structure:

``` cpp
struct SemiGreedyStats {
    double bestCost;
    double averageCost;
    double worstCost;
    Arr bestTour;
};
```

Then:

``` cpp
SemiGreedyStats multiStartSemiGreedy(
    const Matrix& cost,
    int k = 7,
    int R = 13,
    unsigned baseSeed = 100
) {
    double bestCost =
        numeric_limits<double>::infinity();

    double worstCost =
        -numeric_limits<double>::infinity();

    double sum = 0.0;
    Arr bestTour;

    for (int run = 0; run < R; ++run) {

        mt19937 rng(baseSeed + run);

        Arr tour =
            semiGreedyTSP(cost, k, rng, 0);

        double c =
            calculateTourCost(cost, tour);

        sum += c;

        if (c < bestCost) {
            bestCost = c;
            bestTour = tour;
        }

        if (c > worstCost)
            worstCost = c;
    }

    return {
        bestCost,
        sum / R,
        worstCost,
        bestTour
    };
}
```

## RCL idea

If sorted candidates are:

``` text
city 4 → 10
city 2 → 12
city 7 → 15
city 5 → 18
city 1 → 20
```

and:

``` text
k = 3
```

then RCL is:

``` text
{4, 2, 7}
```

Choose uniformly from those 3. Do **not** choose from all unvisited
cities.

------------------------------------------------------------------------

# 6. Experiments required by the assignment

The original assignment requires testing on at least **five TSP
instances**. The base program's `all` mode already creates:

``` text
Test1_10   → 10 cities
Test2_20   → 20 cities
Test3_50   → 50 cities
Test4_100  → 100 cities
Test5_200  → 200 cities
```

The original experiments are:

### Experiment 1 --- Greedy vs SA

For each instance record:

-   Greedy cost
-   Best SA cost
-   Average SA cost
-   Percentage improvement
-   Greedy execution time
-   SA execution time

Base location: `Experiment::runExp1()` around lines **348--387**.

### Experiment 2 --- Different T0

Test at least:

``` text
100
1000
5000
```

Base location: lines **388--403**.

### Experiment 3 --- Different alpha

Test:

``` text
0.90
0.95
0.995
```

Base location: lines **404--418**.

### Experiment 4 --- Random vs Greedy initialization

Run SA with:

``` text
RANDOM
GREEDY_INIT
```

at least 5 times and compare:

-   best
-   average
-   worst
-   average time

Base location: lines **419--447**.

------------------------------------------------------------------------

# 7. Statistics pattern

For randomized algorithms, use multiple runs.

If:

``` text
500, 470, 490, 480, 460
```

then:

``` text
best  = minimum
worst = maximum
average = sum / number of runs
```

C++:

``` cpp
double best =
    *min_element(costs.begin(), costs.end());

double worst =
    *max_element(costs.begin(), costs.end());

double average =
    accumulate(
        costs.begin(),
        costs.end(),
        0.0
    ) / costs.size();
```

Use this pattern for the required A1/B1/C1/C2 experiments.

------------------------------------------------------------------------

# 8. Accepted moves vs worse-accepted moves

The base `Result` already has:

``` cpp
long long acceptedMoves;
long long worseMovesAccepted;
```

They mean different things.

**Accepted moves** = every move accepted.

**Worse-accepted moves** = only accepted moves where:

``` text
neighborCost > currentCost
```

So this is correct:

``` cpp
if (delta <= 0.0) {
    doAccept = true;
}
else {
    double p = exp(-delta / T);

    if (unif(rng) < p) {
        doAccept = true;
        worseAccepted++;
    }
}
```

Do not count a better move as a worse-accepted move.

------------------------------------------------------------------------

# 9. Complexity

### Greedy

Nearest Neighbor scans unvisited cities for each city:

``` text
O(N²)
```

### Tour cost

`calculateTourCost()` scans the tour once:

``` text
O(N)
```

### Simulated Annealing

If every iteration recalculates the complete tour cost:

``` text
O(N)
```

per iteration.

For `I` iterations:

``` text
O(I × N)
```

plus neighbor-generation overhead.

The assignment asks you to discuss how incremental cost calculations
could improve this.

------------------------------------------------------------------------

# 10. Common lab mistakes

### Mistake 1 --- Changing city 0

For these operators, positions should be selected from:

``` cpp
1 ... n-1
```

so city 0 remains fixed.

### Mistake 2 --- Forgetting the final return

The base cost function already adds:

``` text
last → first
```

through `% n`.

### Mistake 3 --- Confusing accepted and worse-accepted

All worse-accepted moves are accepted moves, but not all accepted moves
are worse moves.

### Mistake 4 --- C1 resetting stagnation on any accepted move

Wrong idea:

``` text
accepted → stagnation = 0
```

Correct:

``` text
strict improvement of bestCost → stagnation = 0
```

### Mistake 5 --- Unlimited reheating

Always check:

``` cpp
reheatCount < R
```

### Mistake 6 --- C2 choosing randomly from all unvisited cities

First create the RCL of the `k` nearest cities, then choose randomly
from that RCL.

### Mistake 7 --- Changing unrelated SA logic

For a modification question, change only the requested component unless
the specification explicitly requires more.

------------------------------------------------------------------------

# 11. Fast lab-test recognition

``` text
"Change alpha based on acceptance"
        ↓
A1
        ↓
modify simulatedAnnealing()
```

``` text
"Change neighborhood / 3-opt / reverse / swap"
        ↓
B1-type question
        ↓
modify generateNeighbor()
```

``` text
"No improvement for S iterations"
        ↓
C1-type question
        ↓
add stagnation + restart/reheat logic
```

``` text
"Choose one of k nearest"
        ↓
C2-type question
        ↓
build candidate list + random choice
```

------------------------------------------------------------------------

# 12. Final modification map

  -------------------------------------------------------------------------
  Section                 Main change             Base location
  ----------------------- ----------------------- -------------------------
  **A1 Adaptive Cooling** Window counters +       `simulatedAnnealing()`,
                          adaptive alpha          lines 141--207

  **B1 3-Opt**            Replace neighbor        `generateNeighbor()`,
                          operator                lines 129--139

  **C1 Reheating**        `Result` + stagnation + lines 42--55, 141--207,
                          reheating + output      212--234

  **C2 Semi-Greedy**      RCL construction +      add after `greedyTSP()`,
                          multi-start             around line 117
  -------------------------------------------------------------------------

## One-page memory sheet

``` text
A1:
W iterations
→ accepted/W
→ r > .5 : alpha=.90
→ r < .1 : alpha=.999
→ else    : alpha=.995
→ T *= alpha
→ reset window


B1:
pick i,j,k
1 <= i < j < k <= N-1
A | B | C | D
A + reverse(B) + reverse(C) + D


C1:
better → stagnation=0
else   → stagnation++
if stagnation>=5 AND reheats<3:
    T=T0/2
    stagnation=0
    reheats++


C2:
sort unvisited
→ keep k nearest
→ random pick from RCL
→ repeat
→ run R times with different seeds
→ keep best
k=7, R=13
```

## Submission checklist

-   [ ] Tour starts at city 0.
-   [ ] Every city appears exactly once.
-   [ ] Return to city 0 is included in cost.
-   [ ] A1 uses W=5 or 10.
-   [ ] A1 uses exactly the required acceptance-ratio thresholds.
-   [ ] A1 compares against fixed alpha 0.995.
-   [ ] B1 chooses three valid cut positions.
-   [ ] B1 returns `A + reverse(B) + reverse(C) + D`.
-   [ ] B1 reports accepted and worse-accepted moves.
-   [ ] C1 uses S=5 and maximum R=3.
-   [ ] C1 stores and prints `reheatCount`.
-   [ ] C2 uses k=7 and R=13.
-   [ ] C2 chooses uniformly from the RCL.
-   [ ] Required multiple-run statistics are reported.
-   [ ] At least two instances are used for A1/B1/C2.
-   [ ] Five instances are used for the original assignment experiments.
-   [ ] Complexity analysis is included.
