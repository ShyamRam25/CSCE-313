#include <iostream>
#include <thread>
#include <condition_variable>
#include <semaphore.h>
#include <mutex>
#include <unistd.h>
#include <vector>

using namespace std;
#define MAX_THREADS 100

#define BURGER 0
#define FRIES 1
const char* type_names[] = {"BURGER", "FRIES"};
#define pii pair<int, int>

int k;
int num_orders;
int num_burgers;
int num_fries;

sem_t burger;
sem_t fries;
mutex m;

// Do not change
void process_order() {
    sleep(2);
}

void place_order(int type) {
    
    if(num_orders >= k){

        cout << "Waiting: " << type_names[type] << endl;
        unique_lock<mutex> u_lock(m);
        u_lock.unlock();
        
        if (type == 0) {
            num_burgers++;
            sem_wait(&burger);

        } else {
            num_fries++;
            sem_wait(&fries);
        }
    }
   
    unique_lock<mutex> u_lock(m);
    cout << "Order: " << type_names[type] << endl; 
    
    u_lock.unlock();
    num_orders++;

    process_order();        // Do not remove, simulates preparation
    
    if ((type == 0 && num_burgers > 0) || (num_fries > 0)) {
        if (type == 0) {
            sem_post(&burger);
            num_burgers--;
        } else {
            sem_post(&fries);
            num_fries--;
        }
    } 
    else if (((type == 0 && num_fries == 0) && (num_burgers > 0)) || ((type == 0 && num_burgers == 0) && (type == 0 && num_fries > 0))) {
        if (type == 0) {
            sem_post(&fries);
            num_fries--;
        } else {
            sem_post(&burger);
            num_burgers--;
        }
    } 
    
    num_orders--;
}

int main() {
    // Initialize necessary variables, semaphores etc.
    //sem_init(&sem, 0, 1)
    sem_init(&burger, 0, k);
    sem_init(&fries, 0, k);

    // Read data: done for you, do not change
    pii incoming[MAX_THREADS];
    int _type, _arrival;
    int t;
    cin >> k;
    cin >> t;
    for (int i = 0; i < t; ++i) {
        cin >> _type >> _arrival;
        incoming[i].first = _type;
        incoming[i].second = _arrival;
    }

    // Create threads: done for you, do not change
    thread* threads[MAX_THREADS];
    for (int i = 0; i < t; ++i) {
        _type = incoming[i].first;
        threads[i] = new thread(place_order, _type);
        if (i < t - 1) {
            int _sleep = incoming[i + 1].second - incoming[i].second;
            sleep(_sleep);
        }
    }


    // Join threads: done for you, do not change
    for (int i = 0; i < t; ++i) {
        threads[i]->join();
        delete threads[i];
    }
    return 0;
}