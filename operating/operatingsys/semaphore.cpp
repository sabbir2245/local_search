#include <stdio.h>

#define TRUE 1

/* Shared variables */
int buffer[10];
int in = 0, out = 0;

/* Semaphore and mutex */
typedef struct {
    int value;
} semaphore;

typedef struct {
    int locked;
} mutex;

semaphore numOfemptysolts= {10};
semaphore filledsolts = {0};
mutex m = {0};

/* Semaphore operations */
void down(semaphore *s) {
    while (s->value == 0)
        ;               // Wait (busy waiting)

    s->value--;
}

void up(semaphore *s) {
    s->value++;
}

/* Mutex operations */
void lock(mutex *m) {
    while (m->locked == 1) { // do nothig
       
    };               // Wait until unlocked

    m->locked = 1;
}

void unlock(mutex *m) {
    m->locked = 0;
}

/* Buffer operations */
int produce_item() {
    static int x = 1;
    return x++;
}

void insert_item(int item) {
    buffer[in] = item;
    in = (in + 1) % 10;
}

int remove_item() {
    int item = buffer[out];
    out = (out + 1) % 10;
    return item;
}

void consume_item(int item) {
    printf("Consumed: %d\n", item);
}

/* Producer */
void producer() {
    int item;

    while (TRUE) {
        item = produce_item();

        down(&numOfemptysolts);
        lock(&m);

        insert_item(item);

        unlock(&m);
        up(&filledsolts);
    }
}

/* Consumer */
void consumer() {
    int item;

    while (TRUE) {
        down(&filledsolts);
        lock(&m);

        item = remove_item();

        unlock(&m);
        up(&numOfemptysolts);

        consume_item(item);
    }
}