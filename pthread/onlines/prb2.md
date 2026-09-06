Problem 1: Strict Phase-Ordered Pipeline ($T_1 \to T_2 \to T_3$)Problem StatementRun three threads.Thread 1 prints 'A'Thread 2 prints 'B'Thread 3 prints 'C'Threads must execute in strict round-robin order ($A \to B \to C \to A \to B \to C\dots$) for $N$ total cycles. No character may be printed out of turn. Implement without busy-waiting using counting/binary semaphores.Input format: <Number N cycles of>Sample Input: 2Sample Output: ABCABCSolutionC++#include <iostream>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

sem_t semA, semB, semC;
int N;

void* funcA(void* arg) {
    for (int i = 0; i < N; i++) {
        sem_wait(&semA);
        cout << "A";
        sem_post(&semB); // Hand off directly to B
    }
    return NULL;
}

void* funcB(void* arg) {
    for (int i = 0; i < N; i++) {
        sem_wait(&semB);
        cout << "B";
        sem_post(&semC); // Hand off directly to C
    }
    return NULL;
}

void* funcC(void* arg) {
    for (int i = 0; i < N; i++) {
        sem_wait(&semC);
        cout << "C";
        sem_post(&semA); // Hand off back to A
    }
    return NULL;
}

int main() {
    cin >> N;

    // A starts first; B and C start locked
    sem_init(&semA, 0, 1);
    sem_init(&semB, 0, 0);
    sem_init(&semC, 0, 0);

    pthread_t t1, t2, t3;
    pthread_create(&t1, NULL, funcA, NULL);
    pthread_create(&t2, NULL, funcB, NULL);
    pthread_create(&t3, NULL, funcC, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    sem_destroy(&semA);
    sem_destroy(&semB);
    sem_destroy(&semC);
    return 0;
}
Problem 2: Producer-Consumer with Bounded BufferProblem StatementRun 1 Producer thread and 1 Consumer thread sharing a queue of fixed capacity $K$.Producer generates integers $1, 2, 3 \dots N$ and puts them in the buffer.Consumer pops items from the buffer and prints them.The Producer must block if the buffer is full ($K$ items). The Consumer must block if the buffer is empty ($0$ items). Use empty, full, and mutex semaphores.Input format: <Total N items> <Buffer K capacity>Sample Input: 5 2Sample Output: Produced 1 | Consumed 1 | Produced 2 | Produced 3 | Consumed 2 | Consumed 3 | Produced 4 | Consumed 4 | Produced 5 | Consumed 5SolutionC++#include <iostream>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

int N, K;
int* buffer;
int in_idx = 0, out_idx = 0;

sem_t sem_empty, sem_full, sem_mutex;

void* producer(void* arg) {
    for (int item = 1; item <= N; item++) {
        sem_wait(&sem_empty); // Wait for open space
        sem_wait(&sem_mutex);

        buffer[in_idx] = item;
        cout << "Produced " << item << " | ";
        in_idx = (in_idx + 1) % K;

        sem_post(&sem_mutex);
        sem_post(&sem_full);  // Signal item available
    }
    return NULL;
}

void* consumer(void* arg) {
    for (int i = 1; i <= N; i++) {
        sem_wait(&sem_full);  // Wait for available item
        sem_wait(&sem_mutex);

        int item = buffer[out_idx];
        cout << "Consumed " << item;
        if (i < N) cout << " | ";
        out_idx = (out_idx + 1) % K;

        sem_post(&sem_mutex);
        sem_post(&sem_empty); // Signal open space
    }
    cout << endl;
    return NULL;
}

int main() {
    cin >> N >> K;
    buffer = new int[K];

    sem_init(&sem_empty, 0, K); // Buffer starts empty
    sem_init(&sem_full, 0, 0);   // 0 items ready
    sem_init(&sem_mutex, 0, 1);

    pthread_t prod, cons;
    pthread_create(&prod, NULL, producer, NULL);
    pthread_create(&cons, NULL, consumer, NULL);

    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    sem_destroy(&sem_empty);
    sem_destroy(&sem_full);
    sem_destroy(&sem_mutex);
    delete[] buffer;
    return 0;
}
Problem 3: First Readers-Writers (Reader Preference)Problem StatementRun $R$ Reader threads and $W$ Writer threads.Multiple readers can read a shared counter at the same time.Only one writer can modify the counter at a time, and NO readers may read while a writer is modifying.Once at least one reader holds access, subsequent readers enter without waiting for writers.Each reader reads once; each writer writes once. Print execution traces safely using semaphores.Input format: <Number R Readers of> <Number W Writers of>Sample Input: 3 1Sample Output: Writer updated data | Reader 1 read data | Reader 2 read data | Reader 3 read dataSolutionC++#include <iostream>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

sem_t rw_mutex; // Lock for writers or first/last reader
sem_t mutex;    // Protects reader_count
int reader_count = 0;
int shared_data = 100;

void* reader(void* arg) {
    int id = *(int*)arg;

    sem_wait(&mutex);
    reader_count++;
    if (reader_count == 1) {
        sem_wait(&rw_mutex); // First reader locks out writers
    }
    sem_post(&mutex);

    // Reading critical section
    cout << "Reader " << id << " read data " << shared_data << " | ";

    sem_wait(&mutex);
    reader_count--;
    if (reader_count == 0) {
        sem_post(&rw_mutex); // Last reader allows writers
    }
    sem_post(&mutex);

    return NULL;
}

void* writer(void* arg) {
    sem_wait(&rw_mutex);
    shared_data += 50;
    cout << "Writer updated data | ";
    sem_post(&rw_mutex);
    return NULL;
}

int main() {
    int R, W;
    cin >> R >> W;

    sem_init(&rw_mutex, 0, 1);
    sem_init(&mutex, 0, 1);

    pthread_t r_threads[R], w_threads[W];
    int r_ids[R];

    for (int i = 0; i < W; i++) pthread_create(&w_threads[i], NULL, writer, NULL);
    for (int i = 0; i < R; i++) {
        r_ids[i] = i + 1;
        pthread_create(&r_threads[i], NULL, reader, &r_ids[i]);
    }

    for (int i = 0; i < W; i++) pthread_join(w_threads[i], NULL);
    for (int i = 0; i < R; i++) pthread_join(r_threads[i], NULL);

    cout << endl;
    sem_destroy(&rw_mutex);
    sem_destroy(&mutex);
    return 0;
}
Problem 4: Dining Philosophers (Deadlock-Free Resource Hierarchy)Problem Statement5 Philosophers sit at a round table with 5 chopsticks (0 to 4).To eat, a philosopher $i$ needs both chopstick $i$ (left) and chopstick $(i+1)\%5$ (right).Avoid deadlocks by making odd-numbered philosophers pick up the left chopstick first, and even-numbered philosophers pick up the right chopstick first.Run for $N$ eating cycles per philosopher.Input format: <Eating N cycles per philosopher>Sample Input: 1Sample Output: P0 ate | P1 ate | P2 ate | P3 ate | P4 ateSolutionC++#include <iostream>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

sem_t chopstick[5];
int N;

void* philosopher(void* arg) {
    int id = *(int*)arg;
    int left = id;
    int right = (id + 1) % 5;

    for (int i = 0; i < N; i++) {
        if (id % 2 == 0) {
            sem_wait(&chopstick[right]);
            sem_wait(&chopstick[left]);
        } else {
            sem_wait(&chopstick[left]);
            sem_wait(&chopstick[right]);
        }

        cout << "P" << id << " ate | ";

        sem_post(&chopstick[left]);
        sem_post(&chopstick[right]);
    }
    return NULL;
}

int main() {
    cin >> N;
    for (int i = 0; i < 5; i++) sem_init(&chopstick[i], 0, 1);

    pthread_t phils[5];
    int ids[5];

    for (int i = 0; i < 5; i++) {
        ids[i] = i;
        pthread_create(&phils[i], NULL, philosopher, &ids[i]);
    }

    for (int i = 0; i < 5; i++) pthread_join(phils[i], NULL);
    cout << endl;

    for (int i = 0; i < 5; i++) sem_destroy(&chopstick[i]);
    return 0;
}
Problem 5: Reusable Barrier SynchronizationProblem StatementRun $T$ worker threads.In each phase $k$ (for $N$ phases), every thread executes its first part: printing T<id>_Phase<k>.Barrier Rule: NO thread is allowed to print [Phase k complete] until ALL $T$ threads have completed their first print for that phase. Implement using semaphores without busy-waiting.Input format: <Number T Threads of> <Number N Phases of>Sample Input: 3 2Sample Output:PlaintextT1_Phase1 T2_Phase1 T3_Phase1 [Phase 1 complete]
T1_Phase2 T2_Phase2 T3_Phase2 [Phase 2 complete]
SolutionC++#include <iostream>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

int T, N;
int barrier_count = 0;
sem_t mutex, barrier;

void* worker(void* arg) {
    int id = *(int*)arg;

    for (int phase = 1; phase <= N; phase++) {
        // Step 1: Work before barrier
        sem_wait(&mutex);
        cout << "T" << id << "_Phase" << phase << " ";
        barrier_count++;

        if (barrier_count == T) {
            // Last thread unlocks the barrier for all T threads
            cout << "[Phase " << phase << " complete]" << endl;
            barrier_count = 0;
            for (int i = 0; i < T; i++) sem_post(&barrier);
        }
        sem_post(&mutex);

        // Step 2: Block on barrier
        sem_wait(&barrier);
    }
    return NULL;
}

int main() {
    cin >> T >> N;

    sem_init(&mutex, 0, 1);
    sem_init(&barrier, 0, 0);

    pthread_t threads[T];
    int ids[T];

    for (int i = 0; i < T; i++) {
        ids[i] = i + 1;
        pthread_create(&threads[i], NULL, worker, &ids[i]);
    }

    for (int i = 0; i < T; i++) pthread_join(threads[i], NULL);

    sem_destroy(&mutex);
    sem_destroy(&barrier);
    return 0;
}
Problem 6: Resource Pool Allocation (Multi-Unit Semaphore)Problem StatementA system has $R$ identical GPU resources.Run $T$ worker threads.Thread $i$ needs $K_i$ GPUs simultaneously to run its calculation, then releases them.Use a counting semaphore initialized to $R$ to represent available GPUs, protected by a mutex to ensure safe multi-token acquisition.Input format: <Available GPUs R> <Number T Threads of>Sample Input: 3 2 (Thread 1 needs 2 GPUs; Thread 2 needs 2 GPUs)Sample Output: Thread 1 acquired 2 GPUs | Thread 1 released GPUs | Thread 2 acquired 2 GPUs | Thread 2 released GPUsSolutionC++#include <iostream>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

sem_t gpu_sem;
sem_t lock;

void* requestGPUs(void* arg) {
    int id = *(int*)arg;
    int needed = 2; // Each thread requires 2 GPUs

    sem_wait(&lock);
    for (int i = 0; i < needed; i++) {
        sem_wait(&gpu_sem); // Acquire tokens one by one
    }
    cout << "Thread " << id << " acquired " << needed << " GPUs | ";
    sem_post(&lock);

    // Simulate work
    cout << "Thread " << id << " released GPUs | ";

    for (int i = 0; i < needed; i++) {
        sem_post(&gpu_sem); // Return tokens
    }
    return NULL;
}

int main() {
    int R, T;
    cin >> R >> T;

    sem_init(&gpu_sem, 0, R);
    sem_init(&lock, 0, 1);

    pthread_t threads[T];
    int ids[T];

    for (int i = 0; i < T; i++) {
        ids[i] = i + 1;
        pthread_create(&threads[i], NULL, requestGPUs, &ids[i]);
    }

    for (int i = 0; i < T; i++) pthread_join(threads[i], NULL);
    cout << endl;

    sem_destroy(&gpu_sem);
    sem_destroy(&lock);
    return 0;
}
Problem 7: Binary Rendezvous PointProblem StatementRun two threads: Thread A and Thread B.Thread A must print A1.Thread B must print B1.Both threads must hit a rendezvous point: Neither thread can print A2 or B2 until BOTH A1 and B1 have been printed.Implement using two semaphores initialized to 0.Input format: NoneSample Output: A1 B1 A2 B2 (or B1 A1 B2 A2, or A1 B1 B2 A2)SolutionC++#include <iostream>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

sem_t semA_done, semB_done;

void* threadA(void* arg) {
    cout << "A1 ";
    sem_post(&semA_done); // Signal Thread A reached rendezvous
    sem_wait(&semB_done); // Wait for Thread B
    cout << "A2 ";
    return NULL;
}

void* threadB(void* arg) {
    cout << "B1 ";
    sem_post(&semB_done); // Signal Thread B reached rendezvous
    sem_wait(&semA_done); // Wait for Thread A
    cout << "B2 ";
    return NULL;
}

int main() {
    sem_init(&semA_done, 0, 0);
    sem_init(&semB_done, 0, 0);

    pthread_t tA, tB;
    pthread_create(&tA, NULL, threadA, NULL);
    pthread_create(&tB, NULL, threadB, NULL);

    pthread_join(tA, NULL);
    pthread_join(tB, NULL);

    cout << endl;
    sem_destroy(&semA_done);
    sem_destroy(&semB_done);
    return 0;
}
Problem 8: Asymmetric H2O Molecule BondingProblem StatementRun Hydrogen (H) threads and Oxygen (O) threads.To form a single Water molecule ($H_2O$), exactly 2 Hydrogen threads and 1 Oxygen thread must bond.Thread 3 (Oxygen) acts as the barrier leader: once two 'H's and one 'O' are present, print H2O and unblock the next group. Run for $N$ molecules.Input format: <Number H2O N molecules of>Sample Input: 2Sample Output: H2O H2OSolutionC++#include <iostream>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

sem_t semH, semO, semBond;
int N;

void* hydrogen(void* arg) {
    for (int i = 0; i < N * 2; i++) {
        sem_wait(&semH);
        sem_post(&semBond); // Signal arrival to Oxygen
    }
    return NULL;
}

void* oxygen(void* arg) {
    for (int i = 0; i < N; i++) {
        sem_wait(&semO);

        // Wait for 2 Hydrogen threads to arrive
        sem_wait(&semBond);
        sem_wait(&semBond);

        cout << "H2O ";

        // Replenish permits for next molecule
        sem_post(&semH);
        sem_post(&semH);
        sem_post(&semO);
    }
    return NULL;
}

int main() {
    cin >> N;

    sem_init(&semH, 0, 2); // Start with permit for 2 H's
    sem_init(&semO, 0, 1); // Start with permit for 1 O
    sem_init(&semBond, 0, 0);

    pthread_t tH, tO;
    pthread_create(&tH, NULL, hydrogen, NULL);
    pthread_create(&tO, NULL, oxygen, NULL);

    pthread_join(tH, NULL);
    pthread_join(tO, NULL);

    cout << endl;
    sem_destroy(&semH);
    sem_destroy(&semO);
    sem_destroy(&semBond);
    return 0;
}
Problem 9: Reader-Writer with Writer Priority (No Writer Starvation)Problem StatementStandard Reader-Writer code starves writers if readers arrive continuously.Implement Writer-Preference Readers-Writers:If a writer arrives, subsequent readers MUST block until all waiting writers have finished writing.Use semaphores readTry (blocks incoming readers when a writer is queued), rw_mutex, and counter locks.Input format: <Readers R> <Writers W>Sample Input: 2 2Sample Output: Writer 1 wrote | Writer 2 wrote | Reader 1 read | Reader 2 readSolutionC++#include <iostream>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

sem_t readTry, rMutex, wMutex, resource;
int rCount = 0, wCount = 0;

void* writer(void* arg) {
    int id = *(int*)arg;

    sem_wait(&wMutex);
    wCount++;
    if (wCount == 1) sem_wait(&readTry); // First writer blocks incoming readers
    sem_post(&wMutex);

    sem_wait(&resource); // Acquire exclusive write access
    cout << "Writer " << id << " wrote | ";
    sem_post(&resource);

    sem_wait(&wMutex);
    wCount--;
    if (wCount == 0) sem_post(&readTry); // Last writer allows readers again
    sem_post(&wMutex);

    return NULL;
}

void* reader(void* arg) {
    int id = *(int*)arg;

    sem_wait(&readTry); // Check if writers are waiting
    sem_wait(&rMutex);
    rCount++;
    if (rCount == 1) sem_wait(&resource); // First reader locks resource from writers
    sem_post(&rMutex);
    sem_post(&readTry);

    cout << "Reader " << id << " read | ";

    sem_wait(&rMutex);
    rCount--;
    if (rCount == 0) sem_post(&resource); // Last reader releases resource
    sem_post(&rMutex);

    return NULL;
}

int main() {
    int R, W;
    cin >> R >> W;

    sem_init(&readTry, 0, 1);
    sem_init(&rMutex, 0, 1);
    sem_init(&wMutex, 0, 1);
    sem_init(&resource, 0, 1);

    pthread_t rT[R], wT[W];
    int rIds[R], wIds[W];

    for (int i = 0; i < W; i++) { wIds[i] = i + 1; pthread_create(&wT[i], NULL, writer, &wIds[i]); }
    for (int i = 0; i < R; i++) { rIds[i] = i + 1; pthread_create(&rT[i], NULL, reader, &rIds[i]); }

    for (int i = 0; i < W; i++) pthread_join(wT[i], NULL);
    for (int i = 0; i < R; i++) pthread_join(rT[i], NULL);

    cout << endl;
    sem_destroy(&readTry);
    sem_destroy(&rMutex);
    sem_destroy(&wMutex);
    sem_destroy(&resource);
    return 0;
}
Problem 10: Sleeping Barber ProblemProblem StatementA barbershop has 1 Barber chair and $N$ waiting chairs.If there are no customers, the Barber sleeps (sem_wait(&customers)).When a customer arrives:If all $N$ chairs are full, the customer leaves (cout << "Customer left").Otherwise, the customer sits in a chair, wakes the barber (sem_post(&customers)), and waits for the haircut (sem_wait(&barber)).Run for $C$ total arriving customers.Input format: <Waiting N chairs> <Total C arriving customers>Sample Input: 1 3Sample Output: Customer 1 getting haircut | Customer 2 getting haircut | Customer 3 leftSolutionC++#include <iostream>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

int N, C;
int waiting_customers = 0;

sem_t sem_customers;
sem_t sem_barber;
sem_t lock;

void* barber_func(void* arg) {
    for (int i = 0; i < C; i++) {
        // Wait for a customer (or exit check)
        if (sem_wait(&sem_customers) != 0) break;

        sem_wait(&lock);
        waiting_customers--;
        sem_post(&sem_barber); // Signal customer to sit in barber chair
        sem_post(&lock);
    }
    return NULL;
}

void* customer_func(void* arg) {
    int id = *(int*)arg;

    sem_wait(&lock);
    if (waiting_customers < N) {
        waiting_customers++;
        cout << "Customer " << id << " getting haircut | ";
        sem_post(&sem_customers); // Wake barber
        sem_post(&lock);

        sem_wait(&sem_barber);    // Wait for barber ready
    } else {
        cout << "Customer " << id << " left | ";
        sem_post(&lock);
    }
    return NULL;
}

int main() {
    cin >> N >> C;

    sem_init(&sem_customers, 0, 0);
    sem_init(&sem_barber, 0, 0);
    sem_init(&lock, 0, 1);

    pthread_t b_thread, c_threads[C];
    int ids[C];

    pthread_create(&b_thread, NULL, barber_func, NULL);

    for (int i = 0; i < C; i++) {
        ids[i] = i + 1;
        pthread_create(&c_threads[i], NULL, customer_func, &ids[i]);
    }

    for (int i = 0; i < C; i++) pthread_join(c_threads[i], NULL);

    cout << endl;
    sem_destroy(&sem_customers);
    sem_destroy(&sem_barber);
    sem_destroy(&lock);
    return 0;