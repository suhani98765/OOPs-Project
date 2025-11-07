// vehicle_rental.cpp
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <memory>    // smart pointers if desired
#include <algorithm>

using namespace std;

// ===========================
// 1) BASE CLASS: Vehicle
// ===========================
class Vehicle {
protected:
    int id;
    string model;
    double baseRatePerDay; // base rent per day
    int quantity;          // how many vehicles of this model available

public:
    Vehicle(int id=0, const string& model="", double rate=0.0, int qty=0)
        : id(id), model(model), baseRatePerDay(rate), quantity(qty) {}

    virtual ~Vehicle() {}

    virtual void display() const {
        cout << left << setw(6) << id << setw(18) << model
             << setw(12) << baseRatePerDay << setw(6) << quantity << endl;
    }

    int getID() const { return id; }
    string getModel() const { return model; }
    double getBaseRate() const { return baseRatePerDay; }
    int getQuantity() const { return quantity; }
    void updateQuantity(int q) { quantity = q; }

    // Virtual members - different vehicle types may override
    virtual double rentalRatePerDay() const = 0; // may include fees-specific to type
    virtual double calculateTaxPerDay() const = 0;
};

// ===========================
// 2) DERIVED CLASSES
// ===========================
class Car : public Vehicle {
public:
    Car(int id, const string& model, double rate, int qty)
        : Vehicle(id, model, rate, qty) {}

    double rentalRatePerDay() const override {
        // cars may have extra service charge added (example)
        return baseRatePerDay + 200.0; // fixed service/insurance per day example
    }

    double calculateTaxPerDay() const override {
        return rentalRatePerDay() * 0.12; // 12% tax
    }
};

class Bike : public Vehicle {
public:
    Bike(int id, const string& model, double rate, int qty)
        : Vehicle(id, model, rate, qty) {}

    double rentalRatePerDay() const override {
        return baseRatePerDay; // bikes simpler
    }

    double calculateTaxPerDay() const override {
        return rentalRatePerDay() * 0.05; // 5% tax
    }
};

class Truck : public Vehicle {
public:
    Truck(int id, const string& model, double rate, int qty)
        : Vehicle(id, model, rate, qty) {}

    double rentalRatePerDay() const override {
        return baseRatePerDay + 500.0; // heavy vehicle surcharge
    }

    double calculateTaxPerDay() const override {
        return rentalRatePerDay() * 0.18; // 18% tax
    }
};

// ===========================
// 3) OWNER / AGENCY CLASS
// ===========================
class Owner {
    string code;
    string name;
public:
    Owner(string code="", string name="") : code(code), name(name) {}

    void display() const {
        cout << "Owner: " << name << " (" << code << ")\n";
    }

    string getCode() const { return code; }
};

// ===========================
// 4) FLEET: inventory manager
//    (dynamic memory allocation, overloaded search)
// ===========================
class Fleet {
    vector<Vehicle*> vehicles; // we manage memory manually here for teaching purpose

public:
    Fleet() {}
    ~Fleet() {
        for (auto v : vehicles) delete v;
    }

    void addVehicle(Vehicle* v) {
        vehicles.push_back(v);
    }

    // Remove model by id (dynamic removal)
    bool removeVehicleByID(int id) {
        auto it = find_if(vehicles.begin(), vehicles.end(),
                          [id](Vehicle* v){ return v->getID() == id; });
        if (it != vehicles.end()) {
            delete *it;
            vehicles.erase(it);
            return true;
        }
        return false;
    }

    // Function overloading: search by id or by model name
    Vehicle* search(int id) {
        for (auto v : vehicles)
            if (v->getID() == id) return v;
        return nullptr;
    }

    Vehicle* search(const string& modelName) {
        for (auto v : vehicles)
            if (v->getModel() == modelName) return v;
        return nullptr;
    }

    void displayAll() const {
        cout << "\n---- FLEET ----\n";
        cout << left << setw(6) << "ID" << setw(18) << "Model"
             << setw(12) << "BaseRate" << setw(6) << "Qty" << endl;
        for (auto v : vehicles) v->display();
    }
};

// ===========================
// 5) RENTAL AGREEMENT (Bill)
//    Operator overloading, static count, file handling
// ===========================
struct RentSelection {
    Vehicle* vehicle;
    int qty;
    int days;
    RentSelection(Vehicle* v=nullptr,int q=0,int d=0):vehicle(v),qty(q),days(d){}
};

class RentalItem {
public:
    string model;
    int qty;
    int days;
    double ratePerDay;
    double taxPerDay;
    double total;

    RentalItem(const string& m, int q, int d, double r, double t)
        : model(m), qty(q), days(d), ratePerDay(r), taxPerDay(t) {
        total = (ratePerDay + taxPerDay) * qty * days;
    }
};

class RentalAgreement {
    vector<RentalItem> items;
    double grandTotal;
    static int totalRentals; // static counter for how many times vehicles added (or agreements created)

public:
    RentalAgreement():grandTotal(0.0){}

    // Operator overloading: add a RentSelection to the agreement
    RentalAgreement& operator+(const RentSelection& sel) {
        if (!sel.vehicle) throw runtime_error("Null vehicle passed to rental.");
        double rate = sel.vehicle->rentalRatePerDay();
        double tax  = sel.vehicle->calculateTaxPerDay();
        RentalItem it(sel.vehicle->getModel(), sel.qty, sel.days, rate, tax);
        items.push_back(it);
        grandTotal += it.total;
        totalRentals++;
        return *this;
    }

    void displayAgreement() const {
        cout << "\n--- RENTAL AGREEMENT ---\n";
        cout << left << setw(18) << "Model" << setw(6) << "Qty" << setw(6) << "Days"
             << setw(12) << "Rate/day" << setw(10) << "Tax/day" << setw(12) << "Total" << endl;
        for (auto &it : items) {
            cout << left << setw(18) << it.model << setw(6) << it.qty << setw(6) << it.days
                 << setw(12) << it.ratePerDay << setw(10) << it.taxPerDay << setw(12) << it.total << endl;
        }
        cout << fixed << setprecision(2) << "\nGrand Total: Rs. " << grandTotal << "\n";
    }

    void saveInvoice(const string& filename, const Owner& owner) {
        ofstream fout(filename, ios::app);
        if (!fout) throw runtime_error("Cannot open invoice file.");
        fout << "===== RENTAL INVOICE =====\n";
        fout << "Owner: " << owner.getCode() << "\n";
        for (auto &it : items) {
            fout << it.model << " x" << it.qty << " for " << it.days
                 << " day(s) -> Rs." << it.total << "\n";
        }
        fout << "Grand Total: Rs. " << grandTotal << "\n";
        fout << "==========================\n\n";
        fout.close();
    }

    static void showTotalRentals() {
        cout << "Total rental operations performed: " << totalRentals << endl;
    }
};
int RentalAgreement::totalRentals = 0;

// ===========================
// 6) TEMPLATE: generic rental generator/saver
// ===========================
template<typename T>
class RentalGenerator {
public:
    void generate(T& agreement, const string& file, const Owner& owner) {
        // We expect T to have saveInvoice method; no static check here.
        agreement.saveInvoice(file, owner);
    }
};

// ===========================
// 7) Helper: validate owner codes (throws exception if invalid)
// ===========================
void validateOwnerCode(const Owner& owner) {
    if (owner.getCode().empty() || owner.getCode().size() < 3)
        throw invalid_argument("Invalid owner code provided.");
}

// ===========================
// 8) MAIN: interactive demo
// ===========================
int main() {
    try {
        Fleet fleet;
        // Populate fleet (dynamic allocation)
        fleet.addVehicle(new Car(101, "Toyota-Innova", 3000, 3));
        fleet.addVehicle(new Car(102, "Honda-City", 2500, 4));
        fleet.addVehicle(new Bike(201, "Royal-Enfield", 800, 5));
        fleet.addVehicle(new Bike(202, "Honda-Activa", 400, 10));
        fleet.addVehicle(new Truck(301, "Tata-407", 5000, 2));

        Owner owner("OWN001", "FastRentals");
        // validate owner (demonstrates exception handling)
        validateOwnerCode(owner);

        RentalAgreement agreement;
        RentalGenerator<RentalAgreement> generator;

        int choice = -1;
        do {
            cout << "\n=== VEHICLE RENTAL SYSTEM ===\n";
            cout << "1. Show Fleet\n";
            cout << "2. Search by ID\n";
            cout << "3. Search by Model\n";
            cout << "4. Add Rental (choose vehicle, qty, days)\n";
            cout << "5. Remove Vehicle Model from Fleet\n";
            cout << "6. Show Current Agreement\n";
            cout << "7. Save Agreement to File\n";
            cout << "8. Show Total Rentals (static)\n";
            cout << "0. Exit\n";
            cout << "Enter choice: ";
            if (!(cin >> choice)) { cin.clear(); cin.ignore(10000,'\n'); cout<<"Invalid input\n"; continue; }

            if (choice == 1) {
                fleet.displayAll();
            }
            else if (choice == 2) {
                int id; cout << "Enter vehicle ID: "; cin >> id;
                Vehicle* v = fleet.search(id);
                if (v) v->display();
                else cout << "Vehicle not found.\n";
            }
            else if (choice == 3) {
                cout << "Enter model name (exact): ";
                string name; cin.ignore(); getline(cin, name);
                Vehicle* v = fleet.search(name);
                if (v) v->display();
                else cout << "Vehicle not found.\n";
            }
            else if (choice == 4) {
                int id; cout << "Enter vehicle ID to rent: "; cin >> id;
                Vehicle* v = fleet.search(id);
                if (!v) { cout << "Invalid vehicle ID.\n"; continue; }

                int qty; cout << "Enter quantity (number of vehicles): "; cin >> qty;
                int days; cout << "Enter number of days: "; cin >> days;

                if (qty <= 0 || days <= 0) {
                    cout << "Quantity and days must be positive.\n";
                    continue;
                }
                if (qty > v->getQuantity()) {
                    cout << "Requested quantity not available. Available: " << v->getQuantity() << "\n";
                    continue;
                }

                // Create selection and use operator+ to add to agreement
                RentSelection sel(v, qty, days);
                agreement + sel; // operator +

                // Deduct from fleet
                v->updateQuantity(v->getQuantity() - qty);
                cout << "Added to rental agreement: " << qty << " x " << v->getModel() << " for " << days << " day(s).\n";
            }
            else if (choice == 5) {
                int id; cout << "Enter vehicle ID to remove from fleet: "; cin >> id;
                if (fleet.removeVehicleByID(id)) cout << "Removed from fleet.\n";
                else cout << "Vehicle ID not found.\n";
            }
            else if (choice == 6) {
                agreement.displayAgreement();
            }
            else if (choice == 7) {
                string fname = "rentals.txt";
                generator.generate(agreement, fname, owner);
                cout << "Agreement saved to " << fname << endl;
            }
            else if (choice == 8) {
                RentalAgreement::showTotalRentals();
            }
            else if (choice == 0) {
                cout << "Exiting. Goodbye!\n";
            }
            else {
                cout << "Invalid choice\n";
            }
        } while (choice != 0);
    }
    catch (const invalid_argument& ia) {
        cerr << "Validation error: " << ia.what() << endl;
    }
    catch (const exception& ex) {
        cerr << "Error: " << ex.what() << endl;
    }

    return 0;
}
