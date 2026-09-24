
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


hought: 2.2s
I couldn't read the PDF (model limitation), but the code comments explain everything. Here's how to run it:
Build
g++ -O2 -std=c++17 -o tsp tsp.cpp
Quick Start — Run Everything
./tsp all
This generates 5 test instances (10, 20, 50, 100, 200 cities) and runs all 4 experiments. Output goes to stdout — redirect to a file:
./tsp all > report_output.txt
Individual Commands
Command	Description
./tsp gen <N> <file>	Generate random N-city instance
./tsp solve <file>	Solve with Greedy + SA
./tsp exp1 <files...>	Experiment 1: Greedy vs SA across instances
./tsp exp2 <file>	Experiment 2: Vary initial temperature T0
./tsp exp3 <file>	Experiment 3: Vary cooling rate alpha
./tsp exp4 <file>	Experiment 4: Random vs Greedy initialization
Examples
# Generate instances
./tsp gen 50 instance_50.txt
./tsp gen 100 instance_100.txt

# Solve a single instance
./tsp solve instance_50.txt --t0 1000 --alpha 0.995 --init greedy

# Run specific experiments
./tsp exp1 instance_50.txt instance_100.txt instance_200.txt
./tsp exp2 instance_50.txt
./tsp exp3 instance_50.txt
./tsp exp4 instance_50.txt

# Save all experiment results for report
./tsp all > experiment_results.txt
▣  Build · MiMo V2.5 Free · 20.4s