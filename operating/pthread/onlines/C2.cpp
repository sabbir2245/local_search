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
