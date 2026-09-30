# 20 batch  IPC Online — Missing Problems (from Online 4 IPC zip)

Only the problems **not already covered** in `ipc-onlinesolves.md`,
`ipc-online-prb1.md`, or `ipc-online-prb2.md` are added here.
(Zip A2 ≈ onlinesolves A1-minimum-threshold; Zip B2 ≈ onlinesolves C2 —
both skipped.)

---

## Problem 1 (Zip A1): Growing p/q/r Per Iteration

**Problem Statement**
Run three threads.
a. Thread1 prints 'p' in an infinite loop
b. Thread2 prints 'q' in an infinite loop
c. Thread3 prints 'r' in an infinite loop

At each iteration k, 'p', 'q' and 'r' must all be printed before moving to
the next iteration, and the count of each letter grows with k:

```
pqr [iteration 1]
qqpprr [iteration 2]
pppqqqrrr [iteration 3]
...
```

Take iteration count as input. Implement using semaphore only —
one binary semaphore lock suffices.

**Input format:** `<Number of iterations>`
**Sample Input:** `3`
**Sample Output:**

```
pqr [iteration 1]
ppqqrr [iteration 2]
pppqqqrrr [iteration 3]
```

**Note:** Any order within a line earns 8/10; exact grouped order
(`p…q…r…`) earns full marks.

**Solution**

```cpp
#include <iostream>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

sem_t lock;
int N;
int cur_iter = 1;
int p_count = 0, q_count = 0, r_count = 0;

void* printThread(void* arg) {
    char letter = *(char*)arg;
    while (cur_iter <= N) {
        sem_wait(&lock);
        if (cur_iter > N) {
            sem_post(&lock);
            break;
        }

        bool printed = false;
        if (letter == 'p' && p_count < cur_iter) { cout << letter; p_count++; printed = true; }
        else if (letter == 'q' && q_count < cur_iter) { cout << letter; q_count++; printed = true; }
        else if (letter == 'r' && r_count < cur_iter) { cout << letter; r_count++; printed = true; }

        (void)printed;

        if (p_count == cur_iter && q_count == cur_iter && r_count == cur_iter) {
            cout << " [iteration " << cur_iter << "]" << endl;
            p_count = 0; q_count = 0; r_count = 0;
            cur_iter++;
        }
        sem_post(&lock);
    }
    return NULL;
}

int main() {
    cin >> N;
    sem_init(&lock, 0, 1);

    pthread_t t1, t2, t3;
    char p = 'p', q = 'q', r = 'r';
    pthread_create(&t1, NULL, printThread, &p);
    pthread_create(&t2, NULL, printThread, &q);
    pthread_create(&t3, NULL, printThread, &r);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    sem_destroy(&lock);
    return 0;
}
```

---

## Problem 2 (Zip B1): Underscore-Plus Triangle

**Problem Statement**
Run two threads.
a. Thread1 prints `_`
b. Thread2 prints `+`

Take input N. Print lines so that line k (k = 1 … N/2) contains
(N − k) underscores followed by k plus signs:

```
_________+
________++
_______+++
______++++
_____+++++
```

Here N = 10: line 1 prints 9 `_` and 1 `+`; last line prints 5 `_` and
5 `+`. Implement using semaphores only.

**Input format:** `<N>`
**Sample Input:** `10`
**Sample Output:**

```
_________+
________++
_______+++
______++++
_____+++++
```

**Solution**

```cpp
#include <iostream>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

sem_t lock;
int N;
int lines;          // N / 2 lines for even N
int cur_line = 1;   // 1-based
int u_count = 0, p_count = 0;

void* printUnderscore(void* arg) {
    while (cur_line <= lines) {
        sem_wait(&lock);
        if (cur_line > lines) { sem_post(&lock); break; }

        int u_target = N - cur_line;
        if (u_count < u_target) {
            cout << '_';
            u_count++;
        }
        if (u_count == u_target && p_count == cur_line) {
            cout << endl;
            u_count = 0; p_count = 0;
            cur_line++;
        }
        sem_post(&lock);
    }
    return NULL;
}

void* printPlus(void* arg) {
    while (cur_line <= lines) {
        sem_wait(&lock);
        if (cur_line > lines) { sem_post(&lock); break; }

        int u_target = N - cur_line;
        // Enforce order: '+' prints only after all '_' of this line
        if (u_count == u_target && p_count < cur_line) {
            cout << '+';
            p_count++;
        }
        if (u_count == u_target && p_count == cur_line) {
            cout << endl;
            u_count = 0; p_count = 0;
            cur_line++;
        }
        sem_post(&lock);
    }
    return NULL;
}

int main() {
    cin >> N;
    lines = N / 2;
    sem_init(&lock, 0, 1);

    pthread_t t1, t2;
    pthread_create(&t1, NULL, printUnderscore, NULL);
    pthread_create(&t2, NULL, printPlus, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    sem_destroy(&lock);
    return 0;
}
```
