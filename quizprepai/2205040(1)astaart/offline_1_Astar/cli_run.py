import sys
import importlib

up = importlib.import_module("2205040_up")
from heruistics import ultra_heuristic

def process_stdin(output_filename, weight=1.0):
    with open(output_filename, 'w') as outfile:
        lines = sys.stdin.read().splitlines()
        lines = [line.strip() for line in lines if line.strip()]
        idx = 0

        while idx < len(lines):
            k = int(lines[idx])
            idx += 1

            if idx + k > len(lines):
                break

            grid_rows = []
            for _ in range(k):
                row_str = lines[idx]
                idx += 1
                if ' ' in row_str:
                    row_data = [int(x) for x in row_str.split()]
                else:
                    row_data = [int(x) for x in row_str]
                grid_rows.append(tuple(row_data))

            start_grid = tuple(grid_rows)
            result, nodes_explored = up.find_the_star(start_grid, k, weight)

            if result is None:
                outfile.write("Unsolvable puzzle\n")
            else:
                moves = len(result) - 1
                outfile.write(f"Minimum number of moves = {moves}\n")
                for state in result:
                    for row in state:
                        outfile.write(" ".join(f"{x} " for x in row) + "\n")
                    outfile.write("\n")

if __name__ == "__main__":
    weight = float(sys.argv[1]) if len(sys.argv) > 1 else 1.0
    process_stdin("outputcli.txt", weight)
    print("finished")
