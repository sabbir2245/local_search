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
