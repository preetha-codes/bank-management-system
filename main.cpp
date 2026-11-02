
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

const string ACCOUNT_FILE = "accounts.txt";
const string TRANSACTION_FILE = "transactions.txt";

// ============================================================
// TRANSACTION CLASS
// ============================================================

class Transaction {
private:
    string accountNumber;
    string type;
    double amount;
    double resultingBalance;

public:
    // Default constructor
    Transaction() {
        amount = 0;
        resultingBalance = 0;
    }

    // Parameterized constructor
    Transaction(string acc, string transactionType,
                double value, double balance) {
        accountNumber = acc;
        type = transactionType;
        amount = value;
        resultingBalance = balance;
    }

    string getAccountNumber() const {
        return accountNumber;
    }

    void display() const {
        cout << left
             << setw(15) << type
             << setw(15) << fixed << setprecision(2) << amount
             << setw(18) << resultingBalance << "\n";
    }

    string toFileString() const {
        stringstream ss;

        ss << accountNumber << "|"
           << type << "|"
           << fixed << setprecision(2) << amount << "|"
           << resultingBalance;

        return ss.str();
    }

    static bool fromFileString(const string& line,
                               Transaction& result) {
        stringstream ss(line);

        string acc, type, amountText, balanceText;

        if (!getline(ss, acc, '|')) return false;
        if (!getline(ss, type, '|')) return false;
        if (!getline(ss, amountText, '|')) return false;
        if (!getline(ss, balanceText)) return false;

        try {
            double amount = stod(amountText);
            double balance = stod(balanceText);

            if (acc.empty() || amount <= 0 || balance < 0)
                return false;

            result = Transaction(acc, type, amount, balance);
            return true;
        }
        catch (...) {
            return false;
        }
    }
};

// ============================================================
// ACCOUNT CLASS
// ============================================================

class Account {
private:
    string accountNumber;
    string customerName;
    string accountType;
    double balance;

public:
    Account() {
        balance = 0;
    }

    Account(string number, string name,
            string type, double initialBalance) {
        accountNumber = number;
        customerName = name;
        accountType = type;
        balance = initialBalance;
    }

    // Getter functions
    string getAccountNumber() const {
        return accountNumber;
    }

    string getCustomerName() const {
        return customerName;
    }

    string getAccountType() const {
        return accountType;
    }

    double getBalance() const {
        return balance;
    }

    // Deposit operation
    bool deposit(double amount) {
        if (amount <= 0 ||
            amount > 1000000000000.0 - balance) {
            return false;
        }

        balance += amount;
        return true;
    }

    // Withdrawal operation
    bool withdraw(double amount) {
        if (amount <= 0 || amount > balance) {
            return false;
        }

        balance -= amount;
        return true;
    }

    void display() const {
        cout << "\n---------------------------------------------\n";
        cout << "ACCOUNT NUMBER : " << accountNumber << "\n";
        cout << "CUSTOMER NAME  : " << customerName << "\n";
        cout << "ACCOUNT TYPE   : " << accountType << "\n";
        cout << "BALANCE        : Rs. "
             << fixed << setprecision(2) << balance << "\n";
        cout << "---------------------------------------------\n";
    }

    string toFileString() const {
        stringstream ss;

        ss << accountNumber << "|"
           << customerName << "|"
           << accountType << "|"
           << fixed << setprecision(2) << balance;

        return ss.str();
    }

    static bool fromFileString(const string& line,
                               Account& result) {
        stringstream ss(line);

        string number, name, type, balanceText;

        if (!getline(ss, number, '|')) return false;
        if (!getline(ss, name, '|')) return false;
        if (!getline(ss, type, '|')) return false;
        if (!getline(ss, balanceText)) return false;

        try {
            double balance = stod(balanceText);

            if (number.empty() || name.empty() || type.empty() ||
                balance < 0 || balance > 1000000000000.0) {
                return false;
            }

            result = Account(number, name, type, balance);
            return true;
        }
        catch (...) {
            return false;
        }
    }
};

// ============================================================
// INPUT VALIDATION
// ============================================================

string readText(string prompt) {
    string value;

    while (true) {
        cout << prompt;
        getline(cin, value);

        if (!value.empty() &&
            value.find_first_not_of(' ') != string::npos &&
            value.find('|') == string::npos) {
            return value;
        }

        cout << "Invalid input! Enter valid text without |.\n";
    }
}

string readAccountNumber(string prompt) {
    string value;

    while (true) {
        cout << prompt;
        getline(cin, value);

        if (value.length() == 8 &&
            value.find_first_not_of("0123456789") == string::npos) {
            return value;
        }

        cout << "Enter a valid 8-digit account number.\n";
    }
}

int readInt(string prompt, int minimum, int maximum) {
    string line;
    int value;
    char extra;

    while (true) {
        cout << prompt;
        getline(cin, line);

        stringstream ss(line);

        if (ss >> value && !(ss >> extra) &&
            value >= minimum && value <= maximum) {
            return value;
        }

        cout << "Invalid input! Enter a number between "
             << minimum << " and " << maximum << ".\n";
    }
}

double readMoney(string prompt, bool allowZero = false) {
    string line;
    double value;
    char extra;

    while (true) {
        cout << prompt;
        getline(cin, line);

        stringstream ss(line);

        if (ss >> value && !(ss >> extra) &&
            value >= (allowZero ? 0.0 : 0.01) &&
            value <= 1000000000000.0) {
            return value;
        }

        cout << "Invalid amount! Enter a valid amount.\n";
    }
}

bool confirmAction(string prompt) {
    string answer;

    while (true) {
        cout << prompt << " (Y/N): ";
        getline(cin, answer);

        if (answer == "Y" || answer == "y")
            return true;

        if (answer == "N" || answer == "n")
            return false;

        cout << "Please enter Y or N.\n";
    }
}

// ============================================================
// BANK CLASS
// ============================================================

class Bank {
private:
    vector<Account> accounts;
    vector<Transaction> transactions;

    int findAccountIndex(const string& number) const {
        for (int i = 0; i < (int)accounts.size(); i++) {
            if (accounts[i].getAccountNumber() == number) {
                return i;
            }
        }

        return -1;
    }

    bool saveAccounts() const {
        ofstream file(ACCOUNT_FILE.c_str());

        if (!file) {
            cout << "\nERROR: Could not save account records.\n";
            return false;
        }

        for (const Account& account : accounts) {
            file << account.toFileString() << "\n";
        }

        file.close();

        if (!file) {
            cout << "\nERROR: Problem writing account records.\n";
            return false;
        }

        return true;
    }

    bool saveTransactions() const {
        ofstream file(TRANSACTION_FILE.c_str());

        if (!file) {
            cout << "\nERROR: Could not save transaction history.\n";
            return false;
        }

        for (const Transaction& transaction : transactions) {
            file << transaction.toFileString() << "\n";
        }

        file.close();

        if (!file) {
            cout << "\nERROR: Problem writing transaction history.\n";
            return false;
        }

        return true;
    }

    bool saveAll() const {
        if (!saveAccounts()) return false;
        if (!saveTransactions()) return false;

        return true;
    }

public:
    Bank() {
        loadData();
    }

    // --------------------------------------------------------
    // LOAD DATA
    // --------------------------------------------------------

    void loadData() {
        ifstream accountFile(ACCOUNT_FILE.c_str());

        string line;

        while (getline(accountFile, line)) {
            Account account;

            if (Account::fromFileString(line, account) &&
                findAccountIndex(account.getAccountNumber()) == -1) {
                accounts.push_back(account);
            }
        }

        accountFile.close();

        ifstream transactionFile(TRANSACTION_FILE.c_str());

        while (getline(transactionFile, line)) {
            Transaction transaction;

            if (Transaction::fromFileString(line, transaction)) {
                transactions.push_back(transaction);
            }
        }

        transactionFile.close();

        cout << "\nLoaded " << accounts.size()
             << " account(s) successfully.\n";
    }

    // --------------------------------------------------------
    // CREATE ACCOUNT
    // --------------------------------------------------------

    void createAccount() {
        cout << "\n=============== CREATE BANK ACCOUNT ===============\n";

        string number =
            readAccountNumber("Enter 8-digit account number: ");

        if (findAccountIndex(number) != -1) {
            cout << "\nAn account with this number already exists!\n";
            return;
        }

        string name = readText("Enter customer name: ");

        cout << "\nSelect Account Type:\n";
        cout << "1. Savings\n";
        cout << "2. Current\n";

        int choice = readInt("Enter choice: ", 1, 2);

        string type = (choice == 1) ? "Savings" : "Current";

        double initialBalance =
            readMoney("Enter initial deposit (minimum Rs. 500): ", true);

        if (initialBalance < 500) {
            cout << "\nInitial deposit must be at least Rs. 500.\n";
            return;
        }

        Account newAccount(number, name, type, initialBalance);

        accounts.push_back(newAccount);

        if (saveAccounts()) {
            cout << "\n[SUCCESS] Bank account created successfully!\n";
            newAccount.display();
        }
        else {
            accounts.pop_back();
            cout << "\nAccount creation cancelled because saving failed.\n";
        }
    }

    // --------------------------------------------------------
    // DEPOSIT MONEY
    // --------------------------------------------------------

    void depositMoney() {
        cout << "\n================ DEPOSIT MONEY =================\n";

        string number =
            readAccountNumber("Enter account number: ");

        int index = findAccountIndex(number);

        if (index == -1) {
            cout << "\nAccount not found!\n";
            return;
        }

        accounts[index].display();

        double amount = readMoney("Enter deposit amount: ");

        if (!confirmAction("Confirm this deposit?")) {
            cout << "\nTransaction cancelled.\n";
            return;
        }

        Account original = accounts[index];

        if (!accounts[index].deposit(amount)) {
            cout << "\nDeposit could not be completed.\n";
            return;
        }

        transactions.push_back(
            Transaction(number, "DEPOSIT", amount,
                        accounts[index].getBalance()));

        if (saveAll()) {
            cout << "\n[SUCCESS] Deposit completed!\n";
            cout << "Updated Balance: Rs. "
                 << fixed << setprecision(2)
                 << accounts[index].getBalance() << "\n";
        }
        else {
            accounts[index] = original;
            transactions.pop_back();

            cout << "\nTransaction rolled back because saving failed.\n";
        }
    }

    // --------------------------------------------------------
    // WITHDRAW MONEY
    // --------------------------------------------------------

    void withdrawMoney() {
        cout << "\n================ WITHDRAW MONEY =================\n";

        string number =
            readAccountNumber("Enter account number: ");

        int index = findAccountIndex(number);

        if (index == -1) {
            cout << "\nAccount not found!\n";
            return;
        }

        accounts[index].display();

        double amount = readMoney("Enter withdrawal amount: ");

        if (amount > accounts[index].getBalance()) {
            cout << "\nINSUFFICIENT BALANCE! Withdrawal denied.\n";
            return;
        }

        if (!confirmAction("Confirm this withdrawal?")) {
            cout << "\nTransaction cancelled.\n";
            return;
        }

        Account original = accounts[index];

        if (!accounts[index].withdraw(amount)) {
            cout << "\nWithdrawal could not be completed.\n";
            return;
        }

        transactions.push_back(
            Transaction(number, "WITHDRAW", amount,
                        accounts[index].getBalance()));

        if (saveAll()) {
            cout << "\n[SUCCESS] Withdrawal completed!\n";
            cout << "Remaining Balance: Rs. "
                 << fixed << setprecision(2)
                 << accounts[index].getBalance() << "\n";
        }
        else {
            accounts[index] = original;
            transactions.pop_back();

            cout << "\nTransaction rolled back because saving failed.\n";
        }
    }

    // --------------------------------------------------------
    // CHECK BALANCE
    // --------------------------------------------------------

    void checkBalance() const {
        cout << "\n================ BALANCE INQUIRY =================\n";

        string number =
            readAccountNumber("Enter account number: ");

        int index = findAccountIndex(number);

        if (index == -1) {
            cout << "\nAccount not found!\n";
            return;
        }

        accounts[index].display();
    }

    // --------------------------------------------------------
    // SEARCH ACCOUNT
    // --------------------------------------------------------

    void searchAccount() const {
        cout << "\n================ SEARCH ACCOUNT =================\n";

        string number =
            readAccountNumber("Enter account number: ");

        int index = findAccountIndex(number);

        if (index == -1) {
            cout << "\nAccount not found!\n";
            return;
        }

        cout << "\nAccount found successfully!\n";
        accounts[index].display();
    }

    // --------------------------------------------------------
    // DISPLAY ALL ACCOUNTS
    // --------------------------------------------------------

    void displayAllAccounts() const {
        cout << "\n================ ALL BANK ACCOUNTS ================\n";

        if (accounts.empty()) {
            cout << "No accounts available.\n";
            return;
        }

        cout << left
             << setw(14) << "ACCOUNT NO."
             << setw(24) << "CUSTOMER NAME"
             << setw(14) << "TYPE"
             << setw(16) << "BALANCE" << "\n";

        cout << string(68, '-') << "\n";

        for (const Account& account : accounts) {
            cout << left
                 << setw(14) << account.getAccountNumber()
                 << setw(24) << account.getCustomerName().substr(0, 22)
                 << setw(14) << account.getAccountType()
                 << "Rs. " << fixed << setprecision(2)
                 << account.getBalance() << "\n";
        }

        cout << string(68, '-') << "\n";
        cout << "TOTAL ACCOUNTS: " << accounts.size() << "\n";
    }

    // --------------------------------------------------------
    // TRANSACTION HISTORY
    // --------------------------------------------------------

    void showTransactionHistory() const {
        cout << "\n================ TRANSACTION HISTORY ================\n";

        string number =
            readAccountNumber("Enter account number: ");

        if (findAccountIndex(number) == -1) {
            cout << "\nAccount not found!\n";
            return;
        }

        cout << "\n"
             << left << setw(15) << "TYPE"
             << setw(15) << "AMOUNT"
             << setw(18) << "BALANCE AFTER" << "\n";

        cout << string(48, '-') << "\n";

        int count = 0;

        for (const Transaction& transaction : transactions) {
            if (transaction.getAccountNumber() == number) {
                transaction.display();
                count++;
            }
        }

        if (count == 0) {
            cout << "No transactions recorded for this account.\n";
        }

        cout << "\nTOTAL TRANSACTIONS: " << count << "\n";
    }
};

// ============================================================
// MAIN MENU
// ============================================================

void showMenu() {
    cout << "\n";
    cout << "========================================================\n";
    cout << "               BANK MANAGEMENT SYSTEM                  \n";
    cout << "========================================================\n";
    cout << "              SECURE  |  SIMPLE  |  SMART               \n";
    cout << "========================================================\n";

    cout << "\n  1. Create New Account";
    cout << "\n  2. Deposit Money";
    cout << "\n  3. Withdraw Money";
    cout << "\n  4. Check Account Balance";
    cout << "\n  5. Search Account";
    cout << "\n  6. Display All Accounts";
    cout << "\n  7. View Transaction History";
    cout << "\n  8. Exit";

    cout << "\n\n--------------------------------------------------------\n";
}

// ============================================================
// MAIN FUNCTION
// ============================================================

int main() {
    Bank bank;

    while (true) {
        showMenu();

        int choice = readInt("Enter your choice (1-8): ", 1, 8);

        switch (choice) {
            case 1:
                bank.createAccount();
                break;

            case 2:
                bank.depositMoney();
                break;

            case 3:
                bank.withdrawMoney();
                break;

            case 4:
                bank.checkBalance();
                break;

            case 5:
                bank.searchAccount();
                break;

            case 6:
                bank.displayAllAccounts();
                break;

            case 7:
                bank.showTransactionHistory();
                break;

            case 8:
                cout << "\n========================================================\n";
                cout << "        THANK YOU FOR USING BANK MANAGEMENT SYSTEM      \n";
                cout << "             Your records have been saved.              \n";
                cout << "========================================================\n";
                return 0;
        }

        cout << "\nPress ENTER to return to the main menu...";
        string pause;
        getline(cin, pause);
    }

    return 0;
}
