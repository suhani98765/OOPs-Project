// vehicle_rental.cpp
#include<iostream>
#include<string>
#include<fstream>
using namespace std;

// ============ Base Class ============
class Vehicle {
protected:
    int id;
    string model;
    double rate;
    int quantity;

public:
    Vehicle(int id = 0, string model = "", double rate = 0, int qty = 0)
        : id(id), model(model), rate(rate), quantity(qty) {}

    virtual ~Vehicle() {}

    virtual void display() {
        cout << "\nID: " << id 
             << "\nModel: " << model
             << "\nBase Rate/day: " << rate
             << "\nQuantity: " << quantity << endl;
    }

    int getID() { return id; }
    string getModel() { return model; }
    int getQty() { return quantity; }
    void updateQty(int q) { quantity = q; }

    virtual double rentalRate() = 0;
    virtual double taxRate() = 0;
};

// ============ Derived Classes ============
class Car : public Vehicle {
public:
    Car(int i, string m, double r, int q) : Vehicle(i, m, r, q) {}

    double rentalRate() { return rate + 200; }
    double taxRate() { return rentalRate() * 0.12; }
};

class Bike : public Vehicle {
public:
    Bike(int i, string m, double r, int q) : Vehicle(i, m, r, q) {}

    double rentalRate() { return rate; }
    double taxRate() { return rentalRate() * 0.05; }
};

class Truck : public Vehicle {
public:
    Truck(int i, string m, double r, int q) : Vehicle(i, m, r, q) {}

    double rentalRate() { return rate + 500; }
    double taxRate() { return rentalRate() * 0.18; }
};

// ============ Fleet ============
class Fleet {
    Vehicle* list[20];
    int count;

public:
    Fleet() { count = 0; }

    void add(Vehicle* v) {
        list[count++] = v;
    }

    Vehicle* search(int id) {
        for (int i = 0; i < count; i++)
            if (list[i]->getID() == id)
                return list[i];
        return NULL;
    }

    void display() {
        for (int i = 0; i < count; i++)
            list[i]->display();
    }
};

// ============ Rental Agreement ============
class RentalAgreement {
    struct Item {
        string model;
        int qty;
        int days;
        double totaling;
    } items[30];

    int c;
    double total;

public:
    RentalAgreement() {
        c = 0;
        total = 0;
    }

    void add(Vehicle* v, int qty, int days) {
        double rate = v->rentalRate();
        double tax = v->taxRate();
        double sum = (rate + tax) * qty * days;

        items[c].model = v->getModel();
        items[c].qty = qty;
        items[c].days = days;
        items[c].totaling = sum;
        c++;

        total += sum;

        v->updateQty(v->getQty() - qty);

        cout << "\nAdded " << qty << " x " << v->getModel()
             << " for " << days << " day(s)\n";
    }

    void display() {
        cout << "\n---- RENTAL AGREEMENT ----\n";
        for (int i = 0; i < c; i++) {
            cout << items[i].model << " x " << items[i].qty
                 << " for " << items[i].days
                 << " day(s). Amount = " << items[i].totaling << endl;
        }
        cout << "\nGrand Total: " << total << endl;
    }

    void save() {
        ofstream fout("rentals.txt", ios::app);
        fout << "==== RENTAL INVOICE ====\n";
        for (int i = 0; i < c; i++) {
            fout << items[i].model << " x " << items[i].qty
                 << " days=" << items[i].days
                 << " => Rs." << items[i].totaling << "\n";
        }
        fout << "Grand Total: " << total << "\n\n";
        fout.close();
        cout << "\nInvoice saved.\n";
    }
};

// ============ MAIN ============
int main() {

    Fleet fleet;
    fleet.add(new Car(101, "Toyota-Innova", 3000, 3));
    fleet.add(new Car(102, "Honda-City", 2500, 4));
    fleet.add(new Bike(201, "Royal-Enfield", 800, 5));
    fleet.add(new Bike(202, "Honda-Activa", 400, 10));
    fleet.add(new Truck(301, "Tata-407", 5000, 2));

    RentalAgreement ag;
    int ch;

    do {
        cout << "\n==== VEHICLE RENTAL MENU ====\n";
        cout << "1. Show Fleet\n2. Search by ID\n3. Add Rental\n4. Show Agreement\n5. Save Invoice\n0. Exit\n";
        cout << "\nEnter choice: ";
        cin >> ch;

        if (ch == 1) fleet.display();
        else if (ch == 2) {
            int id;
            cout << "Enter ID: ";
            cin >> id;
            Vehicle* v = fleet.search(id);
            if (v) v->display();
            else cout << "\nNot Found.\n";
        }
        else if (ch == 3) {
            int id, q, d;
            cout << "Enter Vehicle ID: ";
            cin >> id;
            Vehicle* v = fleet.search(id);
            if (!v) {
                cout << "Invalid ID\n";
                continue;
            }
            cout << "Enter quantity: ";
            cin >> q;
            if (q > v->getQty()) {
                cout << "Not enough vehicles.\n";
                continue;
            }
            cout << "Enter days: ";
            cin >> d;

            ag.add(v, q, d);
        }
        else if (ch == 4) ag.display();
        else if (ch == 5) ag.save();

    } while (ch != 0);

    return 0;
}
