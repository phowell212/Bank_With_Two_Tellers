#include <iostream>
#include <queue>
#include <vector>
#include <algorithm>

const int NUM_CUSTOMERS = 100;

using namespace std;

struct Customer {
    int arrivalTime;
    int customerID;
    int waitTime = 0;
    bool processed = false;
};

struct Teller {
    int processingTime = 0;
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

    // Init the teller processing time vectors for the histogram later
    vector<int> teller1Times;
    vector<int> teller2Times;

    // Give each customer an ID and a random arrival time
    for(int i = 0; i < NUM_CUSTOMERS; i++) {
        customers[i].customerID = i;
        customers[i].arrivalTime = int(rand() % 3 + 1); // Random time between 1 and 4
    }

    // Randomize the array of customers - This doesn't really matter, but it's more realistic
    random_shuffle(begin(customers), end(customers));

    // Process each customer
    queue<Customer*> waitingCustomers;
    int minute = 0;
    int lastCustomerAddedTime = 0;
    int nextCustomerIndex = 0;
    while(any_of(customers.begin(), customers.end(), [](Customer& c){ return !c.processed; })) {
        for(auto &t : tellers) {

            // Process the current customer
            if(t.isAvailable && !waitingCustomers.empty()) {
                t.currentCustomer = waitingCustomers.front();
                waitingCustomers.pop();
                t.processingTime = getRandomTime(t);
                t.isAvailable = false;

                // Add the processing time to the histogram vector
                if (&t - tellers == 0) {
                    teller1Times.push_back(t.processingTime);
                } else {
                    teller2Times.push_back(t.processingTime);
                }

            } else if (!t.isAvailable) {
                t.processingTime--;
                if(t.currentCustomer != nullptr) {
                    t.currentCustomer->waitTime++;
                }
                if(t.processingTime <= 0) {
                    t.isAvailable = true;
                    t.currentCustomer->processed = true;
                    t.currentCustomer = nullptr;
                }
            }

            // Add customers to the queue if their arrival time has come
            if (nextCustomerIndex < customers.size()) {
                auto &nextCustomer = customers[nextCustomerIndex];
                if (minute - lastCustomerAddedTime >= nextCustomer.arrivalTime && !nextCustomer.processed) {
                    waitingCustomers.push(&nextCustomer);
                    lastCustomerAddedTime = minute;
                    nextCustomerIndex++;
                }
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

    // Print the results
    for(auto &c : customers) {
        cout << "Customer " << c.customerID << " waited " << c.waitTime << " minutes." << endl;
    }
    cout << "Teller 1 histogram:\n";
    for(int i = 2; i <= 7; i++) {
        cout << i << ": " << count(teller1Times.begin(), teller1Times.end(), i) << "\n";
    }
    cout << "Teller 2 histogram:\n";
    for(int i = 2; i <= 7; i++) {
        cout << i << ": " << count(teller2Times.begin(), teller2Times.end(), i) << "\n";
    }
    cout << "Total time: " << minute << " minutes." << endl;
}