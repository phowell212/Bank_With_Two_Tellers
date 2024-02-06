#include <iostream>
#include <iomanip>
#include <queue>
#include <random>
#include <vector>
#include <numeric>
#include <algorithm>

const int NUM_CUSTOMERS = 25;

using namespace std;

struct Customer {
    int arrivalTime = 0;
    int customerID = 0;
    int waitTime = 0;
    int orderServed = 0;
    int timeAtDesk = 0;
    int teller = 0;
    int perviousTellerIdleTime = 0;
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

    // Init the vectors for the histograms
    vector<int> teller1Times;
    vector<int> teller2Times;
    vector<int> interArrivalTimes;

    // Give each customer an ID and a random arrival time
    for(int i = 0; i < NUM_CUSTOMERS; i++) {
        customers[i].customerID = i;
        customers[i].arrivalTime = int(rand() % 5); // Random time between 1 and 4
    }

    // Randomize the array of customers - This doesn't really matter, but it's more realistic
    shuffle(begin(customers), end(customers), mt19937(random_device()()));

    // Process each customer
    queue<Customer*> waitingCustomers;
    int minute = 0;
    int lastCustomerAddedTime = 0;
    int nextCustomerIndex = 0;

    while(any_of(customers.begin(), customers.end(), [](Customer& c){ return !c.processed; })) {

        // Assign a customer to teller 0 if they are available
        if(tellers[0].isAvailable && !waitingCustomers.empty()) {
            tellers[0].currentCustomer = waitingCustomers.front();
            tellers[0].currentCustomer->teller = 0;
            waitingCustomers.pop();
            tellers[0].processingTime = getRandomTime(tellers[0]);

            // Assign the order served to the customer
            tellers[0].currentCustomer->orderServed = customersProcessed + 1;
            customersProcessed++;
            tellers[0].isAvailable = false;

            // Set the Previous Teller Idle Time
            if(tellers[0].currentCustomer->orderServed != 1) {
                tellers[0].currentCustomer->perviousTellerIdleTime = tellers[0].idleTime;
                tellers[0].idleTime = 0;
            }

            // Add the processing time to the histogram vector
            teller1Times.push_back(tellers[0].processingTime);
        }

        // If teller 0 is not available, assign a customer to teller 1
        else if(tellers[1].isAvailable && !waitingCustomers.empty()) {
            tellers[1].currentCustomer = waitingCustomers.front();
            tellers[1].currentCustomer->teller = 1;
            waitingCustomers.pop();
            tellers[1].processingTime = getRandomTime(tellers[1]);

            // Assign the order served to the customer
            tellers[1].currentCustomer->orderServed = customersProcessed + 1;
            customersProcessed++;
            tellers[1].isAvailable = false;

            // Set the Previous Teller Idle Time
            if(tellers[1].currentCustomer->orderServed != 1) {
                tellers[1].currentCustomer->perviousTellerIdleTime = tellers[1].idleTime;
                tellers[1].idleTime = 0;
            }

            // Add the processing time to the histogram vector
            teller2Times.push_back(tellers[1].processingTime);
        }

        // Process the current customer
        for(auto &t : tellers) {
            if (!t.isAvailable) {
                t.processingTime--;
                t.activeTime++;
                // Increase the wait time of the customer at the desk
                if(t.currentCustomer != nullptr) {
                    t.currentCustomer->waitTime++;
                    t.currentCustomer->timeAtDesk++;
                }
                if(t.processingTime <= 0) {
                    t.currentCustomer->processed = true;
                    t.currentCustomer = nullptr;
                    t.isAvailable = true;
                }
            } else {
                t.idleTime++;
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
    cout << left << setw(10) << "Customer ID " << setw(15) << "Wait Time " << setw(20) << "Time in Bank " << setw(20) << "Teller 1 Active " << setw(20) << "Teller 1 Idle " << setw(20) << "Teller 2 Active " << setw(20) << "Teller 2 Idle " << endl;
    cout << string(140, '-') << endl;
    for (const auto& customer : customers) {
        string teller1Active = "-";
        string teller1Idle = "-";
        string teller2Active = "-";
        string teller2Idle = "-";

        if (customer.teller == 0) {
            teller1Active = to_string(customer.timeAtDesk);
            teller1Idle = to_string(customer.perviousTellerIdleTime);
        } else if (customer.teller == 1) {
            teller2Active = to_string(customer.timeAtDesk);
            teller2Idle = to_string(customer.perviousTellerIdleTime);
        }

        cout << left << setw(10) << customer.customerID << setw(15) << customer.waitTime << setw(20) << customer.waitTime + customer.timeAtDesk << setw(20) << teller1Active << setw(20) << teller1Idle << setw(20) << teller2Active << setw(20) << teller2Idle << endl;
    }

    // Compute the performance metrics
    cout << endl << "Performance Metrics:" << endl;

    cout << "Average customer time in queue: " << accumulate(customers.begin(), customers.end(), 0,
        [](int a, Customer& b) { return a + b.waitTime; }) / NUM_CUSTOMERS << " minutes." << endl;
    int totalBankTime = 0;
    for (const auto& customer : customers) {
        totalBankTime += customer.waitTime;
        if (customer.orderServed != 0 && customer.orderServed - 1 < teller1Times.size()) {
            totalBankTime += teller1Times[customer.orderServed - 1];
        }
    }
    cout << "Average customer time in the bank: " << totalBankTime / NUM_CUSTOMERS << " minutes." << endl;
    cout << "Fraction of Teller 1 active time: " << static_cast<float>(tellers[0].activeTime) / minute << endl;
    cout << "Fraction of Teller 1 idle time: " << static_cast<float>(minute - tellers[0].activeTime) / minute << endl;
    cout << "Fraction of Teller 2 active time: " << static_cast<float>(tellers[1].activeTime) / minute << endl;
    cout << "Fraction of Teller 2 idle time: " << static_cast<float>(minute - tellers[1].activeTime) / minute << endl;

    // Print the histograms
    cout << endl << "Inter-arrival times histogram:" << endl;
    cout << left << setw(10) << "Time" << setw(10) << "Count" << endl;
    cout << string(20, '-') << endl;
    for(int i = 1; i <= *max_element(interArrivalTimes.begin(), interArrivalTimes.end()); i++) {
        cout << left << setw(10) << i << setw(10) << count(interArrivalTimes.begin(), interArrivalTimes.end(), i) << endl;
    }

    cout << endl << "Teller 1 histogram:" << endl;
    cout << left << setw(10) << "Time" << setw(10) << "Count" << endl;
    cout << string(20, '-') << endl;
    for(int i = 2; i <= 7; i++) {
        cout << left << setw(10) << i << setw(10) << count(teller1Times.begin(), teller1Times.end(), i) << endl;
    }

    cout << endl << "Teller 2 histogram:"  << endl;
    cout << left << setw(10) << "Time" << setw(10) << "Count" << endl;
    cout << string(20, '-') << endl;
    for(int i = 2; i <= 7; i++) {
        cout << left << setw(10) << i << setw(10) << count(teller2Times.begin(), teller2Times.end(), i) << endl;
    }

    cout << endl << "The Bank took " << minute << " minutes to process all customers.";
}