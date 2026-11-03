#include <iostream>
#include <string>
#include <limits>
#include <cctype>
using namespace std;

#define MAX 100
#define MAX_SLOTS 10

struct Vehicle
{
    string vehicleID;
    int batteryLevel;
    float requiredEnergy;
    string connectorType;
};

struct ChargingSlot
{
    bool available;
    string currentVehicle;
};

struct Station
{
    int stationID;
    float chargingSpeed;
    string connectorType;
    int totalSlots;
    ChargingSlot slots[MAX_SLOTS];
};

Vehicle vehicles[MAX];
Station stations[MAX];
Vehicle waitingQueue[MAX];

int vehicleCount = 0;
int stationCount = 0;
int queueCount = 0;

bool readInt(const string &message, int &value)
{
    cout << message;
    if (cin >> value) return true;
    if (cin.eof()) return false;
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Invalid input! Please enter a whole number.\n";
    return false;
}

bool readFloat(const string &message, float &value)
{
    cout << message;
    if (cin >> value) return true;
    if (cin.eof()) return false;
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Invalid input! Please enter a numeric value.\n";
    return false;
}

bool readString(const string &message, string &value)
{
    cout << message;
    return static_cast<bool>(cin >> value);
}

string normalizeConnector(string connector)
{
    string upper = connector;
    for (char &ch : upper)
        ch = static_cast<char>(toupper(static_cast<unsigned char>(ch)));

    if (upper == "CCS") return "CCS";
    if (upper == "CHADEMO") return "CHAdeMO";
    if (upper == "TYPE2") return "Type2";
    return "INVALID";
}

bool validConnector(const string &connector)
{
    return connector == "CCS" || connector == "CHAdeMO" || connector == "Type2";
}

bool enqueue(const Vehicle &v)
{
    if (queueCount >= MAX)
    {
        cout << "\nWaiting queue is full!\n";
        return false;
    }
    waitingQueue[queueCount++] = v;
    cout << "\nVehicle added to waiting queue.\n";
    return true;
}

void removeVehicle(int position)
{
    for (int i = position; i < queueCount - 1; i++)
        waitingQueue[i] = waitingQueue[i + 1];
    if (queueCount > 0) queueCount--;
}

void sortStations()
{
    for (int i = 0; i < stationCount - 1; i++)
    {
        for (int j = 0; j < stationCount - i - 1; j++)
        {
            if (stations[j].chargingSpeed < stations[j + 1].chargingSpeed)
            {
                Station temp = stations[j];
                stations[j] = stations[j + 1];
                stations[j + 1] = temp;
            }
        }
    }
}

// Sequential search: finds the first compatible station with a free slot.
bool searchStation(const Vehicle &v, int &stationIndex, int &slotIndex)
{
    for (int i = 0; i < stationCount; i++)
    {
        if (stations[i].connectorType == v.connectorType)
        {
            for (int j = 0; j < stations[i].totalSlots; j++)
            {
                if (stations[i].slots[j].available)
                {
                    stationIndex = i;
                    slotIndex = j;
                    return true;
                }
            }
        }
    }
    return false;
}

bool assignSlot(const Vehicle &v)
{
    int stationIndex = -1, slotIndex = -1;
    if (!searchStation(v, stationIndex, slotIndex)) return false;

    ChargingSlot &slot = stations[stationIndex].slots[slotIndex];
    slot.available = false;
    slot.currentVehicle = v.vehicleID;

    float chargingTime = (v.requiredEnergy / stations[stationIndex].chargingSpeed) * 60.0f;

    cout << "\n========================================\n";
    cout << "     CHARGING SLOT SUCCESSFULLY ASSIGNED\n";
    cout << "========================================\n";
    cout << "Vehicle Number   : " << v.vehicleID << endl;
    cout << "Battery Level    : " << v.batteryLevel << "%\n";
    cout << "Station ID       : " << stations[stationIndex].stationID << endl;
    cout << "Slot Number      : " << slotIndex + 1 << endl;
    cout << "Charging Speed   : " << stations[stationIndex].chargingSpeed << " kW\n";
    cout << "Connector Type   : " << stations[stationIndex].connectorType << endl;
    cout << "Estimated Time   : " << chargingTime << " minutes\n";
    cout << "========================================\n";
    return true;
}

void processQueue()
{
    int i = 0;
    bool assignedAny = false;

    // Check each waiting request in arrival order. Requests that cannot
    // use a currently free compatible slot remain in the queue.
    while (i < queueCount)
    {
        Vehicle v = waitingQueue[i];
        if (assignSlot(v))
        {
            removeVehicle(i);
            assignedAny = true;
        }
        else
        {
            i++;
        }
    }

    if (queueCount == 0)
        cout << "\nAll waiting vehicles have been processed.\n";
    else if (!assignedAny)
        cout << "\nNo waiting vehicle could be assigned at this time.\n";
    else
        cout << "\nSome vehicles are still waiting for compatible slots.\n";
}

void addVehicle()
{
    if (vehicleCount >= MAX)
    {
        cout << "\nVehicle storage is full!\n";
        return;
    }

    Vehicle v;
    if (!readString("\nEnter Vehicle Registration Number: ", v.vehicleID)) return;

    for (int i = 0; i < vehicleCount; i++)
    {
        if (vehicles[i].vehicleID == v.vehicleID)
        {
            cout << "\nVehicle already registered!\n";
            return;
        }
    }

    if (!readInt("Enter Battery Level (%): ", v.batteryLevel)) return;
    if (!readFloat("Enter Required Energy (kWh): ", v.requiredEnergy)) return;
    string connectorInput;
    if (!readString("Enter Connector Type (CCS/CHAdeMO/Type2): ", connectorInput)) return;
    v.connectorType = normalizeConnector(connectorInput);

    if (v.batteryLevel < 0 || v.batteryLevel > 100 || v.requiredEnergy <= 0 || !validConnector(v.connectorType))
    {
        cout << "\nInvalid vehicle information! Check battery level, energy and connector type.\n";
        return;
    }

    vehicles[vehicleCount++] = v;

    // If requests are already waiting, add this request behind them.
    if (queueCount > 0)
    {
        if (enqueue(v)) processQueue();
        return;
    }

    if (!assignSlot(v))
    {
        enqueue(v);
        cout << "\nNo compatible charging slot is currently available.\n";
    }
}

void addStations()
{
    int count;
    if (!readInt("\nEnter number of charging stations: ", count)) return;
    if (count <= 0 || count > MAX)
    {
        cout << "\nInvalid number of stations! Enter between 1 and " << MAX << ".\n";
        return;
    }

    for (int i = 0; i < count; i++)
    {
        Station s;
        cout << "\n========== STATION " << i + 1 << " ==========\n";
        if (!readInt("Enter Station ID: ", s.stationID)) return;

        for (int j = 0; j < i; j++)
        {
            if (stations[j].stationID == s.stationID)
            {
                cout << "\nDuplicate Station ID! Restart the program and enter unique IDs.\n";
                stationCount = 0;
                return;
            }
        }

        if (!readFloat("Enter Charging Speed (kW): ", s.chargingSpeed)) return;
        string connectorInput;
        if (!readString("Enter Connector Type (CCS/CHAdeMO/Type2): ", connectorInput)) return;
        s.connectorType = normalizeConnector(connectorInput);
        if (s.chargingSpeed <= 0 || !validConnector(s.connectorType))
        {
            cout << "\nInvalid station information. Restart and enter valid details.\n";
            stationCount = 0;
            return;
        }

        if (!readInt("Enter number of charging slots (1-10): ", s.totalSlots)) return;
        if (s.totalSlots < 1 || s.totalSlots > MAX_SLOTS)
        {
            cout << "\nInvalid slot count! Each station can have 1 to " << MAX_SLOTS << " slots.\n";
            stationCount = 0;
            return;
        }

        for (int k = 0; k < s.totalSlots; k++)
        {
            s.slots[k].available = true;
            s.slots[k].currentVehicle = "";
        }
        stations[i] = s;
    }

    stationCount = count;
    sortStations();
    cout << "\nCharging stations registered successfully!\n";
    cout << "Stations arranged according to charging speed.\n";
}

void completeCharging()
{
    int id, slotNumber;
    if (!readInt("\nEnter Station ID: ", id)) return;
    if (!readInt("Enter Slot Number: ", slotNumber)) return;

    for (int i = 0; i < stationCount; i++)
    {
        if (stations[i].stationID == id)
        {
            if (slotNumber < 1 || slotNumber > stations[i].totalSlots)
            {
                cout << "\nInvalid slot number for this station.\n";
                return;
            }

            ChargingSlot &slot = stations[i].slots[slotNumber - 1];
            if (!slot.available)
            {
                cout << "\nCharging completed for Vehicle Number: " << slot.currentVehicle << endl;
                slot.available = true;
                slot.currentVehicle = "";
                cout << "\nStation " << id << ", Slot " << slotNumber << " is now available.\n";
                processQueue();
            }
            else
            {
                cout << "\nThis charging slot is already available.\n";
            }
            return;
        }
    }
    cout << "\nStation not found!\n";
}

void displayStations()
{
    cout << "\n========== CHARGING STATIONS ==========" << endl;
    for (int i = 0; i < stationCount; i++)
    {
        int availableCount = 0;
        for (int j = 0; j < stations[i].totalSlots; j++)
            if (stations[i].slots[j].available) availableCount++;

        cout << "\nStation ID       : " << stations[i].stationID << endl;
        cout << "Charging Speed   : " << stations[i].chargingSpeed << " kW\n";
        cout << "Connector Type   : " << stations[i].connectorType << endl;
        cout << "Total Slots      : " << stations[i].totalSlots << endl;
        cout << "Available Slots  : " << availableCount << endl;

        for (int j = 0; j < stations[i].totalSlots; j++)
        {
            cout << "  Slot " << j + 1 << " : ";
            if (stations[i].slots[j].available)
                cout << "AVAILABLE\n";
            else
                cout << "OCCUPIED - Vehicle " << stations[i].slots[j].currentVehicle << endl;
        }
        cout << "---------------------------------------\n";
    }
}

void displayQueue()
{
    cout << "\n========== WAITING VEHICLES ==========" << endl;
    if (queueCount == 0)
    {
        cout << "No vehicles waiting.\n";
        return;
    }

    for (int i = 0; i < queueCount; i++)
    {
        cout << "\nVehicle Number : " << waitingQueue[i].vehicleID << endl;
        cout << "Battery Level  : " << waitingQueue[i].batteryLevel << "%\n";
        cout << "Required Energy: " << waitingQueue[i].requiredEnergy << " kWh\n";
        cout << "Connector Type : " << waitingQueue[i].connectorType << endl;
        cout << "-----------------------------\n";
    }
    cout << "\nTotal Waiting Vehicles: " << queueCount << endl;
}

int main()
{
    int choice = 0;
    cout << "\n================================================\n";
    cout << "       EV CHARGING SLOT ASSIGNMENT SYSTEM\n";
    cout << "       MULTIPLE SLOTS PER CHARGING STATION\n";
    cout << "================================================\n";

    addStations();
    if (stationCount == 0)
    {
        cout << "\nUnable to initialize charging stations. Exiting.\n";
        return 0;
    }

    do
    {
        cout << "\n\n============== MAIN MENU ==============\n";
        cout << "1. Register New Vehicle\n";
        cout << "2. Complete Charging\n";
        cout << "3. Display Charging Stations\n";
        cout << "4. Display Waiting Queue\n";
        cout << "5. Process Waiting Queue\n";
        cout << "6. Exit\n";
        cout << "=======================================\n";

        if (!readInt("Enter Your Choice: ", choice))
        {
            cout << "\nInvalid input or input ended. Exiting program.\n";
            break;
        }

        switch (choice)
        {
            case 1: addVehicle(); break;
            case 2: completeCharging(); break;
            case 3: displayStations(); break;
            case 4: displayQueue(); break;
            case 5: processQueue(); break;
            case 6: cout << "\nThank you for using EV Charging System!\n"; break;
            default: cout << "\nInvalid Choice! Enter a number between 1 and 6.\n";
        }
    } while (choice != 6);

    return 0;
}
