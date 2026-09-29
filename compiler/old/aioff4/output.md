## Travelling Salesman Problem (TSP) Using Greedy Technique and Simulated Annealing

## Contents 1 Assignment Overview

2 Problem Description

3 Input Format

4 Implementation  4.1 Greedy Algorithm (Nearest Neighbor) . . . . . . . . . . . . . . . . . . . .  4.2 Simulated Annealing . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  4.3 Key Implementation Details . . . . . . . . . . . . . . . . . . . . . . . . .

5 Experimental Setup

6 Experiment 1: Greedy vs. Simulated Annealing

7 Experiment 2: Varying Initial Temperature (

8 Experiment 3: Varying Cooling Rate (

9 Experiment 4: Random vs. Greedy Initialization

10 Complexity Analysis  10.1 Greedy Technique . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .  10.2 Simulated Annealing . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

11 Comparative Summary

12 Conclusion

13 Submission Contents

## AI Assignment 4

Sabbir Ahmmed Student ID: 2205040

September 7, 2026

α 1

2

2

2

3

3 3 3

4

4

T 5

0

6

7

8

8 8

9

9

10


---

AI Assignment 4 1 Assignment Overview

The Travelling Salesman Problem (TSP) is one of the most well-known NP-hard opti-mization problems in computer science. A salesman must visit a number of cities exactly once and return to the starting city, minimizing the total travelling cost. This assignment implements two approaches:

1. Greedy Technique (Nearest Neighbor) lution.
2. Simulated Annealing  a metaheuristic optimization technique designed to im-prove solutions and escape local optima.

The goal is to implement, test, compare, and analyze both techniques across multiple TSP instances of varying sizes.

2 Problem Description

Given N cities and a travelling cost between every pair of cities, the task is to nd a tour that:

1. Starts from a specied starting city (City 0).
2. Visits every city exactly once.
3. Returns to the starting city.
4. Minimizes the total travelling cost.

For a tour 0 c c → · · → c 0, the total cost is: 1 2

N 1

N 1X Cost

cost c, ci

Sabbir Ahmmed

to quickly construct a feasible so-

i+1) mod N

i=0 3 Input Format

The input is a text le with the following format:

N c[0][0] c[0][1] c[0][2] ... c[0][N-1] c[1][0] c[1][1] c[1][2] ... c[1][N-1]

... c[N-1][0] c[N-1][1] ... c[N-1][N-1]

Assumptions: 2 N 500

cost i, i) = 0

All other costs are positive.

The matrix may be symmetric or asymmetric.

City numbering starts from 0.

2


---

AI Assignment 4

Sabbir Ahmmed 4 Implementation

4.1 Greedy Algorithm (Nearest Neighbor)

The greedy heuristic builds a tour by always choosing the nearest unvisited city:

1. Start at City 0. Mark it as visited.
2. Among all unvisited cities, select the one with minimum travelling cost from the

current city.

3. Move to the selected city, mark it as visited.
4. Repeat until all cities have been visited.
5. Return to City 0.

Time Complexity: O N the nearest unvisited one.

4.2 Simulated Annealing

Simulated Annealing is inspired by the physical process of annealing in metallurgy. A solution is iteratively improved by:

1. Initial Solution: Either a random permutation or the greedy tour.
2. Neighbor Generation: N, reverse the segment 2  At each of the

Using 2-opt segment reversal: pick positions i, j

3. Acceptance Rule:

If 0 (improvement): always accept.

If 0 (worsening): accept with probability

4. Cooling Schedule: Geometric cooling Tk+1
5. Termination: When T < T or maximum iterations reached.min

Time Complexity: O I N where I is the total number of iterations. Each iteration generates a neighbor in O N (segment reversal) and recalculates the tour cost in 4.3 Key Implementation Details

The codebase is organized into the following modular components: 3

N steps, we scan all N

/T

P e

α Tk

cities to nd 1 i < j <

O N


---

AI Assignment 4

Table 1: Implementation Functions

Function

readInput()  calculateTourCost()

greedyTSP()  generateRandomTour()  generateNeighbor()

simulatedAnnealing()

printResults()  isValidTour()  generateEuclideanInstance() 5 Experimental Setup

Five TSP instances of increasing size were generated using random Euclidean coordinates:

Sabbir Ahmmed

Purpose

Read cost matrix from le Compute total tour cost including return edge Nearest Neighbor heuristic Random permutation with City 0 xed 2-opt segment reversal Full SA algorithm with metrics Formatted output Tour validation Random Euclidean cost matrix generator

Default SA parameters: = 100, maximum iterations = 100,000.

6 Experiment 1: Greedy vs. Simulated Annealing

For each instance, the greedy algorithm was run once, and SA was run 5 times with dierent seeds. Results:

Instance Cities

| Test1_10 | 10 | 2462 | 2376 | 2376 | 3.49% | 0.0098 |
|---|---|---|---|---|---|---|
| Test2_20 | 20 | 4916 | 4170 | 4176 | 15.17% | 0.0123 |
| Test3_50 | 50 | 7387 | 5887 | 5985 | 20.31% | 0.0235 |
| Test4_100 | 100 | 10379 | 8315 | 8556 | 19.89% | 0.0342 |
| Test5_200 | 200 | 13521 | 13349 | 13624 | 1.27% | 0.0592 |

Table 2: Test Instances

Instance Cities

| Test1_10 | 10 | Euclidean | 1244 |
|---|---|---|---|
| Test2_20 | 20 | Euclidean | 1254 |
| Test3_50 | 50 | Euclidean | 1284 |
| Test4_100 | 100 | Euclidean | 1334 |
| Test5_200 | 200 | Euclidean | 1434 |

T = 1000 α = 0 995 0

Type

T = 0 001

min

Seed

, iterations per temperature

Greedy

Table 3: Experiment 1 Results

SA Best SA Avg 4

Improve% Time(s)


---

AI Assignment 4 104

Greedy SA Best 379 10

1

315 8

387

7

Cost 887 5

916 4 170

Tour0 5 4 462 376 2 2

0 10.20

50

Figure 1: Greedy vs. SA Best Cost across instances 20 31 20 (%)

15 17

100 Number of Cities

19 89

10

3 49

Improvement 0 10.20

50

100

Number of Cities

Figure 2: SA improvement over Greedy (%)

Discussion: SA consistently outperforms the greedy solution, with improvements rang-ing from 1.27% to 20.31%. The largest improvement is observed at N = 50 (20.31%). For the 200-city instance, the improvement is modest (1.27%), likely because the xed iteration budget (100,000) becomes insucient for larger instances.  rithm runs in microseconds while SA takes tens of milliseconds, but the solution quality improvement justies the extra computation.

7 Experiment 2: Varying Initial Temperature (

Using the 50-city instance with T ∈ {100 1000 5000 0

5

Sabbir Ahmmed 521 349 13 13

200

1 27

200

The greedy algo-

T0


---

AI Assignment 4

T Best Cost 0

| 100 | 5977 | 3565 | 647 | 0.0205 |
|---|---|---|---|---|
| 1000 | 5940 | 25976 | 11911 | 0.0197 |
| 5000 | 6508 | 54705 | 26249 | 0.0201 |

6 600

6 400 Cost

Table 4: Experiment 2 Results

Accepted Moves Worse Accepted

Sabbir Ahmmed

Time(s) 6 508

6 200 Best 5 977

5 940

6 000

5 800 102

103

103 7

Initial Temperature (T0

Figure 3: Best cost vs. initial temperature

Discussion:

Low T (100): Accepts only 647 worse moves. The algorithm behaves like a greedy

0 local search  converges quickly but risks getting trapped in a local optimum (cost = 5977).

Moderate T (1000): Accepts 11,911 worse moves, achieving the best cost of

0 5940. This value balances exploration and exploitation within the iteration budget.

High T (5000): Accepts 26,249 worse moves  too much exploration early on

0 wastes iterations before the temperature drops enough for the search to converge. The nal cost (6508) is worse than even the low-T run. 0

8 Experiment 3: Varying Cooling Rate (α

Using the 50-city instance with α ∈ {0 90 0 95 0 995

Table 5: Experiment 3 Results

α Best Cost Temp Steps to T Time(s)

min

| 0.900 | 6104 | 132 | 0.0026 |
|---|---|---|---|
| 0.950 | 6117 | 270 | 0.0054 |
| 0.995 | 5940 | 2757 | 0.0200 |

6


---

AI Assignment 4 6 200 6 104

Cost

Sabbir Ahmmed 6 117

6 000

5 940

Best 5 800 0 9

0 95

1 Cooling Rate (α

Figure 4: Best cost vs. cooling rate

Discussion:

Fast cooling (α = 0 90 Reaches T in only 132 temperature steps. The searchmin spends very little time at each temperature and freezes quickly into a local optimum (cost = 6104). Computation is cheap (0.0026s).

Moderate cooling (α = 0 95 Takes 270 steps. Slightly worse than α = 0 90 in this run (cost = 6117), likely due to random variation.

Slow cooling (α = 0 995 Takes 2,757 steps  far more temperature levels. This gives the algorithm much more time to explore the solution space and escape local optima, achieving the best cost of 5940. The computational cost (0.02s) is still very manageable.

9 Experiment 4: Random vs. Greedy Initialization

Using the 50-city instance, SA was run 5 times with each initialization method:

Table 6: Experiment 4 Results

Init Method Best Avg Worst Avg Time(s)

Random 5917 5974 6023

0.0196 Greedy

5981 6041 6207

0.0197

7


---

AI Assignment 4

Random Init 6 200 Greedy Init 041 Cost

6

Sabbir Ahmmed 207 6

023

981 5

6 000 917

Tour 5 6 974 5

5 800 Best

Average

Worst

Figure 5: Random vs. Greedy initialization comparison

Discussion: In these runs, random initialization achieved better results than greedy initialization across all metrics (best, average, worst). This is because:

Random initialization provides diverse starting points, giving SA a better chance of exploring dierent basins of attraction in the solution space.

Greedy initialization starts SA in a nearby region of the search space. With a limited iteration budget, SA may not have enough iterations to move far from the greedy starting point.

For larger iteration budgets, greedy initialization would likely outperform random initialization because it starts closer to a good solution.

This result highlights an important practical consideration: the eectiveness of the initialization strategy depends on the interaction between the starting solution quality and the available computational budget.

10 Complexity Analysis

10.1 Greedy Technique

The Nearest Neighbor algorithm has time complexity O(N2

 At each of the N steps, the algorithm scans all N cities to nd the nearest unvisited one.

The tour cost calculation is O N

Overall: O N2) for the tour construction, plus O(N) for cost calculation.

Space complexity: O N2 for the cost matrix.

10.2 Simulated Annealing

Let I be the total number of iterations:

Neighbor generation (2-opt reversal): O N per iteration.

8


---

AI Assignment 4

Tour cost recalculation: O

Per-iteration cost: O N

Total time: O I N

Potential Optimization: Instead of recalculating the entire tour cost after each 2-opt reversal, we can compute only the delta in

∆ = cost i 1 j) + cost

This would reduce the per-iteration cost from ity of O I for the inner loop. However, the current implementation uses full recalculation for simplicity and correctness. Space complexity: O N2 for the cost matrix.

11 Comparative Summary

Table 7: Algorithm Comparison

Aspect

Time Complexity  Solution Quality

Local optimum  Determinism  Escapes Local Optima  Parameters  Typical Improvement

N per iteration (full recalculation).

O(1)

i, j + 1) cost

O N to

Greedy

O N2

Deterministic

No  None 12 Conclusion

This assignment demonstrates the trade-os between a deterministic greedy heuristic and a probabilistic metaheuristic for the TSP:

1. The Greedy algorithm provides a fast O N2 optima. It is useful as a baseline or as an initialization for more advanced methods.
2. Simulated Annealing consistently nds better tours (320% improvement) at the cost of more computation. Its ability to accept worse moves allows it to escape local optima.
3. Parameter sensitivity is critical: moderate initial temperature ( slow cooling (α = 0 995) yield the best results for the tested instances.
4. Initialization strategy interacts with the iteration budget: random initialization can outperform greedy initialization when the budget is limited, as it provides more diverse exploration.

9

Sabbir Ahmmed

i 1 i cost j, j + 1)

O(1), yielding an overall complex-

Simulated Annealing

O I N Near-global optimum Probabilistic Yes T α T , iterations 0 min 320% over Greedy

solution but gets trapped in local

T = 1000) and 0


---

AI Assignment 4

Sabbir Ahmmed 13 Submission Contents

tsp2.cpp  Complete C++ source code

report.tex  This LaTeX report

report.txt  Raw experimental output

Test1_10.txt through Test5_200.txt  Generated test instances 10
