// Purpose: BankAccount class with constructor and methods for deposit, withdrawal, and display
// Date: 26/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>
#include <string>
using namespace std;

class BankAccount {
    int accNo;
    string name;
    double balance;

public:
    // Constructor
    BankAccount(int no, string n, double bal) {
        accNo = no;
        name = n;
        balance = bal;
    }

    void deposit(double amt) {
        balance += amt;
    }

    void withdraw(double amt) {
        if (amt <= balance)
            balance -= amt;
        else
            cout << "Insufficient Balance!" << endl;
    }

    void display() {
        cout << "Account Number: " << accNo << endl;
        cout << "Account Holder: " << name << endl;
        cout << "Balance: " << balance << endl;
    }
};

int main() {
    BankAccount b1(102, "Anita", 8000);
    b1.display();

    b1.deposit(1500);
    cout << "\nAfter Deposit:\n";
    b1.display();

    b1.withdraw(4000);
    cout << "\nAfter Withdrawal:\n";
    b1.display();

    return 0;
}
