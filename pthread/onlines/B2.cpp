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
