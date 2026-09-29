import random

def get_goal_state(n):
    goal = []
    val = 1
    for i in range(n):
        row = []
        for j in range(n):
            if i == n - 1 and j == n - 1:
                row.append(0)
            else:
                row.append(val)
                val += 1
        goal.append(list(row))
    return goal

def find_blank(state, n):
    for r in range(n):
        for c in range(n):
            if state[r][c] == 0:
                return r, c

def generate_solvable_puzzle(n, steps=100):
    state = get_goal_state(n)
    r, c = n - 1, n - 1
    
    # Walk backward randomly to scramble the puzzle layout safely
    for _ in range(steps):
        moves = []
        if r > 0: moves.append((-1, 0))
        if r < n - 1: moves.append((1, 0))
        if c > 0: moves.append((0, -1))
        if c < n - 1: moves.append((0, 1))
        
        dr, dc = random.choice(moves)
        nr, nc = r + dr, c + dc
        
        state[r][c], state[nr][nc] = state[nr][nc], state[r][c]
        r, c = nr, nc
        
    return state

def create_puzzle_file(filename, num_puzzles=10, n=4):
    with open(filename, "w") as f:
        f.write(f"{num_puzzles}\n")
        
        for _ in range(num_puzzles):
            f.write(f"{n}\n")
            # Generate a distinct puzzle configuration using a healthy mix of scrambles
            puzzle = generate_solvable_puzzle(n, steps=150)
            for row in puzzle:
                f.write(" ".join(str(x) for x in row) + "\n")

if __name__ == "__main__":
    create_puzzle_file("binput.txt", num_puzzles=10, n=4)
    print("Successfully generated 10 solvable 4x4 puzzles inside 'binput.txt'!")