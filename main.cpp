#include <iostream>
#include <iomanip>
#include <queue>
#include <random>
#include <vector>
#include <numeric>
#include <algorithm>

const int NUM_CUSTOMERS = 100;
const bool SHOW_TABLE = true;

using namespace std;

struct Customer {
    int interArrivalTime = 0;
    int customerID = 0;
    int waitTime = 0;
    int orderServed = 0;
    int timeAtDesk = 0;
    int minuteArrived = 0;
    int minuteFinished = 0;
    int teller = 0;
    int tellerPreviousIdleTime = 0;
    bool processed = false;
};

struct Teller {
    int processingTime = 0;
    int activeTime = 0;
    int idleTime = 0;
    bool isAvailable = true;
    vector<pair<int, float>> probabilities;
    Customer* currentCustomer = nullptr;
};

int getRandomTime(Teller& teller) {
    float randNum = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    for (const auto& p : teller.probabilities) {
        if ((randNum -= p.second) < 0)
            return p.first;
    }
    return teller.probabilities.back().first;
}

int main() {
    // Seed the random number generator
    srand(time(NULL));

    // Init the actors
    vector<Customer> customers(NUM_CUSTOMERS);
    Teller tellers[2];
    tellers[0].probabilities = {{2, 0.1}, {3, 0.4}, {4, 0.2}, {5, 0.1}, {6, 0.1}, {7, 0.1}};
    tellers[1].probabilities = {{2, 0.1}, {3, 0.1}, {4, 0.1}, {5, 0.2}, {6, 0.4}, {7, 0.1}};
    int customersProcessed = 0;

    // Give each customer an ID and a random arrival time
    for(int i = 0; i < NUM_CUSTOMERS; i++) {
        customers[i].customerID = i;
        customers[i].interArrivalTime = rand() % 4 + 1; // Random time between 1 and 4 minutes
    }

    // Randomize the array of customers - This doesn't really matter, but it's more realistic
    shuffle(begin(customers), end(customers), mt19937(random_device()()));

    // Process each customer
    queue<Customer*> waitingCustomers;
    int minute = 0;

    while(any_of(customers.begin(), customers.end(), [](Customer& c){ return !c.processed; })) {


        // Add customers to the queue if their arrival time has come
        //@todo: make this happen only to the "next" customer
        for (auto &customer : customers) {
            if (minute == customer.interArrivalTime && !customer.processed) {
                waitingCustomers.push(&customer);
                customer.minuteArrived = minute;
            }
        }

        // Assign customers to tellers
        for (int i = 0; i < 2; ++i) {
            if (tellers[i].isAvailable && !waitingCustomers.empty()) {
                tellers[i].currentCustomer = waitingCustomers.front();
                tellers[i].currentCustomer->teller = i;
                waitingCustomers.pop();
                tellers[i].processingTime = getRandomTime(tellers[i]);

                tellers[i].currentCustomer->orderServed = customersProcessed + 1;
                customersProcessed++;
                tellers[i].isAvailable = false;

                if (tellers[i].currentCustomer->orderServed != 1) {
                    tellers[i].currentCustomer->tellerPreviousIdleTime = tellers[i].idleTime;
                    tellers[i].idleTime = 0;
                }
            }
        }

        // Process the current customer
        for (auto &t : tellers) {
            if (!t.isAvailable) {
                t.processingTime--;
                t.activeTime++;
                if (t.currentCustomer != nullptr) {
                    t.currentCustomer->waitTime++;
                    t.currentCustomer->timeAtDesk++;
                }
                if (t.processingTime <= 0) {
                    t.currentCustomer->processed = true;
                    t.currentCustomer->minuteFinished = minute;
                    t.currentCustomer = nullptr;
                    t.isAvailable = true;
                }
            } else {
                t.idleTime++;
            }
        }

        // Increase the wait time of all customers in the queue
        queue<Customer*> tempQueue;
        while(!waitingCustomers.empty()) {
            auto c = waitingCustomers.front();
            c->waitTime++;
            tempQueue.push(c);
            waitingCustomers.pop();
        }
        waitingCustomers = tempQueue;

        minute++;
    }

    // Sort the customers by their order served
    sort(customers.begin(), customers.end(), [](Customer& a, Customer& b) { return a.orderServed < b.orderServed; });

    // Print the table
    if(SHOW_TABLE) {
        cout << left << setw(20) << "Customer ID" << setw(20) << "Wait Time" << setw(20) << "Time in Bank" <<
            setw(20) << "Teller 1 Active" << setw(20) << "Teller 1 Idle" << setw(20) << "Teller 2 Active" <<
            setw(20) << "Teller 2 Idle" << setw(20) << "Minute Arrived" << setw(20) << "Minute Finished" << endl;
        cout << string(180, '-') << endl;

        for (const auto& customer : customers) {
            string teller1Active = "-", teller1Idle = "-", teller2Active = "-", teller2Idle = "-";
            if (customer.teller == 0) {
                teller1Active = to_string(customer.timeAtDesk);
                teller1Idle = to_string(customer.tellerPreviousIdleTime);
            } else if (customer.teller == 1) {
                teller2Active = to_string(customer.timeAtDesk);
                teller2Idle = to_string(customer.tellerPreviousIdleTime);
            }

            cout << left << setw(20) << customer.customerID << setw(20) << customer.waitTime << setw(20) <<
                customer.waitTime + customer.timeAtDesk << setw(20) << teller1Active << setw(20) << teller1Idle <<
                setw(20) << teller2Active << setw(20) << teller2Idle << setw(20) << customer.minuteArrived <<
                setw(20) << customer.minuteFinished << endl;
        }
    }

    // Compute the performance metrics
    cout << endl << "Performance Metrics:" << endl;
    cout << "Average customer time in queue: " << accumulate(customers.begin(), customers.end(), 0,
        [](int a, Customer& b) { return a + b.waitTime; }) / NUM_CUSTOMERS << " minutes." << endl;

    cout << endl << "The Bank took " << minute << " minutes to process all customers.";
    return 0;

}