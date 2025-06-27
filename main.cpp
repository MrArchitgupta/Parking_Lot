#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cmath>
using namespace std;

// ==================== VEHICLE ====================
class Vehicle {
protected:
    string licensePlate;
    string type;
public:
    Vehicle(string licensePlate, string type)
        : licensePlate(licensePlate), type(type) {}

    string getLicensePlate() const { return licensePlate; }
    string getType() const { return type; }

    virtual void display() const {
        cout << "License: " << licensePlate << ", Type: " << type << endl;
    }
};

// ==================== PARKING SLOT ====================
class ParkingSlot {
    int slotNumber;
    bool occupied;
    Vehicle* parkedVehicle;

public:
    ParkingSlot(int num) : slotNumber(num), occupied(false), parkedVehicle(nullptr) {}

    bool isOccupied() const { return occupied; }

    bool parkVehicle(Vehicle* v) {
        if (occupied) return false;
        parkedVehicle = v;
        occupied = true;
        return true;
    }

    void removeVehicle() {
        if (parkedVehicle) delete parkedVehicle;
        parkedVehicle = nullptr;
        occupied = false;
    }

    Vehicle* getVehicle() const {
        return parkedVehicle;
    }

    int getSlotNumber() const {
        return slotNumber;
    }

    void display() const {
        cout << "Slot " << slotNumber << ": ";
        if (occupied && parkedVehicle) parkedVehicle->display();
        else cout << "Empty\n";
    }
};

// ==================== MANAGER ====================
class ParkingLotManager {
    vector<ParkingSlot> slots;
    map<string, int> vehicleToSlot;
    map<string, time_t> entryTime;

public:
    ParkingLotManager(int totalSlots) {
        for (int i = 1; i <= totalSlots; i++) {
            slots.push_back(ParkingSlot(i));
        }
        loadFromCSV(); // restore previous data
    }

    // -------------------- Park Vehicle --------------------
    void parkVehicle(Vehicle* v) {
        if (vehicleToSlot.find(v->getLicensePlate()) != vehicleToSlot.end()) {
            cout << "Vehicle already parked.\n";
            return;
        }

        for (int i = 0; i < slots.size(); i++) {
            if (!slots[i].isOccupied()) {
                slots[i].parkVehicle(v);
                vehicleToSlot[v->getLicensePlate()] = i;
                entryTime[v->getLicensePlate()] = time(0);
                cout << "Vehicle parked in slot " << i + 1 << endl;
                saveToCSV();
                return;
            }
        }
        cout << "Parking Full!\n";
    }

    // -------------------- Remove Vehicle --------------------
    void removeVehicle(string license) {
        if (vehicleToSlot.find(license) == vehicleToSlot.end()) {
            cout << "Vehicle not found!\n";
            return;
        }

        int idx = vehicleToSlot[license];
        time_t in = entryTime[license];
        time_t now = time(0);
        double hours = difftime(now, in) / 3600.0;
        double fee = calculateFee(hours);

        slots[idx].removeVehicle();
        vehicleToSlot.erase(license);
        entryTime.erase(license);
        removeFromCSV(license);

        cout << "Vehicle " << license << " removed. Parking fee: Rs. " << fee << endl;
    }

    // -------------------- Fee Calculation --------------------
    double calculateFee(double hours) {
        return max(20.0, ceil(hours) * 10.0); // ₹20 base + ₹10/hour
    }

    // -------------------- Display Status --------------------
    void displayStatus() {
        for (auto& s : slots) {
            s.display();
        }
    }

    // -------------------- Save to CSV --------------------
    void saveToCSV() {
        ofstream file("parking_data.csv");
        file << "license,type,slot,time\n";
        for (auto& u : vehicleToSlot) {
            auto license=u.first;
            auto idx=u.second;
            Vehicle* v = slots[idx].getVehicle();
            file << license << "," << v->getType() << "," << slots[idx].getSlotNumber() << "," << entryTime[license] << "\n";
        }
        file.close();
    }

    // -------------------- Remove from CSV --------------------
 void removeFromCSV(const string& license) {
    ifstream in("parking_data.csv");
    if (!in.is_open()) {
        cout << "Error: Could not open CSV file for reading.\n";
        return;
    }

    ofstream out("temp.csv");
    if (!out.is_open()) {
        cout << "Error: Could not open temp file for writing.\n";
        return;
    }

    string line;
    bool headerWritten = false;

    while (getline(in, line)) {
        // Skip empty lines
        if (line.empty()) continue;

        stringstream ss(line);
        string lic;
        getline(ss, lic, ',');

        // Write the header always
        if (!headerWritten && lic == "license") {
            out << line << "\n";
            headerWritten = true;
            continue;
        }

        // Skip this line if it's the license we want to remove
        if (lic == license) {
            continue;
        }

        // Otherwise, write the line to the temp file
        out << line << "\n";
    }

    in.close();
    out.close();

    remove("parking_data.csv");
    rename("temp.csv", "parking_data.csv");
}



    // -------------------- Load from CSV --------------------
    void loadFromCSV() {
        ifstream file("parking_data.csv");
        if (!file) return;

        string line;
        getline(file, line); // skip header

        while (getline(file, line)) {
            stringstream ss(line);
            string license, type, slotStr, timeStr;
            getline(ss, license, ',');
            getline(ss, type, ',');
            getline(ss, slotStr, ',');
            getline(ss, timeStr, ',');

            int slot = stoi(slotStr);
            time_t timestamp = stol(timeStr);

            Vehicle* v = new Vehicle(license, type);
            slots[slot - 1].parkVehicle(v);
            vehicleToSlot[license] = slot - 1;
            entryTime[license] = timestamp;
        }

        file.close();
    }
};

// ==================== MAIN ====================
int main() {
    ParkingLotManager manager(10); // 5 slots

    while (true) {
        cout << "\n===== Parking Lot Menu =====\n";
        cout << "1. Park Vehicle\n";
        cout << "2. Remove Vehicle\n";
        cout << "3. Display Parking Status\n";
        cout << "4. Exit\n";
        cout << "Enter choice: ";

        int ch;
        cin >> ch;

        if (ch == 1) {
            string plate, type;
            cout << "Enter license plate: ";
            cin >> plate;
            cout << "Enter type (Car/Bike/Truck): ";
            cin >> type;
            Vehicle* v = new Vehicle(plate, type);
            manager.parkVehicle(v);
        } else if (ch == 2) {
            string plate;
            cout << "Enter license plate to remove: ";
            cin >> plate;
            manager.removeVehicle(plate);
        } else if (ch == 3) {
            manager.displayStatus();
        } else if (ch == 4) {
            cout << "Exiting program...\n";
            break;
        } else {
            cout << "Invalid choice!\n";
        }
    }

    return 0;
}
