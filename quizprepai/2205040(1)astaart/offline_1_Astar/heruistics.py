import math

def get_goal_positions(goal):
    n = len(goal)
    goal_pos = {}
    for r in range(n):
        for c in range(n):
            if goal[r][c] != 0:
                goal_pos[goal[r][c]] = (r, c)
    return goal_pos

def hamming_distance(state, goal):
    n = len(state)
    misplaced = 0
    for r in range(n):
        for c in range(n):
            val = state[r][c]
            if val != 0 and val != goal[r][c]:
                misplaced += 1
    return misplaced

def manhattan_distance(state, goal_pos):
    n = len(state)
    distance = 0
    for r in range(n):
        for c in range(n):
            val = state[r][c]
            if val != 0:
                g_r, g_c = goal_pos[val]
                distance += abs(r - g_r) + abs(c - g_c)
    return distance

def euclidean_distance(state, goal_pos):
    n = len(state)
    distance = 0.0
    for r in range(n):
        for c in range(n):
            val = state[r][c]
            if val != 0:
                g_r, g_c = goal_pos[val]
                distance += math.isqrt((r - g_r) ** 2 + (c - g_c) ** 2)
    return int(distance)

def linear_conflict_penalty(state, goal_pos):
    n = len(state)
    penalty = 0

    for r in range(n):
        for c1 in range(n):
            val1 = state[r][c1]
            if val1 == 0:
                continue
            g_r1, g_c1 = goal_pos[val1]
            if g_r1 != r:
                continue

            for c2 in range(c1 + 1, n):
                val2 = state[r][c2]
                if val2 == 0:
                    continue
                g_r2, g_c2 = goal_pos[val2]
                if g_r2 != r:
                    continue

                if g_c1 > g_c2:
                    penalty += 2

    for c in range(n):
        for r1 in range(n):
            val1 = state[r1][c]
            if val1 == 0:
                continue
            g_r1, g_c1 = goal_pos[val1]
            if g_c1 != c:
                continue

            for r2 in range(r1 + 1, n):
                val2 = state[r2][c]
                if val2 == 0:
                    continue
                g_r2, g_c2 = goal_pos[val2]
                if g_c2 != c:
                    continue

                if g_r1 > g_r2:
                    penalty += 2

    return penalty

def ultra_heuristic(state, goal, weight=1.0):
    goal_pos = get_goal_positions(goal)

    h_manhattan = manhattan_distance(state, goal_pos)
    h_linear = linear_conflict_penalty(state, goal_pos)

    val = h_manhattan + h_linear
    return weight * val 