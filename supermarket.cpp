#include "supermarket.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unordered_set>
#include <thread>
#include <chrono>
#include <limits>
#include <cctype>
#include <climits>

using namespace std;

// ------------------------- util -------------------------
string util::trim(const string &s){
    size_t a = s.find_first_not_of(" \t\r\n");
    size_t b = s.find_last_not_of(" \t\r\n");
    if(a==string::npos) return "";
    return s.substr(a, b - a + 1);
}
string util::xor22(string s){ for(char &c: s) c = char(c ^ 22); return s; }
bool util::to_int(const string &s, int &out){
    try{
        string t = trim(s); size_t p=0;
        long long v = stoll(t, &p);
        if(p!=t.size()) return false;
        if(v < INT_MIN || v > INT_MAX) return false;
        out = (int)v; return true;
    }catch(...){ return false; }
}
void util::clear_screen(){
#ifdef _WIN32
    system("cls");
#else
#endif
}

// ------------------------- InventoryIO -------------------------
bool InventoryIO::load(const string &path, vector<Item> &items){
    items.clear();
    ifstream in(path);
    if(!in){
        seed_default(path);
        in.open(path);
        if(!in) return false;
    }
    int n=0; in>>n; string dummy; getline(in, dummy);
    for(int i=0;i<n;i++){
        string name, type, sq, sp;
        if(!getline(in, name)) return false;
        if(!getline(in, type)) return false;
        if(!getline(in, sq))   return false;
        if(!getline(in, sp))   return false;
        int qty=0, price=0; util::to_int(sq, qty); util::to_int(sp, price);
        items.emplace_back(util::trim(name), util::trim(type), qty, price);
    }
    return true;
}

bool InventoryIO::save(const string &path, const vector<Item> &items){
    ofstream out(path);
    if(!out) return false;
    out << items.size() << "\n";
    for(const auto &it : items){
        out << it.name  << "\n"
            << it.type  << "\n"
            << it.qty   << "\n"
            << it.price << "\n";
    }
    return true;
}

void InventoryIO::seed_default(const string &path){
    vector<Item> items = {
        {"tylenol","med",55,20},
        {"aspirin","med",70,30},
        {"ibuprofen","med",66,50},
        {"multivitamins","med",57,50},
        {"lays-salted","snacks",50,10},
        {"doritos-flamas","snacks",70,10}
    };
    save(path, items);
}

// ------------------------- AccountIO -------------------------
bool AccountIO::load_list(const string &path, vector<Account> &out){
    out.clear();
    ifstream in(path);
    if(!in){
        if(path == "admin.txt") seed_default_admin(path);
        else if(path == "employees.txt") seed_default_employees(path);
        in.open(path);
        if(!in) return false;
    }
    int n=0; in>>n; string dummy; getline(in, dummy);
    for(int i=0;i<n;i++){
        string u, p;
        if(!getline(in, u)) return false;
        if(!getline(in, p)) return false;
        out.push_back(Account{ util::xor22(util::trim(u)), util::xor22(util::trim(p)) });
    }
    return true;
}

bool AccountIO::save_list(const string &path, const vector<Account> &lst){
    ofstream out(path);
    if(!out) return false;
    out << lst.size() << "\n";
    for(const auto &acc : lst){
        out << util::xor22(acc.user) << "\n"
            << util::xor22(acc.pass) << "\n";
    }
    return true;
}

bool AccountIO::append_account(const string &path, const Account &acc){
    vector<Account> all;
    if(!load_list(path, all)) return false;
    all.push_back(acc);
    return save_list(path, all);
}

void AccountIO::seed_default_admin(const string &path){
    vector<Account> admins = { {"afraaz","afraaz123"} };
    save_list(path, admins);
}
void AccountIO::seed_default_employees(const string &path){
    vector<Account> emps = { {"afraaz","afraaz123"} };
    save_list(path, emps);
}

// ------------------------- AdminPanel -------------------------
AdminPanel::AdminPanel(string adminP, string empP, string itemsP)
: adminPath(std::move(adminP)), employeesPath(std::move(empP)), itemsPath(std::move(itemsP)){}

bool AdminPanel::login(){
    vector<Account> admins;
    if(!AccountIO::load_list(adminPath, admins)){
        cout << "admin.txt not found\n";
        return false;
    }
    string u, p;
    cout << "Enter user: "; cin >> u;
    cout << "Enter pass: "; cin >> p;
    for(const auto &a : admins){
        if(a.user == u && a.pass == p) return true;
    }
    cout << "Incorrect user or password.\n";
    return false;
}

void AdminPanel::addItem(){
    vector<Item> inv;
    if(!InventoryIO::load(itemsPath, inv)){
        cout << "items.txt not found\n"; return;
    }
    string name, type; int qty, price;
    cout << "Enter name of the item: "; cin >> ws; getline(cin, name);
    cout << "Enter type of the item: "; getline(cin, type);
    cout << "Enter quantity of the item: "; cin >> qty;
    cout << "Enter price of the item: "; cin >> price;
    inv.emplace_back(name, type, qty, price);
    if(InventoryIO::save(itemsPath, inv)) cout << "Added.\n";
    else cout << "Save failed.\n";
}

void AdminPanel::addEmployee(){
    vector<Account> emps;
    if(!AccountIO::load_list(employeesPath, emps)){
        cout << "employees.txt not found\n"; return;
    }
    string user, pass, cp;
    cout << "Enter new user: "; cin >> user;
    for(const auto &e : emps){ if(e.user == user){ cout << "User already exists.\n"; return; } }
    cout << "Enter password: "; cin >> pass;
    cout << "Confirm password: "; cin >> cp;
    if(pass != cp){ cout << "Passwords dont match.\n"; return; }
    emps.push_back(Account{user, pass});
    if(AccountIO::save_list(employeesPath, emps)) cout << "Added.\n";
    else cout << "Save failed.\n";
}

// ------------------------- POS -------------------------
POS::POS(string empP, string itemsP) : employeesPath(std::move(empP)), itemsPath(std::move(itemsP)) {}

bool POS::loadInventory(){
    if(!InventoryIO::load(itemsPath, items)) return false;

    return InventoryIO::save(itemsPath, items);
}

bool POS::saveInventory(){ return InventoryIO::save(itemsPath, items); }

bool POS::employeeLogin(){
    vector<Account> emps;
    if(!AccountIO::load_list(employeesPath, emps)){
        cout << "File not found\n"; return false;
    }
    cout << emps.size() << " employees\n";
    string u, p; cout << "User: "; cin >> u; cout << "Password: "; cin >> p;
    for(const auto &e : emps){ if(e.user == u && e.pass == p) return true; }
    cout << "Incorrect user or password.\n";
    return false;
}

void POS::listItems() const{
    for(size_t i=0;i<items.size();++i){
        cout << (i+1) << ": " << items[i].name << " price: " << items[i].price << "\n";
    }
}

void POS::billing(){
    if(items.empty()){ cout << "No items loaded.\n"; return; }
    std::vector<int> chosenQty(items.size());
    long long bill = 0; int choice = 1;
    while(choice > 0 && choice <= (int)items.size()){
        listItems();
        cout << "Enter item number or any invalid number to exit: ";
        if(!(cin >> choice)){ cin.clear(); string junk; getline(cin, junk); break; }
        if(choice <= 0 || choice > (int)items.size()) break;
        int q; cout << "Enter qty: "; cin >> q;
        auto &it = items[choice-1];
        if(it.qty >= q){
            bill += 1LL * q * it.price;
            cout << "Item added.\n";
            chosenQty[choice-1] += q;
            it.qty -= q;
        } else {
            cout << "This quantity not available.\n";
        }
        cout << "\n";
    }
    cout << "\nYour Bill:\n";
    for(size_t i=0;i<items.size();++i){
        if(chosenQty[i] > 0){
            cout << chosenQty[i] << "  " << items[i].name << ": " << (1LL*chosenQty[i]*items[i].price) << "\n";
        }
    }
    cout << "Total: " << bill << "\n";
    long long amount=0; do{ cout << "Enter amount paid: "; cin >> amount; } while(amount < bill);
    if(amount == bill) cout << "Paid.\n"; else cout << "Paid... change: " << (amount - bill) << "\n";
    saveInventory();
}

void POS::reorder(){
    const int reorderlvl = 50; bool any=false;
    for(auto &it : items){
        if(it.qty <= reorderlvl){
            it.qty += 50;
            cout << "ordering " << it.name << " from supplier....\n";
            any=true;
        }
    }
    if(any) saveInventory();
}

// ------------------------- main -------------------------
int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    while(true){
        cout << "\nSUPER MARKET SYSTEM (C++)\n";
        cout << "1) Admin\n2) Employee (POS)\n0) Exit\nChoice: ";
        int top; if(!(cin >> top)) return 0; util::clear_screen();
        if(top == 0) break;
        if(top == 1){
            AdminPanel ap;
            while(!ap.login()){ }
            cout << "Hello Admin.\n";
            int ch=0; do{
                cout << "Enter 1 to add new items to stock.\n";
                cout << "Enter 2 to add a new employee.\n";
                cout << "Enter 3 to sign out.\nChoice: ";
                cin >> ch; util::clear_screen();
                if(ch==1) ap.addItem();
                else if(ch==2) ap.addEmployee();
            }while(ch!=3);
        } else if(top == 2){
            POS pos;
            while(!pos.employeeLogin()){ }
            cout << "Logged in.\n";
            std::this_thread::sleep_for(std::chrono::seconds(1));
            util::clear_screen();
            if(!pos.loadInventory()){ cout << "Failed to load items.txt\n"; continue; }
            while(true){
                pos.billing();
                cout << "Checking for need of reordering from supplier and updating...\n";
                pos.reorder();
                cout << "Next order? (enter 1 for more orders or 0 to sign out)\n";
                int cont=0; cin >> cont; if(cont != 1) break; util::clear_screen();
            }
            cout << "Signing out...\n";
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        util::clear_screen();
    }
    cout << "Goodbye!\n";
    return 0;
}
