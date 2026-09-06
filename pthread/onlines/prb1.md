Problem 1: Alternating Fixed Sequence (Ping-Pong Control)

Problem Statement
Run three threads.

    Thread 1 prints letter “X” one letter at a time.

    Thread 2 prints letter “Y” one letter at a time.

    Thread 3 prints a dash “-” when a full pair is completed.

For N iterations, every line must follow the exact strict sequence: X - Y - X - Y. Thread 3 prints a newline after two pairs (total 4 letters) are completed. Implement using semaphores only without busy-waiting.

    Input format: <Number N iterations of>

    Sample Input: 2

    Sample Output:
    Plaintext

    X-Y-X-Y- [Line 1]
    X-Y-X-Y- [Line 2]

Solution
C++
``` cpp
#include <iostream>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

sem_t semX, semY, semDash, semLine;
int N;

void* funcX(void* arg) {
    for (int i = 0; i < N * 2; i++) {
        sem_wait(&semX);
        cout << "X";
        sem_post(&semDash);
    }
    return NULL;
}

void* funcY(void* arg) {
    for (int i = 0; i < N * 2; i++) {
        sem_wait(&semY);
        cout << "Y";
        sem_post(&semDash);
    }
    return NULL;
}

void* funcDash(void* arg) {
    for (int i = 0; i < N * 2; i++) {
        sem_wait(&semDash);
        cout << "-";
        
        // Hand off to Y after first X-, or trigger next line after second Y-
        if (i % 2 == 0) {
            sem_post(&semY);
        } else {
            sem_post(&semLine);
        }
    }
    return NULL;
}

void* funcLine(void* arg) {
    for (int i = 1; i <= N; i++) {
        sem_wait(&semLine);
        cout << " [Line " << i << "]" << endl;
        sem_post(&semX); // Start next line with X
    }
    return NULL;
}

int main() {
    cin >> N;

    // Initialize semaphores: X starts first
    sem_init(&semX, 0, 1);
    sem_init(&semY, 0, 0);
    sem_init(&semDash, 0, 0);
    sem_init(&semLine, 0, 0);

    pthread_t t1, t2, t3, t4;
    pthread_create(&t1, NULL, funcX, NULL);
    pthread_create(&t2, NULL, funcY, NULL);
    pthread_create(&t3, NULL, funcDash, NULL);
    pthread_create(&t4, NULL, funcLine, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);

    sem_destroy(&semX);
    sem_destroy(&semY);
    sem_destroy(&semDash);
    sem_destroy(&semLine);
    return 0;
}
```
Problem 2: Strict Majority Threshold Ratio

Problem Statement
Run three threads.

    Thread 1 prints 'A' in an infinite loop.

    Thread 2 prints 'B' in an infinite loop.

    Thread 3 prints 'C' in an infinite loop.

Threads run independently. A line finishes and prints [Iteration X] when the number of As printed on that line is at least double the sum of Bs and Cs combined, AND at least one B or C has been printed (Condition: countA≥2×(countB+countC) and (countB+countC)≥1). Run for N iterations using one binary semaphore lock.

    Input format: <Number N iterations of>

    Sample Input: 2

    Sample Output:
    Plaintext

    B A A A A [Iteration 1]
    C A A B A A [Iteration 2]

Solution
C++
``` cpp
#include <iostream>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

sem_t lock;
int N;
int current_iter = 1;
int countA = 0, countB = 0, countC = 0;

void* printThread(void* arg) {
    char letter = *(char*)arg;
    while (current_iter <= N) {
        sem_wait(&lock);
        if (current_iter > N) {
            sem_post(&lock);
            break;
        }

        cout << letter << " ";
        if (letter == 'A') countA++;
        else if (letter == 'B') countB++;
        else if (letter == 'C') countC++;

        int nonA = countB + countC;
        if (nonA >= 1 && countA >= 2 * nonA) {
            cout << "[Iteration " << current_iter << "]" << endl;
            countA = 0; countB = 0; countC = 0;
            current_iter++;
        }
        sem_post(&lock);
    }
    return NULL;
}

int main() {
    cin >> N;
    sem_init(&lock, 0, 1);

    pthread_t t1, t2, t3;
    char a = 'A', b = 'B', c = 'C';

    pthread_create(&t1, NULL, printThread, &a);
    pthread_create(&t2, NULL, printThread, &b);
    pthread_create(&t3, NULL, printThread, &c);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    sem_destroy(&lock);
    return 0;
}
```
Problem 3: Arithmetic Growth Per Line

Problem Statement
Run two threads.

    Thread 1 prints '0' in an infinite loop.

    Thread 2 prints '1' in an infinite loop.

For N total lines, line k (starting at line 1) must contain exactly k zeros and k ones in any order. When line k reaches k zeros and k ones, a line ending flag # [Line k] is printed, and the process advances to line k+1. Implement using a single semaphore lock.

    Input format: <Number N lines of>

    Sample Input: 3

    Sample Output:
    Plaintext

    01# [Line 1]
    1001# [Line 2]
    011001# [Line 3]

Solution
C++
``` cpp
#include <iostream>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

sem_t lock;
int N;
int current_line = 1;
int count0 = 0, count1 = 0;

void* printBinary(void* arg) {
    char digit = *(char*)arg;
    while (current_line <= N) {
        sem_wait(&lock);
        if (current_line > N) {
            sem_post(&lock);
            break;
        }

        // Only print if this digit hasn't hit its target count for this line
        bool printed = false;
        if (digit == '0' && count0 < current_line) {
            cout << '0';
            count0++;
            printed = true;
        } else if (digit == '1' && count1 < current_line) {
            cout << '1';
            count1++;
            printed = true;
        }

        if (count0 == current_line && count1 == current_line) {
            cout << "# [Line " << current_line << "]" << endl;
            count0 = 0;
            count1 = 0;
            current_line++;
        }
        sem_post(&lock);
    }
    return NULL;
}

int main() {
    cin >> N;
    sem_init(&lock, 0, 1);

    pthread_t t1, t2;
    char d0 = '0', d1 = '1';

    pthread_create(&t1, NULL, printBinary, &d0);
    pthread_create(&t2, NULL, printBinary, &d1);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    sem_destroy(&lock);
    return 0;
}
```
Problem 4: Barrier Synchronization (Equal Batch Printing)

Problem Statement
Run three worker threads (Thread 1 prints 'D', Thread 2 prints 'E', Thread 3 prints 'F') and one controller thread.
In each iteration, each worker thread must print its character exactly twice in any order (total 6 characters per line). No worker thread can start printing for iteration k+1 until the controller thread prints | [Batch k] for iteration k. Run for N iterations. Avoid busy-waiting using counting semaphores.

    Input format: <Iterations N>

    Sample Input: 2

    Sample Output:
    Plaintext

    DEDEFF| [Batch 1]
    FEDFED| [Batch 2]

Solution
C++
``` cpp
#include <iostream>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

sem_t semD, semE, semF, semBatch;
int N;

void* funcD(void* arg) {
    for (int i = 0; i < N * 2; i++) {
        sem_wait(&semD);
        cout << "D";
        sem_post(&semBatch);
    }
    return NULL;
}

void* funcE(void* arg) {
    for (int i = 0; i < N * 2; i++) {
        sem_wait(&semE);
        cout << "E";
        sem_post(&semBatch);
    }
    return NULL;
}

void* funcF(void* arg) {
    for (int i = 0; i < N * 2; i++) {
        sem_wait(&semF);
        cout << "F";
        sem_post(&semBatch);
    }
    return NULL;
}

void* controller(void* arg) {
    for (int i = 1; i <= N; i++) {
        // Wait until 6 characters total are printed (2 D's, 2 E's, 2 F's)
        for (int j = 0; j < 6; j++) {
            sem_wait(&semBatch);
        }
        cout << "| [Batch " << i << "]" << endl;

        // Reload tokens for next batch
        for (int j = 0; j < 2; j++) sem_post(&semD);
        for (int j = 0; j < 2; j++) sem_post(&semE);
        for (int j = 0; j < 2; j++) sem_post(&semF);
    }
    return NULL;
}

int main() {
    cin >> N;

    // Initialize each character with 2 tokens
    sem_init(&semD, 0, 2);
    sem_init(&semE, 0, 2);
    sem_init(&semF, 0, 2);
    sem_init(&semBatch, 0, 0);

    pthread_t t1, t2, t3, t4;
    pthread_create(&t1, NULL, funcD, NULL);
    pthread_create(&t2, NULL, funcE, NULL);
    pthread_create(&t3, NULL, funcF, NULL);
    pthread_create(&t4, NULL, controller, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);

    sem_destroy(&semD);
    sem_destroy(&semE);
    sem_destroy(&semF);
    sem_destroy(&semBatch);
    return 0;
}
```
Problem 5: Odd-Even Character Length Toggle

Problem Statement
Run three threads printing 'M', 'N', and 'O' in an infinite loop.
At iteration i (for i=1…N):

    If i is odd, print L1​ total letters per line followed by *.

    If i is even, print L2​ total letters per line followed by #.

Implement using one binary semaphore lock.

    Input format: <N iterations> <L1 length> <L2 length>

    Sample Input: 3 4 6

    Sample Output:
    Plaintext

    MNOM* [Iteration 1]
    ONMMNN# [Iteration 2]
    NMON* [Iteration 3]

Solution
C++
``` cpp
#include <iostream>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

sem_t lock;
int N, L1, L2;
int current_iter = 1;
int letter_count = 0;

void* printMNO(void* arg) {
    char letter = *(char*)arg;
    while (current_iter <= N) {
        sem_wait(&lock);
        if (current_iter > N) {
            sem_post(&lock);
            break;
        }

        cout << letter;
        letter_count++;

        int target_length = (current_iter % 2 != 0) ? L1 : L2;

        if (letter_count == target_length) {
            char mark = (current_iter % 2 != 0) ? '*' : '#';
            cout << mark << " [Iteration " << current_iter << "]" << endl;
            letter_count = 0;
            current_iter++;
        }
        sem_post(&lock);
    }
    return NULL;
}

int main() {
    cin >> N >> L1 >> L2;
    sem_init(&lock, 0, 1);

    pthread_t t1, t2, t3;
    char m = 'M', n = 'N', o = 'O';

    pthread_create(&t1, NULL, printMNO, &m);
    pthread_create(&t2, NULL, printMNO, &n);
    pthread_create(&t3, NULL, printMNO, &o);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    sem_destroy(&lock);
    return 0;
}

How would you like to review these exam problems?