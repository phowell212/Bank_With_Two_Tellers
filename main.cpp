#include <iostream>
#include <iomanip>
#include <queue>
#include <random>
#include <vector>
#include <numeric>

const int NUM_CUSTOMERS = 100;

using namespace std;

struct Customer {
    int arrivalTime;
    int customerID;
    int waitTime;
    int orderServed = 0;
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
    int customersProcessed = 0;

    // Init the vectors for the histograms
    vector<int> teller1Times;
    vector<int> teller2Times;
    vector<int> interArrivalTimes;

    // Give each customer an ID and a random arrival time
    for(int i = 0; i < NUM_CUSTOMERS; i++) {
        customers[i].customerID = i;
        customers[i].arrivalTime = int(rand() % 3 + 1); // Random time between 1 and 4
    }

    // Randomize the array of customers - This doesn't really matter, but it's more realistic
    shuffle(begin(customers), end(customers), std::mt19937(std::random_device()()));

    // Process each customer
    queue<Customer*> waitingCustomers;
    int minute = 0;
    int lastCustomerAddedTime = 0;
    int nextCustomerIndex = 0;
    while(any_of(customers.begin(), customers.end(), [](Customer& c){ return !c.processed; })) {
        for(auto &t : tellers) {

            // Assign a customer to the teller if they are available
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


            // Process the current customer
            } else if (!t.isAvailable) {
                t.processingTime--;
                if(t.currentCustomer != nullptr) {
                    t.currentCustomer->waitTime++;
                }
                if(t.processingTime <= 0) {
                    customersProcessed++;
                    t.currentCustomer->orderServed = customersProcessed;
                    t.currentCustomer->processed = true;
                    t.currentCustomer = nullptr;
                    t.isAvailable = true;
                }
            }

            // Add customers to the queue if their arrival time has come
            if (nextCustomerIndex < customers.size()) {
                auto &nextCustomer = customers[nextCustomerIndex];
                if (minute - lastCustomerAddedTime >= nextCustomer.arrivalTime && !nextCustomer.processed) {
                    waitingCustomers.push(&nextCustomer);

                    // Add the inter-arrival time to the histogram vector
                    if (lastCustomerAddedTime != 0) {
                        interArrivalTimes.push_back(minute - lastCustomerAddedTime);
                    }
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

    // Sort the customers by ID
    sort(customers.begin(), customers.end(), [](Customer& a, Customer& b) { return a.customerID < b.customerID; });

    // Print the table header
    cout << left << setw(15) << "Customer ID" << setw(15) << "Wait Time (m)" << setw(15) << "Order Served" << endl;
    cout << string(45, '-') << endl; // Print a separator line

    // Print the results
    for(auto &c : customers) {
        cout << left << setw(15) << c.customerID << setw(15) << c.waitTime << setw(15) << c.orderServed << endl;
    }

    // Print the histograms
    cout << endl << "Inter-arrival times histogram:" << endl;
    cout << left << setw(10) << "Time" << setw(10) << "Count" << endl;
    cout << string(20, '-') << endl; // Print a separator line
    for(int i = 1; i <= *max_element(interArrivalTimes.begin(), interArrivalTimes.end()); i++) {
        cout << left << setw(10) << i << setw(10) << count(interArrivalTimes.begin(), interArrivalTimes.end(), i) << endl;
    }

    cout << endl << "Teller 1 histogram:" << endl;
    cout << left << setw(10) << "Time" << setw(10) << "Count" << endl;
    cout << string(20, '-') << endl; // Print a separator line
    for(int i = 2; i <= 7; i++) {
        cout << left << setw(10) << i << setw(10) << count(teller1Times.begin(), teller1Times.end(), i) << endl;
    }

    cout << endl << "Teller 2 histogram:"  << endl;
    cout << left << setw(10) << "Time" << setw(10) << "Count" << endl;
    cout << string(20, '-') << endl; // Print a separator line
    for(int i = 2; i <= 7; i++) {
        cout << left << setw(10) << i << setw(10) << count(teller2Times.begin(), teller2Times.end(), i) << endl;
    }

    cout << endl << "The Bank took " << minute << " minutes to process all customers.";
    cout << endl << "The average wait time was " << accumulate(customers.begin(), customers.end(), 0, [](int a, Customer& b) { return a + b.waitTime; }) / NUM_CUSTOMERS << " minutes.";
    return 0;
}