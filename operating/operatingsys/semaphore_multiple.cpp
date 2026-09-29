#define N 100
#define M 5                          /* Number of producers, e.g., 5 */

typedef int semaphore;
semaphore mutex = 1;                 /* Controls buffer access */
semaphore empty = N;                 /* Tracks empty slots */
semaphore full = 0;                  /* Tracks full slots */

semaphore P_turn[M];                 /* Array of semaphores for cyclic order */

void init_semaphores() {
    P_turn[0] = 1;                   /* P1 starts first */
    for (int i = 1; i < M; i++) {
        P_turn[i] = 0;               /* All other producers start blocked */
    }
}

/* Producer code for Producer i (where i ranges from 0 to M-1) */
void producer(int i) {
    int item;
    while (TRUE) {
        item = produce_item();
        
        down(&P_turn[i]);             /* Wait for Producer i's turn in cycle */
        down(&empty);                 /* Check for available buffer slot */
        down(&mutex);                 /* Enter critical region */
        
        insert_item(item);            /* Place item in buffer */
        
        up(&mutex);                   /* Leave critical region */
        up(&full);                    /* Signal consumer */
        
        up(&P_turn[(i + 1) % M]);     /* Hand over turn to Producer (i+1) */
    }
}