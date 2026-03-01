#pragma once

#include <string>
#include <vector>

namespace util {
    std::string trim(const std::string& s);
    std::string xor22(std::string s);
    bool to_int(const std::string& s, int& out);
    void clear_screen();
}

struct Item {
    std::string name;
    std::string type;
    int qty;
    int price;
    Item() : qty(0), price(0) {}
    Item(std::string n, std::string t, int q, int p)
        : name(std::move(n)), type(std::move(t)), qty(q), price(p) {}
};

struct Account {
    std::string user;
    std::string pass;
};

class InventoryIO {
public:
    static bool load(const std::string& path, std::vector<Item>& items);
    static bool save(const std::string& path, const std::vector<Item>& items);
    static void seed_default(const std::string& path); // create default items.txt
};

class AccountIO {
public:
    static bool load_list(const std::string& path, std::vector<Account>& out); // decoded in memory
    static bool save_list(const std::string& path, const std::vector<Account>& lst); // encodes to file
    static bool append_account(const std::string& path, const Account& acc);
    static void seed_default_admin(const std::string& path);     // create admin.txt
    static void seed_default_employees(const std::string& path); // create employees.txt
};

namespace migrate {
    void applyItemRenames(std::vector<Item>& items);
    void applyUserRenames(std::vector<Account>& admins, std::vector<Account>& employees);
}

class AdminPanel {
    std::string adminPath, employeesPath, itemsPath;
public:
    AdminPanel(std::string adminP = "admin.txt",
               std::string empP   = "employees.txt",
               std::string itemsP = "items.txt");

    bool login();
    void addItem();
    void addEmployee();
};

class POS {
    std::string employeesPath, itemsPath;
    std::vector<Item> items;
public:
    POS(std::string empP = "employees.txt", std::string itemsP = "items.txt");

    bool loadInventory();
    bool saveInventory();
    bool employeeLogin();
    void listItems() const;
    void billing();
    void reorder(); // if qty <= 50 -> +50
};

int main();
