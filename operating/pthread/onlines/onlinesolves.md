These Interprocess Communication (IPC) problems from January 2026 involve multi-threading and synchronization using semaphores in C++.

### Section: B2 - Incremental Iteration Counts

**Problem Statement**
Run three threads.
a. Thread1 prints ‘P’ in an infinite loop.
b. Thread2 prints ‘Q’ in an infinite loop.
c. Thread3 prints ‘R’ in an infinite loop.

Now, these three threads should run independently and should print only one letter
at a time. At each iteration, ‘P’, ‘Q’, and ‘R’—all three letters—must be printed in
random order. Also, the number of each letter to be printed at each iteration equal
s the iteration count. Implement this using semaphores only. Take iteration count as input.

  * **Input format:** `<Number of iterations>`
  * **Sample Output:**
      * PQR \[iteration 1\]
      * QQPRRP \[iteration 2\]
      * PQPQRQRRP \[iteration 3\]
  * **Note:** The letters can be printed in any order. Create a file `<student_id>.c
  * pp` and submit it.

**Solution**

``` cpp
#include <iostream>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

sem_t mutex;
int N;
int cur_iter = 1;
int p_count = 0, q_count = 0, r_count = 0;

void* printPQR(void* arg) {
    char letter = *(char*)arg;
    while (cur_iter <= N) {
        sem_wait(&mutex);
        if (cur_iter > N) {
            sem_post(&mutex);
            break;
        }

        bool can_print = false;
        if (letter == 'P' && p_count < cur_iter) { can_print = true; p_count++; }
        else if (letter == 'Q' && q_count < cur_iter) { can_print = true; q_count++; }
        else if (letter == 'R' && r_count < cur_iter) { can_print = true; r_count++; }

        if (can_print) cout << letter;

        if (p_count == cur_iter && q_count == cur_iter && r_count == cur_iter) {
            cout << " [iteration " << cur_iter << "]" << endl;
            p_count = 0; q_count = 0; r_count = 0;
            cur_iter++;
        }
        sem_post(&mutex);
    }
    return NULL;
}

int main() {
    cin >> N;
    sem_init(&mutex, 0, 1);
    pthread_t t1, t2, t3;
    char p = 'P', q = 'Q', r = 'R';
    pthread_create(&t1, NULL, printPQR, &p);
    pthread_create(&t2, NULL, printPQR, &q);
    pthread_create(&t3, NULL, printPQR, &r);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    sem_destroy(&mutex);
    return 0;
}

```

-----

### Section: C1 - Fixed Character Counts

**Problem Statement**
Run four threads.
a. Thread 1 prints letter “A” one letter at a time.
b. Thread 2 prints letter “B” one letter at a time.
c. Thread 3 prints letter “C” one letter at a time.
d. Thread 4 prints a newline when a certain condition is met.

Each line contains three “A”s, four “B”s, and five “C”s. The letters can appear in a
ny order. Thread 4 prints a newline when the total count of letters in a line equals 
twelve. There will be seven lines printed in the output. Implement using semaphores only
, avoiding busy waiting. For the count of A, B, Cs and the number of iterations, you mus
t use counting semaphores. (Hint: You will need four counting semaphores.)

  * **Sample Output:**
      * AAABBBBCCCCC
      * ABCBABCBCACC
      * BACABCBACBCC
  * **Note:** Create a file `<student_id>.cpp` and submit it.

**Solution**

``` cpp
#include <iostream>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

sem_t semA, semB, semC, semLine;
int total_lines = 7;

void* funcA(void* arg) {
    for (int i = 0; i < total_lines * 3; i++) {
        sem_wait(&semA);
        cout << "A";
        sem_post(&semLine);
    }
    return NULL;
}

void* funcB(void* arg) {
    for (int i = 0; i < total_lines * 4; i++) {
        sem_wait(&semB);
        cout << "B";
        sem_post(&semLine);
    }
    return NULL;
}

void* funcC(void* arg) {
    for (int i = 0; i < total_lines * 5; i++) {
        sem_wait(&semC);
        cout << "C";
        sem_post(&semLine);
    }
    return NULL;
}

void* funcLine(void* arg) {
    for (int i = 0; i < total_lines; i++) {
        for (int j = 0; j < 12; j++) {
            sem_wait(&semLine);
        }
        cout << endl;
        for (int j = 0; j < 3; j++) sem_post(&semA);
        for (int j = 0; j < 4; j++) sem_post(&semB);
        for (int j = 0; j < 5; j++) sem_post(&semC);
    }
    return NULL;
}

int main() {
    sem_init(&semA, 0, 3);
    sem_init(&semB, 0, 4);
    sem_init(&semC, 0, 5);
    sem_init(&semLine, 0, 0);

    pthread_t t1, t2, t3, t4;
    pthread_create(&t1, NULL, funcA, NULL);
    pthread_create(&t2, NULL, funcB, NULL);
    pthread_create(&t3, NULL, funcC, NULL);
    pthread_create(&t4, NULL, funcLine, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);

    return 0;
}

```

-----

### Section: C2 - Independent Thread Printing

**Problem Statement**
Run three threads.
a. Thread1 prints ‘A’ in an infinite loop.
b. Thread2 prints ‘B’ in an infinite loop.
c. Thread3 prints ‘C’ in an infinite loop.

Now, these three threads should run independently. At each iteration, $L$ letters
are printed in a line followed by a $. There are $N$ iterations. Implement this using 
one semaphore only.

  * **Input format:** `<iteration count> <number of letters per line>`
  * **Sample Input:** `3 10`
  * **Sample Output:**
      * ABCBCBCBAC$ \[iteration 1\]
      * CBAABCACBB$ \[iteration 2\]
      * BACBCABABA$ \[iteration 3\]
  * **Note:** The letters can appear in any sequence. Create a file `<student_id>.cpp` and submit it.

**Solution**

``` cpp
#include <iostream>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

sem_t mutex;
int N, L;
int current_count = 0;
int current_iteration = 1;

void* printLetter(void* arg) {
    char letter = *(char*)arg;
    while (current_iteration <= N) {
        sem_wait(&mutex);
        if (current_iteration > N) {
            sem_post(&mutex);
            break;
        }
        
        cout << letter;
        current_count++;
        
        if (current_count == L) {
            cout << "$ [iteration " << current_iteration << "]" << endl;
            current_count = 0;
            current_iteration++;
        }
        sem_post(&mutex);
    }
    return NULL;
}

int main() {
    cin >> N >> L;
    sem_init(&mutex, 0, 1);
    
    pthread_t t1, t2, t3;
    char a = 'A', b = 'B', c = 'C';
    
    pthread_create(&t1, NULL, printLetter, &a);
    pthread_create(&t2, NULL, printLetter, &b);
    pthread_create(&t3, NULL, printLetter, &c);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    
    sem_destroy(&mutex);
    return 0;
}

```

-----

### Section: A1 - Minimum Character Threshold

**Problem Statement**
Run three threads.
a. Thread1 prints ‘A’ in an infinite loop.
b. Thread2 prints ‘B’ in an infinite loop.
c. Thread3 prints ‘C’ in an infinite loop.

Now, these three threads should run independently. When at least one A, at least 
one B, and at least one C have been printed in a line, a new line is printed. Continue 
printing until $N$ lines have been printed. Implement this using semaphore only. (Hint
: Only one semaphore lock will suffice for this code.)

  * **Input format:** \`\<Number of iterations\>\`
  * **Sample Input:** `N = 3`
  * **Sample Output:**
      * BAC \[Line 1\]
      * BABBBAC \[Line 2\]
      * CCCACACACAAAAB \[Line 3\]
  * **Note:** The letters in a line can appear in any order. Create a file `<student_id>
  * .cpp` and submit it.

**Solution**

``` cpp
#include <iostream>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

sem_t lock;
int N;
int lines_done = 0;
int countA = 0, countB = 0, countC = 0;

void* printThread(void* arg) {
    char letter = *(char*)arg;
    while (lines_done < N) {
        sem_wait(&lock);
        if (lines_done >= N) {
            sem_post(&lock);
            break;
        }

        cout << letter;
        if (letter == 'A') countA++;
        else if (letter == 'B') countB++;
        else if (letter == 'C') countC++;

        if (countA >= 1 && countB >= 1 && countC >= 1) {
            lines_done++;
            cout << " [Line " << lines_done << "]" << endl;
            countA = 0; countB = 0; countC = 0;
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
Sort these probs by difficulty