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
