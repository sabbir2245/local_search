import heapq
import numpy as np 
import random
from heruistics import ultra_heuristic

class Node:
    def __init__(self, state, p=None, g=0, h=0):
        self.state = state
        self.p = p
        self.g = g
        self.f = g + h

    def __lt__(self, other):
        return self.f < other.f

def AstarLowlevel(start, goal, get_neighbors, heuristic, weight=1.0):
    open_list = [Node(start, None, 0, heuristic(start, goal, weight))]
    closed = set()
    best_g = {start: 0}
    nodes_explored = 0

    while open_list:
        curr = heapq.heappop(open_list)
        nodes_explored += 1

        if curr.state == goal:
            path = []
            while curr:
                path.append(curr.state)
                curr = curr.p
            return path[::-1], nodes_explored

        if curr.state in closed:
            continue
        closed.add(curr.state)

        for nxt, cost in get_neighbors(curr.state):
            g_score = curr.g + cost

            if nxt in best_g and g_score >= best_g[nxt]:
                continue

            best_g[nxt] = g_score
            heapq.heappush(open_list, Node(nxt, curr, g_score, heuristic(nxt, goal, weight)))

    return None, nodes_explored

def print_grid(arr):
    n = len(arr)
    for i in range(n):
        print("|", end="")
        for j in range(n):
            if arr[i][j] == 0:
                print("  |", end="")
            else:
                print(f"{arr[i][j]:2d}|", end="")
        print()

def get_unique_grid(n):
    numbers = random.sample(range(n * n), n * n)
    grid = tuple(tuple(numbers[i*n:(i+1)*n]) for i in range(n))
    return grid

def get_inversions(arr, k):
    flat_arr = [x for row in arr for x in row if x != 0]
    
    def merge_sort_and_count(arr_1d):
        if len(arr_1d) <= 1:
            return arr_1d, 0
        
        mid = len(arr_1d) // 2
        left, left_count = merge_sort_and_count(arr_1d[:mid])
        right, right_count = merge_sort_and_count(arr_1d[mid:])
        
        merged, merge_count = merge_and_count(left, right)
        
        return merged, left_count + right_count + merge_count

    def merge_and_count(left, right):
        merged = []
        i = j = count = 0
        
        while i < len(left) and j < len(right):
            if left[i] <= right[j]:
                merged.append(left[i])
                i += 1
            else:
                merged.append(right[j])
                count += (len(left) - i)
                j += 1
                
        merged.extend(left[i:])
        merged.extend(right[j:])
        
        return merged, count

    _, total_inversions = merge_sort_and_count(flat_arr)
    return total_inversions

def get_neighbors(state):
    n = len(state)
    neighbors = []
    zero_pos = [(i, j) for i in range(n) for j in range(n) if state[i][j] == 0][0]
    x, y = zero_pos

    directions = [(-1, 0), (1, 0), (0, -1), (0, 1)] 

    for dx, dy in directions:
        new_x, new_y = x + dx, y + dy
        if 0 <= new_x < n and 0 <= new_y < n:
            temp = [list(row) for row in state]
            temp[x][y], temp[new_x][new_y] = temp[new_x][new_y], temp[x][y] 
            temp_state = tuple(tuple(row) for row in temp)
            neighbors.append((temp_state, 1))
    return neighbors

def find_the_star(arr, k, weight=1.0):
    if not isSolvable(arr, k):
        return None, 0
    
    start = arr 
    
    goal_list = []
    val = 1
    for i in range(k):
        row = []
        for j in range(k):
            if i == k - 1 and j == k - 1:
                row.append(0)
            else:
                row.append(val)
                val += 1
        goal_list.append(tuple(row))
    goal = tuple(goal_list)
    
    path, nodes_explored = AstarLowlevel(start, goal, get_neighbors, heuristic, weight)
    return path, nodes_explored

def heuristic(state, goal, weight=1.0):
    return ultra_heuristic(state, goal, weight)



def isSolvable(arr, k):
    numof_inversions = get_inversions(arr, k)
    if k % 2 == 1:
        return numof_inversions % 2 == 0
    else:
        blank_row_idx = -1
        for i in range(k):
            if 0 in arr[i]:
                blank_row_idx = i
                break
        row_from_bottom = k - blank_row_idx
        if row_from_bottom % 2 == 1:
            return numof_inversions % 2 == 0
        else:
            return numof_inversions % 2 == 1
        
def generateSolvable(filename, k):
    goal = []
    val = 1
    for i in range(k):
        row = []
        for j in range(k):
            if i == k - 1 and j == k - 1:
                row.append(0)
            else:
                row.append(val)
                val += 1
        goal.append(list(row))

    r, c = k - 1, k - 1
    for _ in range(150):
        moves = []
        if r > 0: moves.append((-1, 0))
        if r < k - 1: moves.append((1, 0))
        if c > 0: moves.append((0, -1))
        if c < k - 1: moves.append((0, 1))
        dr, dc = random.choice(moves)
        nr, nc = r + dr, c + dc
        goal[r][c], goal[nr][nc] = goal[nr][nc], goal[r][c]
        r, c = nr, nc

    with open(filename, 'w') as f:
        f.write(f"1\n{k}\n")
        for row in goal:
            f.write(" ".join(str(x) for x in row) + "\n")

def process_file(input_filename, output_filename, weight=1.0):
    with open(input_filename, 'r') as infile, open(output_filename, 'w') as outfile:
        lines = infile.read().splitlines()
        lines = [line.strip() for line in lines if line.strip()]
        
        if not lines:
            return
        
        num_tests = int(lines[0])
        idx = 1
        
        for t in range(num_tests):
            if idx >= len(lines):
                break
                
            k = int(lines[idx])
            idx += 1
            
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
            result, nodes_explored = find_the_star(start_grid, k, weight)
            
            if result is None:
                outfile.write("Unsolvable puzzle\n")
            else:
                moves = len(result) - 1
                outfile.write(f"Minimum number of moves = {moves}\n")
                outfile.write(f"Nodes explored = {nodes_explored}\n")
                for state in result:
                    for row in state:
                        outfile.write(" ".join(f"{x} " for x in row) + "\n")
                    outfile.write("\n") 
    
    print("finished")
            

import json
import re

def extract_metrics(output_filename):
    metrics = []
    with open(output_filename, 'r') as f:
        content = f.read()
    blocks = content.strip().split("Minimum number of moves = ")
    for block in blocks[1:]:
        lines = block.split("\n")
        cost_line = lines[0].strip()
        cost = int(cost_line)
        nodes_line = lines[1].strip() if len(lines) > 1 else ""
        nodes_match = re.search(r"Nodes explored = (\d+)", nodes_line)
        nodes = int(nodes_match.group(1)) if nodes_match else 0
        metrics.append({"cost": cost, "nodes_explored": nodes})
    return metrics

def run_all_weights():
    weights = [1.0, 1.2, 2.0, 5.0]
    all_data = []

    for w in weights:
        out_file = f"output_w{w}.txt"
        process_file("binput.txt", out_file, w)
        metrics = extract_metrics(out_file)
        for i, m in enumerate(metrics):
            if len(all_data) <= i:
                all_data.append({"test_case": i, "results": []})
            all_data[i]["results"].append({
                "W": w,
                "nodes_explored": m["nodes_explored"],
                "cost": m["cost"]
            })

    with open("raw_data.json", "w") as f:
        json.dump(all_data, f, indent=2)
    print("raw_data.json generated")

if __name__ == "__main__":
    import sys
    if len(sys.argv) > 1 and sys.argv[1] == "--all":
        run_all_weights()
    else:
        weight = float(sys.argv[1]) if len(sys.argv) > 1 else 1.0
        process_file("binput.txt", "output.txt", weight)