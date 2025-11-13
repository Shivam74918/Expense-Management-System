#include <iostream>
#include <vector>
#include <unordered_map>
#include <stack>
#include <string>
#include <iomanip>
#include <algorithm>
#include <ctime>

using namespace std;

// ============= DATE STRUCTURE =============
struct Date {
    int day, month, year;
    
    bool operator<=(const Date& other) const {
        if (year != other.year) return year < other.year;
        if (month != other.month) return month < other.month;
        return day <= other.day;
    }
    
    bool operator>=(const Date& other) const {
        return other <= *this;
    }
};

// ============= TRANSACTION STRUCTURE =============
struct Transaction {
    int id;
    Date date;
    string category;
    double amount;
    string description;
    string type; // "Income" or "Expense"
    
    // For sorting
    bool operator<(const Transaction& other) const {
        return amount > other.amount; // Descending order for top expenses
    }
};

// ============= UNDO OPERATION STRUCTURE =============
enum OpType { ADD, DELETE_OP };

struct UndoOp {
    OpType op;
    Transaction data;
};

// ============= EXPENSE MANAGER CLASS =============
class ExpenseManager {
private:
    vector<Transaction> transactions;                          // Array for all transactions
    unordered_map<string, vector<Transaction>> categoryMap;    // Hash Map for categories
    stack<UndoOp> undoStack;                                  // Stack for undo operations
    int nextId;

public:
    ExpenseManager() : nextId(1) {}

    // ===== 1. ADD TRANSACTION =====
    // Time Complexity: O(1) - Array append + Hash map insert
    void addTransaction(const Date& date, const string& category, double amount, 
                       const string& desc, const string& type) {
        Transaction t = {nextId++, date, category, amount, desc, type};
        transactions.push_back(t);
        categoryMap[category].push_back(t);
        undoStack.push({ADD, t});
        cout << "✓ Transaction added (ID: " << t.id << ")\n";
    }

    // ===== 2. DELETE TRANSACTION =====
    // Time Complexity: O(n) - Linear search + removal
    bool deleteTransaction(int id) {
        for (size_t i = 0; i < transactions.size(); ++i) {
            if (transactions[i].id == id) {
                UndoOp uop = {DELETE_OP, transactions[i]};
                string category = transactions[i].category;
                
                // Remove from categoryMap using lambda
                auto& catTransactions = categoryMap[category];
                catTransactions.erase(
                    remove_if(catTransactions.begin(), catTransactions.end(),
                             [id](const Transaction& t) { return t.id == id; }),
                    catTransactions.end()
                );
                
                // Remove from main transactions list
                transactions.erase(transactions.begin() + i);
                undoStack.push(uop);
                
                cout << "✓ Transaction (ID: " << id << ") deleted.\n";
                return true;
            }
        }
        cout << "✗ Transaction ID not found.\n";
        return false;
    }

    // ===== 3. UNDO LAST OPERATION =====
    // Time Complexity: O(1) for stack pop + O(n) for removal
    void undo() {
        if (undoStack.empty()) {
            cout << "✗ No operation to undo.\n";
            return;
        }
        
        UndoOp uop = undoStack.top();
        undoStack.pop();
        
        if (uop.op == ADD) {
            // Undo add by removing
            transactions.erase(
                remove_if(transactions.begin(), transactions.end(),
                         [uop](const Transaction& t) { return t.id == uop.data.id; }),
                transactions.end()
            );
            
            auto& catTransactions = categoryMap[uop.data.category];
            catTransactions.erase(
                remove_if(catTransactions.begin(), catTransactions.end(),
                         [uop](const Transaction& t) { return t.id == uop.data.id; }),
                catTransactions.end()
            );
            cout << "✓ Undo performed: Transaction added is now removed.\n";
        } 
        else if (uop.op == DELETE_OP) {
            // Undo delete by re-adding
            transactions.push_back(uop.data);
            categoryMap[uop.data.category].push_back(uop.data);
            cout << "✓ Undo performed: Transaction deleted is now restored.\n";
        }
    }

    // ===== 4. GET TRANSACTIONS BY CATEGORY =====
    // Time Complexity: O(1) hash lookup + O(k) iteration
    void showByCategory(const string& category) const {
        if (categoryMap.find(category) == categoryMap.end() || categoryMap.at(category).empty()) {
            cout << "✗ No transactions in category: " << category << "\n";
            return;
        }
        
        cout << "\n" << string(60, '=') << "\n";
        cout << "TRANSACTIONS IN CATEGORY: " << category << "\n";
        cout << string(60, '=') << "\n";
        cout << left << setw(5) << "ID" << setw(12) << "Date" 
             << setw(10) << "Amount" << "Description\n";
        cout << string(40, '-') << "\n";
        
        cout << fixed << setprecision(2);
        for (const auto& t : categoryMap.at(category)) {
            cout << left << setw(5) << t.id 
                 << setw(12) << (to_string(t.date.day) + "/" + to_string(t.date.month) + "/" + to_string(t.date.year))
                 << setw(10) << "₹" + to_string(t.amount)
                 << t.description << "\n";
        }
        cout << "\n";
    }

    // ===== 5. DISPLAY ALL TRANSACTIONS =====
    // Time Complexity: O(n)
    void showAll() const {
        if (transactions.empty()) {
            cout << "✗ No transactions.\n";
            return;
        }
        
        cout << "\n" << string(85, '=') << "\n";
        cout << "ALL TRANSACTIONS\n";
        cout << string(85, '=') << "\n";
        cout << left << setw(5) << "ID" << setw(12) << "Date" 
             << setw(15) << "Category" << setw(10) << "Amount" 
             << setw(20) << "Description" << "Type\n";
        cout << string(72, '-') << "\n";
        
        cout << fixed << setprecision(2);
        for (const auto& t : transactions) {
            cout << left << setw(5) << t.id 
                 << setw(12) << (to_string(t.date.day) + "/" + to_string(t.date.month) + "/" + to_string(t.date.year))
                 << setw(15) << t.category 
                 << setw(10) << "₹" + to_string(t.amount)
                 << setw(20) << t.description 
                 << t.type << "\n";
        }
        cout << "\n";
    }

    // ===== 6. CALCULATE MONTHLY TOTAL =====
    // Time Complexity: O(n)
    double getMonthlyTotal(int month, int year, const string& type = "") const {
        double total = 0;
        for (const auto& t : transactions) {
            if (t.date.month == month && t.date.year == year) {
                if (type.empty() || t.type == type) {
                    total += t.amount;
                }
            }
        }
        return total;
    }

    // ===== 7. GET CATEGORY SUMMARY =====
    // Time Complexity: O(n)
    void showCategorySummary() const {
        cout << "\n" << string(50, '=') << "\n";
        cout << "CATEGORY SUMMARY\n";
        cout << string(50, '=') << "\n";
        cout << left << setw(20) << "Category" << "Total Amount\n";
        cout << string(35, '-') << "\n";
        
        cout << fixed << setprecision(2);
        for (const auto& pair : categoryMap) {
            double total = 0;
            for (const auto& t : pair.second) {
                if (t.type == "Expense") {
                    total += t.amount;
                }
            }
            cout << left << setw(20) << pair.first << "₹" << total << "\n";
        }
        cout << "\n";
    }

    // ===== 8. SEARCH BY DATE RANGE =====
    // Time Complexity: O(n) linear search
    // Could be optimized to O(log n) with binary search if sorted
    void searchByDateRange(const Date& start, const Date& end) const {
        cout << "\n" << string(60, '=') << "\n";
        cout << "TRANSACTIONS IN DATE RANGE\n";
        cout << string(60, '=') << "\n";
        cout << left << setw(12) << "Date" << setw(15) << "Category" 
             << setw(10) << "Amount" << "Description\n";
        cout << string(50, '-') << "\n";
        
        cout << fixed << setprecision(2);
        bool found = false;
        for (const auto& t : transactions) {
            if (t.date >= start && t.date <= end) {
                cout << left << setw(12) << (to_string(t.date.day) + "/" + to_string(t.date.month) + "/" + to_string(t.date.year))
                     << setw(15) << t.category 
                     << setw(10) << "₹" + to_string(t.amount)
                     << t.description << "\n";
                found = true;
            }
        }
        if (!found) {
            cout << "No transactions found in this date range.\n";
        }
        cout << "\n";
    }

    // ===== 9. GET TOP EXPENSES =====
    // Time Complexity: O(n log n) for sorting
    void showTopExpenses(int n = 5) const {
        vector<Transaction> expenses;
        for (const auto& t : transactions) {
            if (t.type == "Expense") {
                expenses.push_back(t);
            }
        }
        
        if (expenses.empty()) {
            cout << "✗ No expenses found.\n";
            return;
        }
        
        // Sort by amount (descending) using Quick Sort - O(n log n)
        sort(expenses.begin(), expenses.end(), 
             [](const Transaction& a, const Transaction& b) {
                 return a.amount > b.amount;
             });
        
        cout << "\n" << string(60, '=') << "\n";
        cout << "TOP " << min(n, (int)expenses.size()) << " EXPENSES\n";
        cout << string(60, '=') << "\n";
        cout << left << setw(5) << "Rank" << setw(15) << "Category" 
             << setw(10) << "Amount" << "Description\n";
        cout << string(45, '-') << "\n";
        
        cout << fixed << setprecision(2);
        for (int i = 0; i < min(n, (int)expenses.size()); ++i) {
            cout << left << setw(5) << (i + 1)
                 << setw(15) << expenses[i].category 
                 << setw(10) << "₹" + to_string(expenses[i].amount)
                 << expenses[i].description << "\n";
        }
        cout << "\n";
    }

    // ===== 10. SEARCH BY AMOUNT RANGE =====
    // Time Complexity: O(n)
    void searchByAmountRange(double minAmount, double maxAmount) const {
        cout << "\n" << string(60, '=') << "\n";
        cout << "TRANSACTIONS IN AMOUNT RANGE: ₹" << minAmount << " - ₹" << maxAmount << "\n";
        cout << string(60, '=') << "\n";
        cout << left << setw(5) << "ID" << setw(15) << "Category" 
             << setw(10) << "Amount" << "Description\n";
        cout << string(45, '-') << "\n";
        
        cout << fixed << setprecision(2);
        bool found = false;
        for (const auto& t : transactions) {
            if (t.amount >= minAmount && t.amount <= maxAmount) {
                cout << left << setw(5) << t.id
                     << setw(15) << t.category 
                     << setw(10) << "₹" + to_string(t.amount)
                     << t.description << "\n";
                found = true;
            }
        }
        if (!found) {
            cout << "No transactions found in this amount range.\n";
        }
        cout << "\n";
    }

    // ===== 11. SEARCH BY KEYWORD =====
    // Time Complexity: O(n)
    void searchByKeyword(const string& keyword) const {
        cout << "\n" << string(60, '=') << "\n";
        cout << "SEARCH RESULTS FOR: \"" << keyword << "\"\n";
        cout << string(60, '=') << "\n";
        cout << left << setw(5) << "ID" << setw(15) << "Category" 
             << setw(10) << "Amount" << "Description\n";
        cout << string(45, '-') << "\n";
        
        cout << fixed << setprecision(2);
        bool found = false;
        for (const auto& t : transactions) {
            // Case-insensitive search
            if (t.description.find(keyword) != string::npos) {
                cout << left << setw(5) << t.id
                     << setw(15) << t.category 
                     << setw(10) << "₹" + to_string(t.amount)
                     << t.description << "\n";
                found = true;
            }
        }
        if (!found) {
            cout << "No transactions found with keyword: " << keyword << "\n";
        }
        cout << "\n";
    }

    // ===== 12. GET TOTAL INCOME =====
    // Time Complexity: O(n)
    double getTotalIncome() const {
        double total = 0;
        for (const auto& t : transactions) {
            if (t.type == "Income") {
                total += t.amount;
            }
        }
        return total;
    }

    // ===== 13. GET TOTAL EXPENSES =====
    // Time Complexity: O(n)
    double getTotalExpenses() const {
        double total = 0;
        for (const auto& t : transactions) {
            if (t.type == "Expense") {
                total += t.amount;
            }
        }
        return total;
    }

    // ===== 14. GET TRANSACTION COUNT =====
    int getTransactionCount() const {
        return transactions.size();
    }

    // ===== 15. DISPLAY STATISTICS =====
    void showStatistics() const {
        cout << "\n" << string(60, '=') << "\n";
        cout << "STATISTICS\n";
        cout << string(60, '=') << "\n";
        cout << fixed << setprecision(2);
        cout << "Total Transactions: " << getTransactionCount() << "\n";
        cout << "Total Income: ₹" << getTotalIncome() << "\n";
        cout << "Total Expenses: ₹" << getTotalExpenses() << "\n";
        cout << "Net Balance: ₹" << (getTotalIncome() - getTotalExpenses()) << "\n";
        cout << "Categories: " << categoryMap.size() << "\n";
        cout << "\n";
    }
};

// ============= HELPER FUNCTION =============
Date makeDate(int day, int month, int year) {
    return {day, month, year};
}

// ============= MAIN DEMO =============
int main() {
    ExpenseManager manager;

    cout << "\n";
    cout << "╔════════════════════════════════════════════════════╗\n";
    cout << "║   EXPENSE MANAGEMENT SYSTEM (C++)                 ║\n";
    cout << "║   Demonstrating DSA: Hash Map, Stack, Sorting     ║\n";
    cout << "╚════════════════════════════════════════════════════╝\n";

    // ===== ADD SAMPLE TRANSACTIONS =====
    cout << "\n--- ADDING TRANSACTIONS ---\n";
    manager.addTransaction(makeDate(1, 11, 2025), "Food", 250.50, "Lunch at Café", "Expense");
    manager.addTransaction(makeDate(4, 11, 2025), "Transport", 100, "Uber Ride", "Expense");
    manager.addTransaction(makeDate(7, 11, 2025), "Food", 650, "Groceries", "Expense");
    manager.addTransaction(makeDate(10, 11, 2025), "Entertainment", 500, "Movie Tickets", "Expense");
    manager.addTransaction(makeDate(12, 11, 2025), "Utilities", 1500, "Electricity Bill", "Expense");
    manager.addTransaction(makeDate(15, 11, 2025), "Salary", 20000, "November salary", "Income");

    // ===== DISPLAY ALL =====
    manager.showAll();

    // ===== STATISTICS =====
    manager.showStatistics();

    // ===== HASH MAP: SHOW BY CATEGORY O(1) =====
    manager.showByCategory("Food");

    // ===== AGGREGATION: CATEGORY SUMMARY =====
    manager.showCategorySummary();

    // ===== SORTING: TOP EXPENSES O(n log n) =====
    manager.showTopExpenses(3);

    // ===== SEARCH: DATE RANGE =====
    manager.searchByDateRange(makeDate(5, 11, 2025), makeDate(12, 11, 2025));

    // ===== SEARCH: AMOUNT RANGE =====
    manager.searchByAmountRange(100, 700);

    // ===== SEARCH: KEYWORD =====
    manager.searchByKeyword("Food");

    // ===== MONTHLY TOTAL =====
    double nov_total = manager.getMonthlyTotal(11, 2025, "Expense");
    cout << "\n" << string(60, '=') << "\n";
    cout << "Total Expenses in November 2025: ₹" << fixed << setprecision(2) << nov_total << "\n";
    cout << string(60, '=') << "\n";

    // ===== UNDO: STACK IMPLEMENTATION =====
    cout << "\n--- TESTING UNDO FUNCTIONALITY (STACK) ---\n";
    manager.undo();
    manager.showAll();

    cout << "\n" << string(60, '=') << "\n";
    cout << "Demo Complete!\n";
    cout << string(60, '=') << "\n\n";

    return 0;
}