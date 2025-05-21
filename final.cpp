#include<iostream>
#include<mutex>
#include<thread>
#include<chrono>
#include<cstdlib>
#include<ctime>
#include<random>
#include<sstream>
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;
mt19937 rng(random_device{}());

struct date {
	int day; // day of the month (1-31)
	int month;  // month (0-11, January is 0)
	int year; // years since 1900
};

class AVN {
	string avnID;
	int fine;
	int serviceFeePercent;
	int TotalFine;
	bool paymentStatus;
	string reason;
	date issueDate;
	date dueDate;

public:
	AVN* next;

	AVN(string avnid, int ffine, int fee, string reason, int dd, int mm, int yy) {
		next = NULL;
		avnID = avnid;
		fine = ffine;
		serviceFeePercent = fee;
		TotalFine = ffine + (fine * serviceFeePercent / 100);
		this->reason = reason;
		issueDate.day = dd;
		issueDate.month = mm;
		issueDate.year = yy;
		dueDate.year = yy;
		dueDate.month = mm;
		dueDate.day = dd + 3;
		paymentStatus = 0;
	}

	void updatePaymentStatus() {
		paymentStatus = 1;
	}

	bool getPaymentStatus() {
		return paymentStatus;
	}

	int getTotalFine() {
		return TotalFine;
	}

	string getAvnId() {
		return avnID;
	}

	string getReason() {
		return reason;
	}
};

class StripePay {
public:
	void StripePayment(string AvnId, string AircraftId, int type, int amount) {
		cout << "___StripePay___" << endl;
		cout << "Fine " << AvnId << ", from Aircraft " << AircraftId << " (";
		if (type)
			cout << "Cargo) ";
		else
			cout << "Commercial) ";
		cout << "of amount " << amount << " has been paid!!" << endl << endl;
	}
};

class Aircraft {
private:
	string flight_number;
	int type; //0-->commercial, 1-->cargo, 2-->military, 3-->medical
	bool emergency_check;
	int speed;
	int direction; //0-->arriving or 1-->departuring
	int flight_phase;    //0-->HOLDING, 1-->APPROACH, 2-->LANDING, 3-->taxi, 4-->at_gate // Arrival phases
	//6-->at gate, 7-->taxi, 8-->takeoff roll, 9-->climb, 10-->departure(cruise),  // Departure phases
	bool avnCheck;
	bool fault;
	int flight_type; // 0-->international  1-->domestic
	int airline_no = -1;
	bool runway_assigned = false;
	int onRunway = -1;
	chrono::system_clock::time_point faultTime;
	const string reasonString;
	bool clearance_assigned = false;
	bool hasLanded = false;
	bool hasDepartured = false;
	int fuel;
	int wait;


public:
	AVN* violations;
	int violationCount;

	Aircraft(string fn, int type = -1, int direction = -1, int flight_phase = -1, int speed = -1, int airline_index = -1, int flight_type = -1, bool emergency_check = false) {
		violations = NULL;
		flight_number = fn;
		this->type = type;
		this->direction = direction;
		this->flight_phase = flight_phase;
		this->speed = speed;
		this->airline_no = airline_index;
		this->flight_type = flight_type;
		this->emergency_check = emergency_check;
		fault = false;
		avnCheck = false;
		fuel = 40;
		wait = 0;
		violationCount = 0;
	}

	void detectViolation() {
		// Example Rule 1: Fuel too low and not declared emergency
		if (fuel <= 4 && !emergency_check) {
			updateAvn("Fuel critical but emergency not declared.");
		}

		if (direction == 0) {

			if (flight_phase == 0 && (speed < 400 || speed >600)) {
				updateAvn("Speed outside holding limits while arriving");
			}
			else if (flight_phase == 1 && (speed < 240 || speed >= 400)) {
				updateAvn("Speed outside approaching limits while arriving");
			}
			else if (flight_phase == 2 && (speed < 30 || speed >= 240)) {
				updateAvn("Speed outside landing limits while arriving");
			}
			else if (flight_phase == 3 && (speed < 15 || speed >= 30)) {
				updateAvn("Speed outside taxiing limits while arriving");
			}
			else if (flight_phase == 4 && (speed < 0 || speed >= 15)) {
				updateAvn("Speed outside standing limits while arriving");
			}

		}
		else if (direction == 1) {
			if (flight_phase == 6 && (speed < 0 || speed >= 15)) {
				updateAvn("Speed outside standing limits while departuring");
			}
			else if (flight_phase == 7 && (speed < 15 || speed >= 30)) {
				updateAvn("Speed outside taxxing limits while departuring");
			}
			else if (flight_phase == 8 && (speed < 30 || speed >= 240)) {
				updateAvn("Speed outside taking off limits while departuring");
			}
			else if (flight_phase == 9 && (speed < 240 || speed >= 400)) {
				updateAvn("Speed outside climbing limits while departuring");
			}
			else if (flight_phase == 10 && (speed < 400 || speed >600)) {

				updateAvn("Speed outside crusing limits while departuring");
			}

		}
	}

	void checkViolationResolved() {
		// If all conditions are now safe, reset the violation flag
		if (fuel > 4 && clearance_assigned) {
			if (avnCheck) {
				//cout << "\nViolation cleared for Flight: " << flight_number << "\n";
				updateAvnToFalse();
			}
		}
	}

	void decreaseFuel() {
		if (fuel > 0) fuel--;
		if (fuel <= 4 && !emergency_check) { // example threshold for emergency
			setEmergencyCheck(1);
		}
		detectViolation();
	}

	void incWait() {
		wait++;
	}
	void resetWait() {
		wait = 0;
	}
	int getWait() {
		return wait;
	}

	bool getHasLanded() {
		return hasLanded;
	}

	void setHasLanded(bool a) {
		hasLanded = a;
	}

	bool getHasDepartured() {
		return hasDepartured;
	}

	void setHasDepartured(bool a) {
		hasDepartured = a;
	}

	// Getter and Setter for flight_number
	string getFlightNumber() const {
		return flight_number;
	}
	void setFlightNumber(const std::string& fn) {
		flight_number = fn;
	}

	chrono::system_clock::time_point getFaultTime() const {
		return faultTime;
	}

	void setClearanceGranted(bool val) { clearance_assigned = val; checkViolationResolved(); }
	bool isClearanceGranted() const { return clearance_assigned; }

	void setRunwayAssigned(bool val) { runway_assigned = val; }
	bool is_runway_assigned() const { return runway_assigned; }

	// Getter and Setter for type
	int getFuel() {
		return fuel;
	}
	bool getEmergencyCheck() {
		return emergency_check;
	}

	int getType() const {
		return type;
	}
	void setType(int t) {
		type = t;
	}

	void SetOnRunway(int a) {
		onRunway = a;
	}
	int GetOnRunway() {
		return onRunway;
	}

	// Getter and Setter for airline_no
	int get_airline_no() const {
		return airline_no;
	}
	void set_airline_no(int t) {
		airline_no = t;
	}

	// Getter and Setter for emergency_check
	bool isEmergencyCheck() const {
		return emergency_check;
	}
	void setEmergencyCheck(bool ec) {
		emergency_check = ec;
	}

	// Getter and Setter for fault
	bool isFaultCheck() const {
		return fault;
	}
	void setFaultCheck(bool ec, int reason) {
		fault = ec;
		if (fault) {
			cout << endl << "Fault Found!!!" << endl;
			faultTime = chrono::system_clock::now();
			if (reason == 0) {
				cout << "Reason: Brake failure" << endl;
			}
			else if (reason == 1) {
				cout << "Reason: Hydraulic leak" << endl;
			}
			else {
				cout << "Reason: Steering malfunction" << endl;
			}
		}
		else {
			faultTime = chrono::system_clock::time_point{};
			if (reason == -1)
				cout << endl << "Fault Repaired" << endl;

		}
	}

	// Getter and Setter for speed
	double getSpeed() const {
		return speed;
	}
	void setSpeed(double s) {
		speed = s;
		detectViolation();
	}

	// Getter and Setter for direction
	int getDirection() {
		return direction;
	}
	void setDirection(int d) {
		direction = d;
	}

	// Getter and Setter for flight_phase
	int getFlightPhase() const {
		return flight_phase;
	}
	void setFlightPhase(int phase) {
		flight_phase = phase;
		detectViolation();
	}

	// Getter and Setter for avnCheck
	bool isAvnCheck() const {
		return avnCheck;
	}
	void setAvnCheck(bool ac) {
		avnCheck = ac;
	}

	void updateAvn(string reason) {
		avnCheck = true;
		AVN* temp = violations;
		int ffine = 0;
		if (type == 0)
			ffine = 500000;
		else if (type == 1)
			ffine = 700000;
		time_t now = time(nullptr);
		tm localTime;
		localtime_s(&localTime, &now);
		AVN* current = new AVN(to_string(violationCount) + flight_number, ffine, 15, reason, localTime.tm_mday, localTime.tm_mon + 1, localTime.tm_year);
		for (int i = 0; i < violationCount - 1; i++) {
			temp = temp->next;
		}
		if (!violationCount)
			violations = current;
		else
			temp->next = current;
		violationCount++;
		cout << endl << "Voilation!!!" << endl;
		cout << "AVN Activated For Flight!!";
		cout << "Aircraft Details: " << endl;
		cout << "Reason: " << reason << endl;
		this->displayInfo();
	}

	void updateAvnToFalse() {
		avnCheck = false;
	}

	bool getAVN() {
		return avnCheck;
	}

	void displayInfo() const {
		cout << "Flight Details:" << endl;
		cout << "Flight Number: " << flight_number << "\n";
		if (type == 0)
			cout << "Type: Commercial\n";
		else if (type == 1)
			cout << "Type: Cargo\n";
		else if (type == 2)
			cout << "Type: Military\n";
		else if (type == 3)
			cout << "Type: Medical\n";
		else
			cout << "Type: Unknown\n";
		cout << "Emergency Check: " << (emergency_check ? "Yes" : "No") << "\n";
		cout << "Speed: " << speed << " knots\n";
		cout << "Fuel: " << fuel << "00 liters" << endl;
		cout << "Direction: " << (direction == 0 ? "Arriving" : "Departing") << "\n";
		if (runway_assigned) {
			cout << "Runway: ";
			if (onRunway == 0)
				cout << "A";
			else if (onRunway == 1)
				cout << "B";
			else
				cout << "C";
			cout << endl;

		}
		else {
			cout << "Runway not assigned yet" << endl;
		}

		cout << "Flight Phase: ";
		switch (flight_phase) {
		case 0: cout << "Holding"; break;
		case 1: cout << "Approach"; break;
		case 2: cout << "Landing"; break;
		case 3: cout << "Taxiing (Arrival)"; break;
		case 4: cout << "At Gate (Arrival)"; break;
		case 6: cout << "At Gate (Departure)"; break;
		case 7: cout << "Taxiing (Departure)"; break;
		case 8: cout << "Takeoff Roll"; break;
		case 9: cout << "Climb"; break;
		case 10: cout << "Cruise"; break;
		default: cout << "Unknown Phase"; break;
		}
		cout << "\n";

		cout << "Avionics Violation: " << (avnCheck ? "ON" : "OFF") << "\n";
		cout << "------------------------------------------\n";
	}
};

class Airline {
private:
	string name;
	int type; //0-->commercial, 1-->cargo, 2-->military, 3-->medical
	int total_aircrafts;
	int total_active_aircrafts;
	int avncount;
	int current_active_aircaft_count = 0;
	Aircraft* aircrafts[10];
	StripePay stripepayments;

public:

	Airline(string name = " ", int type = 0, int total_aircrafts = 0, int active_aircrafts = 0, int fine = 0) {
		this->name = name;
		this->type = type;
		this->total_aircrafts = total_aircrafts;
		this->total_active_aircrafts = active_aircrafts;
		this->avncount = fine;

		for (int i = 0; i < 10; i++) {
			aircrafts[i] = NULL;
		}
	}

	Aircraft* createAircraft(const string fn, int type = -1, int direciton = -1, int flight_phase = -1, int speed = -1, int airline_index = -1, int flight_type = -1, bool emergency_check = false) {
		current_active_aircaft_count++;
		return aircrafts[current_active_aircaft_count - 1] = new Aircraft(fn, type, direciton, flight_phase, speed, airline_index, flight_type, emergency_check);
	}

	void payFines() {
		AVN* temp;
		int vCount;
		string id;
		bool type;
		for (int i = 0; i < 10; i++) {
			if (aircrafts[i] != NULL) {
				temp = aircrafts[i]->violations;
				vCount = aircrafts[i]->violationCount;
				for (int j = 0; j < vCount; j++) {
					if (!temp->getPaymentStatus()) {
						stripepayments.StripePayment(temp->getAvnId(), aircrafts[i]->getFlightNumber(), aircrafts[i]->getType(), temp->getTotalFine());
						temp->updatePaymentStatus();
					}
					temp = temp->next;
				}
				aircrafts[i]->setAvnCheck(false);
			}
		}
	}

	// Setters
	void setName(const string& n) {
		name = n;
	}

	void setType(int t) {
		type = t;
	}

	void setTotalAircrafts(int total) {
		total_aircrafts = total;
	}

	void setActiveAircrafts(int active) {
		total_active_aircrafts = active;
	}

	// Getters
	string getName() const {
		return name;
	}

	int getType() const {
		return type;
	}


	int getTotalAircrafts() const {
		return total_aircrafts;
	}

	int getTotalActiveAircrafts() const {
		return total_active_aircrafts;
	}

	int getCurrentActiveAircrafts() const {
		return current_active_aircaft_count;
	}

	int getAvnCount() const {
		return avncount;
	}

	void updateAvnCount() {
		//cout << endl << "AVN Count for " << name << " has increased by 50" << endl;
		avncount += 50;
		//cout << "The updated AVN Count is: " << avncount << endl;
	}

	int retAvnAmount() {
		return avncount;
	}

};

class Runway {
private:
	string type; //1-->RWY-A: North-South alignment (arrivals) 
	//2-->RWY - B: East - West alignment(departures)
	//3-->RWY - C : Flexible for cargo / emergency / overflow
	int number;
	bool isAvailable;
	int currentFlightNumber = -1;
	mutex runwayMutex;

public:
	Runway(int number = -1, string type = " ", bool avil = true) {
		this->number = number;
		this->type = type;
		isAvailable = avil;
	}

	Aircraft* getOnRunwayAircraft(Aircraft** temp) {
		if (currentFlightNumber != -1)
			return temp[currentFlightNumber];
		else
			return NULL;
	}

	bool getIsAvail() {
		return isAvailable;
	}

	mutex& retRunwayMutex() {
		return runwayMutex;
	}

	void setIsAvail(bool a) {
		isAvailable = a;
	}

	void updateFlightOnR(int a) {
		currentFlightNumber = a;
	}

	int getFlightOnR() {
		return currentFlightNumber;
	}

	void resetFlightOnR() {
		currentFlightNumber = -1;
	}
};

class Radar {
private:

public:

	Radar() {};

	void RadarMainLoop(Aircraft* activeFlights[], int activeFlightCount, Airline* airline[]) {
		for (int i = 0; i < activeFlightCount; i++) {

			if (activeFlights[i] == NULL)
				continue;

			if (activeFlights[i]->isFaultCheck())
				continue;

			activeFlights[i]->detectViolation();

			this_thread::sleep_for(chrono::seconds(2));
		}

	}
};

class Dispatcher {
private:

public:
	Dispatcher() {};

	void DispatcherMainLoop(Aircraft* activeFlights[], int activeFlightCount, Runway* runways[]) {
		if (activeFlightCount > 0) {
			// First handle Emergency and Priority Flights
			for (int i = 0; i < activeFlightCount; i++) {
				if (activeFlights[i] == NULL)
					continue;

				if (activeFlights[i]->isFaultCheck())
					continue;

				if ((activeFlights[i]->getDirection() == 0 && (activeFlights[i]->getFlightPhase() == 3 || activeFlights[i]->getFlightPhase() == 4)) ||
					(activeFlights[i]->getDirection() == 1 && (activeFlights[i]->getFlightPhase() == 9 || activeFlights[i]->getFlightPhase() == 10)))
					continue;

				if (!activeFlights[i]->is_runway_assigned() && !(activeFlights[i]->getHasLanded() || activeFlights[i]->getHasDepartured())) {
					if (activeFlights[i]->isEmergencyCheck() || activeFlights[i]->getType() == 1) {
						bool assigned = false;

						if (runways[2]->retRunwayMutex().try_lock()) {
							if (runways[2]->getIsAvail() && runways[2]->getFlightOnR() == -1) {
								runways[2]->setIsAvail(0);
								runways[2]->updateFlightOnR(i);
								activeFlights[i]->SetOnRunway(2);
								activeFlights[i]->setRunwayAssigned(true);
								activeFlights[i]->setClearanceGranted(true);
								assigned = true;
								//cout << "\nRunway C assigned to Priority/Emergency Flight" << endl;
								//activeFlights[i]->displayInfo();
							}
							runways[2]->retRunwayMutex().unlock();
						}

						if (!assigned && activeFlights[i]->isEmergencyCheck()) {
							//cout << "\nRunway C is occupied. Attempting emergency preemption..." << endl;

							// Preempt A or B based on direction
							freeRunwayForEmergency(activeFlights, runways, activeFlights[i]->getDirection());
							int rIdx = activeFlights[i]->getDirection(); // 0 -> A, 1 -> B

							runways[rIdx]->retRunwayMutex().lock();
							runways[rIdx]->setIsAvail(0);
							runways[rIdx]->updateFlightOnR(i);
							activeFlights[i]->SetOnRunway(rIdx);
							activeFlights[i]->setRunwayAssigned(true);
							activeFlights[i]->setClearanceGranted(true);
							//activeFlights[i]->setEmergencyCheck(false);
							//cout << "\nEmergency Flight " << activeFlights[i]->getFlightNumber() << " assigned to Runway " << (rIdx == 0 ? "A" : "B") << " after preemption" << endl;
							//activeFlights[i]->displayInfo();
							runways[rIdx]->retRunwayMutex().unlock();
						}
					}
				}
			}

			// Now handle normal Arrival flights (Runway A)
			if (runways[0]->retRunwayMutex().try_lock()) {
				if (runways[0]->getIsAvail() && runways[0]->getFlightOnR() == -1) {
					Aircraft* bestArrival = nullptr;
					int maxWait = -1;
					int flightNo = -1;

					for (int i = 0; i < activeFlightCount; i++) {
						if (activeFlights[i] == NULL)
							continue;

						if (activeFlights[i]->isFaultCheck())
							continue;

						if (activeFlights[i]->getDirection() != 0)
							continue;

						if (activeFlights[i]->is_runway_assigned() || activeFlights[i]->getHasLanded())
							continue;

						if (activeFlights[i]->getWait() > maxWait) {
							maxWait = activeFlights[i]->getWait();
							bestArrival = activeFlights[i];
							flightNo = i;
						}
					}

					if (bestArrival != nullptr) {
						runways[0]->setIsAvail(0);
						runways[0]->updateFlightOnR(flightNo); // Assuming getFlightNumber() == index
						bestArrival->SetOnRunway(0);
						bestArrival->setRunwayAssigned(true);
						bestArrival->setClearanceGranted(true);
						bestArrival->resetWait();

						//cout << "\nRunway A assigned to Arrival Flight" << endl;
						//bestArrival->displayInfo();
					}
				}
				runways[0]->retRunwayMutex().unlock();
			}

			// Handle normal Departure flights (Runway B)
			if (runways[1]->retRunwayMutex().try_lock()) {
				if (runways[1]->getIsAvail() && runways[1]->getFlightOnR() == -1) {
					Aircraft* bestDeparture = nullptr;
					int maxWait = -1;
					int flightNo = -1;

					for (int i = 0; i < activeFlightCount; i++) {
						if (activeFlights[i] == NULL)
							continue;

						if (activeFlights[i]->isFaultCheck())
							continue;

						if (activeFlights[i]->getDirection() != 1)
							continue;

						if (activeFlights[i]->is_runway_assigned() || activeFlights[i]->getHasDepartured())
							continue;

						if (activeFlights[i]->getWait() > maxWait) {
							maxWait = activeFlights[i]->getWait();
							bestDeparture = activeFlights[i];
							flightNo = i;
						}
					}

					if (bestDeparture != nullptr) {
						runways[1]->setIsAvail(0);
						runways[1]->updateFlightOnR(flightNo);
						bestDeparture->SetOnRunway(1);
						bestDeparture->setRunwayAssigned(true);
						bestDeparture->setClearanceGranted(true);
						bestDeparture->resetWait();

						//cout << "\nRunway B assigned to Departure Flight" << endl;
						//bestDeparture->displayInfo();
					}
				}
				runways[1]->retRunwayMutex().unlock();
			}

			// Increase wait value for unassigned active flights
			for (int i = 0; i < activeFlightCount; i++) {
				if (activeFlights[i] == NULL)
					continue;

				if (activeFlights[i]->isFaultCheck())
					continue;

				if ((activeFlights[i]->getDirection() == 0 && (activeFlights[i]->getFlightPhase() == 3 || activeFlights[i]->getFlightPhase() == 4)) ||
					(activeFlights[i]->getDirection() == 1 && (activeFlights[i]->getFlightPhase() == 9 || activeFlights[i]->getFlightPhase() == 10)))
					continue;

				if (!activeFlights[i]->is_runway_assigned() && !(activeFlights[i]->getHasLanded() || activeFlights[i]->getHasDepartured())) {
					activeFlights[i]->incWait();
					activeFlights[i]->decreaseFuel();
				}
			}

			this_thread::sleep_for(chrono::seconds(5));
		}
	}

	void freeRunway(Aircraft* activeFlights[], Runway* runways[]) {

		for (int i = 0; i < 3; i++) {
			runways[i]->retRunwayMutex().lock();
			int currentflightnumber = runways[i]->getFlightOnR();

			if (activeFlights[currentflightnumber]->getHasLanded() || activeFlights[currentflightnumber]->getHasDepartured()) {
				activeFlights[currentflightnumber]->SetOnRunway(-1);
				activeFlights[currentflightnumber] = NULL;
				runways[i]->setIsAvail(1);
				runways[i]->resetFlightOnR();
			}
			runways[i]->retRunwayMutex().unlock();
		}
	}

	void freeRunwayForEmergency(Aircraft* activeFlights[], Runway* runways[], int direction) {
		int runwayIndex = (direction == 0) ? 0 : 1;

		runways[runwayIndex]->retRunwayMutex().lock();

		int currentFlightNumber = runways[runwayIndex]->getFlightOnR();

		if (currentFlightNumber != -1 && activeFlights[currentFlightNumber] != NULL) {
			// Preempt the current flight
			activeFlights[currentFlightNumber]->SetOnRunway(-1);
			activeFlights[currentFlightNumber]->setRunwayAssigned(false);
			activeFlights[currentFlightNumber]->setClearanceGranted(false);

			//cout << "\nRunway " << (runwayIndex == 0 ? "A" : "B") << " has been PREEMPTED for EMERGENCY flight!" << endl;
			//cout << "Flight " << activeFlights[currentFlightNumber]->getFlightNumber() << " removed from Runway " << (runwayIndex == 0 ? "A" : "B") << endl;
		}

		runways[runwayIndex]->setIsAvail(1);
		runways[runwayIndex]->resetFlightOnR();
		runways[runwayIndex]->retRunwayMutex().unlock();
	}

};

class Faulty {
private:

public:

	Faulty() {};

	void FaultMainLoop(Aircraft* activeFlights[], int activeFlightCount) {
		for (int i = 0; i < activeFlightCount; i++) {

			if (activeFlights[i] == NULL)
				continue;

			int phase = activeFlights[i]->getFlightPhase();

			if (phase == 3 || phase == 4 || phase == 6 || phase == 7) {
				if (!activeFlights[i]->isFaultCheck()) {
					uniform_int_distribution<int> dist(0, 99);
					int random_number = dist(rng);
					int prob = random_number;
					if (prob < 10) {
						uniform_int_distribution<int> dist(0, 2);
						int random_number = dist(rng);
						int reason = random_number;
						activeFlights[i]->setFaultCheck(true, reason);
					}
				}
			}

			this_thread::sleep_for(chrono::seconds(1));

			for (int j = 0; j < activeFlightCount; j++) {

				if (activeFlights[j] == NULL)
					continue;

				if (i != j) {
					if (activeFlights[j]->isFaultCheck() && chrono::system_clock::now() - activeFlights[j]->getFaultTime() > chrono::minutes(1)) {
						activeFlights[j]->setFaultCheck(false, -1);
					}
				}
			}
		}


	}

	//3, 4 (arr) , 6 7 (dept)

	bool getFault(Aircraft* obj) {
		return obj->isFaultCheck();
	}

};

class simluation {
private:

public:

	simluation() {};

	void simulationLoop(Runway* runways[], Aircraft* activeFlights[]) {
		Aircraft* ac;

		for (int i = 0; i < 3; i++) {
			ac = runways[i]->getOnRunwayAircraft(activeFlights);

			if (ac != NULL) {
				if (ac == NULL)
					continue;

				if (ac->isFaultCheck())
					continue;

				int currentPhase = ac->getFlightPhase();
				double speed = ac->getSpeed();
				int direction = ac->getDirection();

				// ARRIVALS
				if (direction == 0) {
					switch (currentPhase) {
					case 0: // Cruising -> Descending
						ac->decreaseFuel();
						if (speed >= 400) {
							ac->setSpeed(speed - 30);
							cout << endl << "Speed Decreased by 30 at Crusing Phase for (arrival)" << endl;
							ac->displayInfo();
							cout << endl;
						}
						else {
							ac->setFlightPhase(1);
							//uniform_int_distribution<int> dist(240, 290);
							//int random_number = dist(rng);
							//speed = random_number;
							ac->setSpeed(speed);
							cout << endl << "Phase changed from holding to descending for (arrival)" << endl;
							ac->displayInfo();
							cout << endl;
						}
						break;
					case 1: // Descending -> Approach
						ac->decreaseFuel();
						if (speed >= 240) {
							ac->setSpeed(speed - 30);
							cout << endl << "Speed Decreased by 30 at Descending Phase for (arrival)" << endl;
							ac->displayInfo();
							cout << endl;
						}
						else {
							ac->setFlightPhase(2);
							//activeFlights[i]->setSpeed(240);
							cout << endl << "Phase changed from Descending to Approach for (arrival)" << endl;
							ac->displayInfo();
							cout << endl;
						}
						break;
					case 2: // Approach -> Holding or Landing
						ac->decreaseFuel();
						if (ac->isClearanceGranted() && ac->is_runway_assigned() && ac->GetOnRunway() != -1) {
							if (ac->getSpeed() > 30) {
								ac->setSpeed(speed - 60);
								cout << endl << "Speed Decreased by 20 at approaching Phase for (arrival)" << endl;
								activeFlights[i]->displayInfo();
								cout << endl;
								//activeFlights[i]->setClearanceGranted(false);
								//activeFlights[i]->SetOnRunway(-1);
								//activeFlights[i]->setRunwayAssigned(false);
							}
							else {
								ac->setFlightPhase(3); // go to taxi
								//uniform_int_distribution<int> dist(15, 30);
								//int random_number = dist(rng);
								//speed = random_number;
								//ac->setSpeed(speed);
								cout << endl << "Phase changed from Landing to Taxi for (arrival)" << endl;
								ac->displayInfo();
								cout << endl;
							}
						}
						else if (ac->getSpeed() > 30) {
							ac->setSpeed(speed - 60);
							cout << endl << "Speed Decreased by 14 at approaching Phase for (arrival)" << endl;
							ac->displayInfo();
							cout << endl;
						}
						break;
					case 3: // Holding -> Try again for approach
						ac->decreaseFuel();
						//if (activeFlights[i]->is_runway_assigned() == false)
							//break;

						if (ac->getSpeed() > 0)
							ac->setSpeed(speed - 10);
						else {
							ac->setSpeed(0);
							ac->setFlightPhase(4);
							cout << endl << "Phase changed from Taxi to Gate for (arrival)" << endl;
							ac->displayInfo();
							cout << endl;
						}
						break;
					case 4: // Landed
						ac->decreaseFuel();
						//if (activeFlights[i]->is_runway_assigned() == false)
							//break;

						ac->setHasLanded(true);
						break;
					}
				}

				// DEPARTURES
				else if (direction == 1) {
					switch (currentPhase) {
					case 6: // Parked -> Taxi
						ac->decreaseFuel();
						if (ac->isClearanceGranted() && ac->is_runway_assigned() && ac->GetOnRunway() != -1) {
							if (speed <= 15)
								ac->setSpeed(speed + 10);
							else {
								//uniform_int_distribution<int> dist(15, 30);
								//int random_number = dist(rng);
								//speed = random_number;
								//activeFlights[i]->setSpeed(speed);
								ac->setFlightPhase(7);
								cout << endl << "Phase changed from Gate to Taxi for (departure)" << endl;
								ac->displayInfo();
								cout << endl;
							}
						}
						else {
							ac->setSpeed(0);
							cout << endl << "Aircraft is at Parking as No Runway has been assigned yet (departure)" << endl;
							ac->displayInfo();
							cout << endl;
						}
						break;
					case 7: // Taxi -> Takeoff
						ac->decreaseFuel();
						if (speed < 30)
							ac->setSpeed(speed + 10);
						else {
							//activeFlights[i]->setSpeed(0);
							ac->setFlightPhase(8);
							cout << endl << "Phase changed, Plane is ready to take off (departure)" << endl;
							ac->displayInfo();
							cout << endl;
						}

						/*if (activeFlights[i]->isClearanceGranted() && activeFlights[i]->is_runway_assigned() && activeFlights[i]->GetOnRunway() != -1) {

							if (activeFlights[i]->getSpeed() <= 30) {
								activeFlights[i]->setSpeed(speed + 4);
								cout << endl << "Speed increased by 4 at Taxi for" << endl;
								activeFlights[i]->displayInfo();
								cout << endl;
							}
							else {
								activeFlights[i]->setSpeed(0);
								activeFlights[i]->setFlightPhase(8);
								cout << endl << "Phase changed from Taxi to Takeoff for" << endl;
								activeFlights[i]->displayInfo();
								cout << endl;

							}

						}
						else {
							activeFlights[i]->setSpeed(0);
							activeFlights[i]->setFlightPhase(6);
							cout << endl << "Phase changed from Taxi to Gate as Runway was not Assigned for" << endl;
							activeFlights[i]->displayInfo();
							cout << endl;
						}*/
						break;
					case 8: // Takeoff -> Ascend
						ac->decreaseFuel();
						if (speed <= 240) {
							ac->setSpeed(speed + 30);
							cout << endl << "Speed incraesed by 30 at Takeoff for (departure)" << endl;
							ac->displayInfo();
							cout << endl;
						}
						else {
							//uniform_int_distribution<int> dist(250, 463);
							//int random_number = dist(rng);
							//speed = random_number;
							//activeFlights[i]->setSpeed(speed);
							ac->setFlightPhase(9);
							cout << endl << "Phase changed from Takeoff to Climb for (departure)" << endl;
							ac->displayInfo();
							cout << endl;
						}
						break;
					case 9: // Ascend -> Cruising
						ac->decreaseFuel();
						if (speed <= 400) {
							ac->setSpeed(speed + 30);
							cout << endl << "Speed incraesed by 30 at Ascend at climb for (departure)" << endl;
							ac->displayInfo();
							cout << endl;
						}
						else {
							ac->setFlightPhase(10);
							ac->setHasDepartured(true);
							//activeFlights[i]->setSpeed(800);
							cout << endl << "Phase changed from Ascend to Crusing for (departure)" << endl;
							ac->displayInfo();
							cout << endl;
						}
						break;
					case 10: // Cruising
						ac->decreaseFuel();
						if (speed <= 600)
							ac->setSpeed(speed + 5);
						else
							ac->setSpeed(speed - 5);

						cout << endl << "Flight is currently Crusing" << endl;
						ac->displayInfo();
						cout << endl;
						break;
					}
				}
			}
		}
		
		/*
		for (int i = 0; i < activeFlightCount; i++) {

			if (activeFlights[i] == NULL)
				continue;

			if (activeFlights[i]->isFaultCheck())
				continue;

			int currentPhase = activeFlights[i]->getFlightPhase();
			double speed = activeFlights[i]->getSpeed();
			int direction = activeFlights[i]->getDirection();

			// ARRIVALS
			if (direction == 0) {
				switch (currentPhase) {
				case 0: // Cruising -> Descending
					activeFlights[i]->decreaseFuel();
					if (speed >= 400) {
						activeFlights[i]->setSpeed(speed - 30);
						cout << endl << "Speed Decreased by 30 at Crusing Phase for (arrival)" << endl;
						activeFlights[i]->displayInfo();
						cout << endl;
					}
					else {
						activeFlights[i]->setFlightPhase(1);
						//uniform_int_distribution<int> dist(240, 290);
						//int random_number = dist(rng);
						//speed = random_number;
						activeFlights[i]->setSpeed(speed);
						cout << endl << "Phase changed from holding to descending for (arrival)" << endl;
						activeFlights[i]->displayInfo();
						cout << endl;
					}
					break;
				case 1: // Descending -> Approach
					activeFlights[i]->decreaseFuel();
					if (speed >= 240) {
						activeFlights[i]->setSpeed(speed - 30);
						cout << endl << "Speed Decreased by 30 at Descending Phase for (arrival)" << endl;
						activeFlights[i]->displayInfo();
						cout << endl;
					}
					else {
						activeFlights[i]->setFlightPhase(2);
						//activeFlights[i]->setSpeed(240);
						cout << endl << "Phase changed from Descending to Approach for (arrival)" << endl;
						activeFlights[i]->displayInfo();
						cout << endl;
					}
					break;
				case 2: // Approach -> Holding or Landing
					activeFlights[i]->decreaseFuel();
					if (activeFlights[i]->isClearanceGranted() && activeFlights[i]->is_runway_assigned() && activeFlights[i]->GetOnRunway() != -1) {
						if (activeFlights[i]->getSpeed() > 30) {
							activeFlights[i]->setSpeed(speed - 60);
							cout << endl << "Speed Decreased by 20 at approaching Phase for (arrival)" << endl;
							activeFlights[i]->displayInfo();
							cout << endl;
							//activeFlights[i]->setClearanceGranted(false);
							//activeFlights[i]->SetOnRunway(-1);
							//activeFlights[i]->setRunwayAssigned(false);
						}
						else {
							activeFlights[i]->setFlightPhase(3); // go to taxi
							uniform_int_distribution<int> dist(15, 30);
							int random_number = dist(rng);
							speed = random_number;
							activeFlights[i]->setSpeed(speed);
							cout << endl << "Phase changed from Landing to Taxi for (arrival)" << endl;
							activeFlights[i]->displayInfo();
							cout << endl;
						}
					}
					else if (activeFlights[i]->getSpeed() > 30) {
						activeFlights[i]->setSpeed(speed - 60);
						cout << endl << "Speed Decreased by 14 at approaching Phase for (arrival)" << endl;
						activeFlights[i]->displayInfo();
						cout << endl;
					}
					break;
				case 3: // Holding -> Try again for approach
					activeFlights[i]->decreaseFuel();
					//if (activeFlights[i]->is_runway_assigned() == false)
						//break;

					if (activeFlights[i]->getSpeed() > 0)
						activeFlights[i]->setSpeed(speed - 6);
					else {
						activeFlights[i]->setSpeed(0);
						activeFlights[i]->setFlightPhase(4);
						cout << endl << "Phase changed from Taxi to Gate for (arrival)" << endl;
						activeFlights[i]->displayInfo();
						cout << endl;
					}
					break;
				case 4: // Landed
					activeFlights[i]->decreaseFuel();
					//if (activeFlights[i]->is_runway_assigned() == false)
						//break;

					activeFlights[i]->setHasLanded(true);
					break;
				}
			}

			// DEPARTURES
			else if (direction == 1) {
				switch (currentPhase) {
				case 6: // Parked -> Taxi
					activeFlights[i]->decreaseFuel();
					if (activeFlights[i]->isClearanceGranted() && activeFlights[i]->is_runway_assigned() && activeFlights[i]->GetOnRunway() != -1) {
						if (speed <= 15)
							activeFlights[i]->setSpeed(speed + 10);
						else {
							//uniform_int_distribution<int> dist(15, 30);
							//int random_number = dist(rng);
							//speed = random_number;
							//activeFlights[i]->setSpeed(speed);
							activeFlights[i]->setFlightPhase(7);
							cout << endl << "Phase changed from Gate to Taxi for (departure)" << endl;
							activeFlights[i]->displayInfo();
							cout << endl;

						}
					}
					else {
						activeFlights[i]->setSpeed(0);
						cout << endl << "Aircraft is at Parking as No Runway has been assigned yet (departure)" << endl;
						activeFlights[i]->displayInfo();
						cout << endl;
					}
					break;
				case 7: // Taxi -> Takeoff
					activeFlights[i]->decreaseFuel();
					if (speed < 30)
						activeFlights[i]->setSpeed(speed + 10);
					else {
						//activeFlights[i]->setSpeed(0);
						cout << endl << "Phase changed, Plane is ready to take off (departure)" << endl;
						activeFlights[i]->displayInfo();
						cout << endl;
					}

					/*if (activeFlights[i]->isClearanceGranted() && activeFlights[i]->is_runway_assigned() && activeFlights[i]->GetOnRunway() != -1) {

						if (activeFlights[i]->getSpeed() <= 30) {
							activeFlights[i]->setSpeed(speed + 4);
							cout << endl << "Speed increased by 4 at Taxi for" << endl;
							activeFlights[i]->displayInfo();
							cout << endl;
						}
						else {
							activeFlights[i]->setSpeed(0);
							activeFlights[i]->setFlightPhase(8);
							cout << endl << "Phase changed from Taxi to Takeoff for" << endl;
							activeFlights[i]->displayInfo();
							cout << endl;

						}

					}
					else {
						activeFlights[i]->setSpeed(0);
						activeFlights[i]->setFlightPhase(6);
						cout << endl << "Phase changed from Taxi to Gate as Runway was not Assigned for" << endl;
						activeFlights[i]->displayInfo();
						cout << endl;
					}/*
					break;
				case 8: // Takeoff -> Ascend
					activeFlights[i]->decreaseFuel();
					if (speed <= 240) {
						activeFlights[i]->setSpeed(speed + 30);
						cout << endl << "Speed incraesed by 30 at Takeoff for (departure)" << endl;
						activeFlights[i]->displayInfo();
						cout << endl;
					}
					else {
						//uniform_int_distribution<int> dist(250, 463);
						//int random_number = dist(rng);
						//speed = random_number;
						//activeFlights[i]->setSpeed(speed);
						activeFlights[i]->setFlightPhase(9);
						cout << endl << "Phase changed from Takeoff to Climb for (departure)" << endl;
						activeFlights[i]->displayInfo();
						cout << endl;
					}
					break;
				case 9: // Ascend -> Cruising
					activeFlights[i]->decreaseFuel();
					if (speed <= 400) {
						activeFlights[i]->setSpeed(speed + 30);
						cout << endl << "Speed incraesed by 30 at Ascend at climb for (departure)" << endl;
						activeFlights[i]->displayInfo();
						cout << endl;
					}
					else {
						activeFlights[i]->setFlightPhase(10);
						//activeFlights[i]->setSpeed(800);
						cout << endl << "Phase changed from Ascend to Crusing for (departure)" << endl;
						activeFlights[i]->displayInfo();
						cout << endl;
					}
					break;
				case 10: // Cruising
					activeFlights[i]->decreaseFuel();
					if (speed <= 600)
						activeFlights[i]->setSpeed(speed + 5);
					else
						activeFlights[i]->setSpeed(speed - 5);

					cout << endl << "Flight is currently Crusing" << endl;
					activeFlights[i]->displayInfo();
					cout << endl;
					break;
				}
			}
		}
		*/
		this_thread::sleep_for(std::chrono::seconds(5));
	}
};

class ATC {
private:
	Runway* runways[3];
	Airline* airlines[6];
	Aircraft* activeFlights[14];
	Radar* radar;
	Dispatcher* dispatcher;
	Faulty* fault;
	simluation* sim;
	RenderWindow& window;
	int activeFlightCount = 0;
	//activeFlights[] <- monitor -> decisions -> updates/errors
	thread flightGenThread;
	thread radarThread;
	thread atcDispatcherThread;
	thread faultHanderlerThread;
	thread simluationThread;
	thread renderFlightsThread;
	thread FlightsDetailsThread;
	thread freeRunwayThread;
	atomic<bool> simulationRunning{ true };

public:

	ATC(RenderWindow& win) : window(win) {
		runways[0] = new Runway(0, "RWY-A");
		runways[1] = new Runway(1, "RWY-B");
		runways[2] = new Runway(2, "RWY-C");

		airlines[0] = new Airline("PIA", 0, 6, 4);   //0
		airlines[1] = new Airline("AirBlue", 0, 4, 4);   //1
		airlines[2] = new Airline("FedEx", 1, 3, 2);   //2
		airlines[3] = new Airline("Pakistan Airforce", 2, 2, 1);   //3
		airlines[4] = new Airline("Blue Dart", 1, 2, 2);   //4
		airlines[5] = new Airline("AghaKhan Air Ambulance", 3, 2, 1);   //5

		radar = new Radar;

		dispatcher = new Dispatcher;

		fault = new Faulty;

		sim = new simluation;

		for (int i = 0; i < 14; i++) {
			activeFlights[i] = NULL;
		}

	}

	void simulation() {

		mutex crt_flight;
		flightGenThread = thread(&ATC::flightGen, this, std::ref(crt_flight));
		radarThread = thread(&ATC::radarCtr, this, std::ref(crt_flight));
		atcDispatcherThread = thread(&ATC::atcDis, this, std::ref(crt_flight));
		faultHanderlerThread = thread(&ATC::faultHnd, this, std::ref(crt_flight));
		simluationThread = thread(&ATC::simlutionThreadFunction, this, std::ref(crt_flight));
		freeRunwayThread = thread(&ATC::freeRunwayFunction, this, std::ref(crt_flight));
		//FlightsDetailsThread = thread(&ATC::activeFlightDetails, this, std::ref(crt_flight));
		//renderFlightsThread = thread(&ATC::renderFlightsFunction, this, std::ref(crt_flight));
		//ATC::renderFlightsFunction(crt_flight);

		int check = 0;
		for (int i = 0; i < 900; ++i) { // 300 seconds = 5 minutes
			if (i - check >= 20) {
				check = i;
				ShowActiveFlights();
				cout << endl;
			}
			cout << "\rSimulation running... Time elapsed: " << i << "s" << flush;
			this_thread::sleep_for(chrono::seconds(1));
		}
		stopSimulation();


		flightGenThread.join();
		radarThread.join();
		atcDispatcherThread.join();
		faultHanderlerThread.join();
		simluationThread.join();
		freeRunwayThread.join();
		//FlightsDetailsThread.join();
		renderFlightsThread.join();
	}

	void stopSimulation() {
		simulationRunning = false;
	}

	void renderFlightsFunction(mutex& crt_flight) {
		while (window.isOpen() && simulationRunning) {

			crt_flight.lock();
			window.setActive(true);
			mainScreen();
			crt_flight.unlock();

			this_thread::sleep_for(chrono::milliseconds(100));

		}
	}

	void mainScreen() {
		static sf::Texture backgroundTexture;
		static bool bgLoaded = backgroundTexture.loadFromFile("D:\\OS-Project\\airportbg.jpg");
		if (!bgLoaded) {
			std::cerr << "Failed to load background\n";
			return;
		}

		sf::Image icon;
		if (!icon.loadFromFile("D:\\OS-Project\\cargo.png")) {
			std::cerr << "Failed to load icon\n";
			// Handle error
			return;
		}
		window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

		// Load aircraft type textures (only once)
		static sf::Texture commercialTexture, cargoTexture, militaryTexture, medicalTexture;
		static bool texturesLoaded = false;
		if (!texturesLoaded) {
			if (!commercialTexture.loadFromFile("D:\\OS-Project\\commercial.png"))
				std::cerr << "Failed to load commercial.png\n";
			if (!cargoTexture.loadFromFile("D:\\OS-Project\\cargo.png"))
				std::cerr << "Failed to load cargo.png\n";
			if (!militaryTexture.loadFromFile("D:\\OS-Project\\military.png"))
				std::cerr << "Failed to load military.png\n";
			if (!medicalTexture.loadFromFile("D:\\OS-Project\\medical.png"))
				std::cerr << "Failed to load medical.png\n";
			texturesLoaded = true;
		}

		sf::Sprite backgroundSprite(backgroundTexture);
		window.clear();
		window.draw(backgroundSprite);

		float width = window.getSize().x;
		float third = width / 3.0f;

		// --- First loop: Render aircraft sprites ---
		Aircraft* ac;

		for (int i = 0; i < 3; i++) {
			ac = runways[i]->getOnRunwayAircraft(activeFlights);

			if (ac != nullptr && ac->is_runway_assigned() && ac->GetOnRunway() != -1) {
				sf::Sprite aircraftSprite;

				switch (ac->getType()) {
				case 0: aircraftSprite.setTexture(commercialTexture); break;
				case 1: aircraftSprite.setTexture(cargoTexture); break;
				case 2: aircraftSprite.setTexture(militaryTexture); break;
				case 3: aircraftSprite.setTexture(medicalTexture); break;
				default: continue;
				}


				if (ac->GetOnRunway() == 0) {

					aircraftSprite.setPosition((600 - ac->getSpeed()) * 2 + 10, 400);
				}
				else if (ac->GetOnRunway() == 1) {

					aircraftSprite.setScale(-1.f, 1.f);
					aircraftSprite.setPosition((ac->getSpeed() - 600) * -2 + 130, 500);
				}
				else if (ac->GetOnRunway() == 2 || ac->getEmergencyCheck()) {

					if (ac->getDirection() == 0) {
						aircraftSprite.setPosition((600 - ac->getSpeed()) * 2 + 10, 615);
					}
					else if (ac->getDirection() == 1) {
						aircraftSprite.setScale(-1.f, 1.f);
						aircraftSprite.setPosition((ac->getSpeed() - 600) * -2 + 130, 615);
					}
				}

				//aircraftSprite.setPosition((third - 10) * i + 100, 150); // <- adjust as needed
				window.draw(aircraftSprite);
			}
		}

		// --- Second loop: Render aircraft details ---
		//for (int i = 0; i < 3; i++) {
		ac = runways[0]->getOnRunwayAircraft(activeFlights);
		if (ac != nullptr) {
			displayInfo1(ac, 90, 90, window); // <- adjust as needed
		}
		ac = runways[1]->getOnRunwayAircraft(activeFlights);
		if (ac != nullptr) {
			displayInfo1(ac, 510, 90, window); // <- adjust as needed
		}
		ac = runways[2]->getOnRunwayAircraft(activeFlights);
		if (ac != nullptr) {
			displayInfo1(ac, 930, 90, window); // <- adjust as needed
		}
		//}

		window.display();
	};

	/*
	void mainScreen() {
		static sf::Texture backgroundTexture;
		static bool bgLoaded = backgroundTexture.loadFromFile("D:\\OS-Project\\airportbg.jpg");
		if (!bgLoaded) {
			std::cerr << "Failed to load background\n";
			return;
		}

		// Load aircraft type textures (only once)
		static sf::Texture commercialTexture, cargoTexture, militaryTexture, medicalTexture;
		static bool texturesLoaded = false;
		if (!texturesLoaded) {
			if (!commercialTexture.loadFromFile("D:\\OS-Project\\commercial.png"))
				std::cerr << "Failed to load commercial.png\n";
			if (!cargoTexture.loadFromFile("D:\\OS-Project\\cargo.png"))
				std::cerr << "Failed to load cargo.png\n";
			if (!militaryTexture.loadFromFile("D:\\OS-Project\\military.png"))
				std::cerr << "Failed to load military.png\n";
			if (!medicalTexture.loadFromFile("D:\\OS-Project\\medical.png"))
				std::cerr << "Failed to load medical.png\n";
			texturesLoaded = true;
		}

		sf::Sprite backgroundSprite(backgroundTexture);
		window.clear();
		window.draw(backgroundSprite);

		float width = window.getSize().x;
		float third = width / 3.0f;

		// --- First loop: Render aircraft sprites ---
		for (int i = 0; i < 14; i++) {
			Aircraft* ac = activeFlights[i];
			if (ac != nullptr) {
				sf::Sprite aircraftSprite;

				switch (ac->getType()) {
				case 0: aircraftSprite.setTexture(commercialTexture); break;
				case 1: aircraftSprite.setTexture(cargoTexture); break;
				case 2: aircraftSprite.setTexture(militaryTexture); break;
				case 3: aircraftSprite.setTexture(medicalTexture); break;
				default: continue;
				}

				if (ac->getFlightPhase() == 0) {
					aircraftSprite.setPosition(1, 331);
					if (ac->getEmergencyCheck())
						aircraftSprite.setPosition(1, 585);
				}
				else if (ac->getFlightPhase() == 1) {
					aircraftSprite.setPosition(95, 331);
					if (ac->getEmergencyCheck())
						aircraftSprite.setPosition(95, 585);
				}
				else if (ac->getFlightPhase() == 2) {
					aircraftSprite.setPosition(600, 331);
					if (ac->getEmergencyCheck())
						aircraftSprite.setPosition(600, 585);
				}
				else if (ac->getFlightPhase() == 3) {
					aircraftSprite.setPosition(1000, 331);
					if (ac->getEmergencyCheck())
						aircraftSprite.setPosition(1000, 585);
				}
				else if (ac->getFlightPhase() == 4) {
					aircraftSprite.setPosition(1200, 331);
					if (ac->getEmergencyCheck())
						aircraftSprite.setPosition(1200, 585);
				}
				else if (ac->getFlightPhase() == 6) {
					aircraftSprite.setPosition(1, 460);
					if (ac->getEmergencyCheck())
						aircraftSprite.setPosition(1, 585);
				}
				else if (ac->getFlightPhase() == 7) {
					aircraftSprite.setPosition(100, 460);
					if (ac->getEmergencyCheck())
						aircraftSprite.setPosition(100, 585);
				}
				else if (ac->getFlightPhase() == 8) {
					aircraftSprite.setPosition(600, 460);
					if (ac->getEmergencyCheck())
						aircraftSprite.setPosition(600, 585);
				}
				else if (ac->getFlightPhase() == 9) {
					aircraftSprite.setPosition(1000, 460);
					if (ac->getEmergencyCheck())
						aircraftSprite.setPosition(1000, 585);
				}
				else if (ac->getFlightPhase() == 10) {
					aircraftSprite.setPosition(1200, 460);
					if (ac->getEmergencyCheck())
						aircraftSprite.setPosition(1200, 585);
				}

				//aircraftSprite.setPosition((third - 10) * i + 100, 150); // <- adjust as needed
				window.draw(aircraftSprite);
			}
		}

		// --- Second loop: Render aircraft details ---
		for (int i = 0; i < 3; i++) {
			Aircraft* ac = runways[i]->getOnRunwayAircraft(activeFlights);
			if (ac != nullptr) {
				displayInfo1(ac, (third - 10) * i + 70, 55, window); // <- adjust as needed
			}
		}

		window.display();
	}
	*/

	void displayInfo1(Aircraft* temp, float x, float y, sf::RenderWindow& window) const {
		static Font font;
		static bool fontLoaded = false;

		if (!fontLoaded) {
			if (!font.loadFromFile("D:\\OS-Project\\ARIALN.ttf")) {
				std::cerr << "Failed to load font\n";
				return;
			}
			fontLoaded = true;
		}

		float lineSpacing = 24.f;
		int line = 0;

		Text text("String", font, 24);  // Example with string, font, and size

		text.setFillColor(sf::Color::Black);

		auto draw = [&](const std::string& str) {
			text.setString(str);
			text.setPosition({ x, y + line * lineSpacing });
			//text.setPosition(x, y + line * lineSpacing);
			window.draw(text);
			line++;
			};

		draw("Flight Details:");
		draw("Flight Number: " + temp->getFlightNumber());
		draw("Type: " + std::string(
			temp->getType() == 0 ? "Commercial" :
			temp->getType() == 1 ? "Cargo" :
			temp->getType() == 2 ? "Military" :
			temp->getType() == 3 ? "Medical" : "Unknown"));
		draw("Emergency Check: " + std::string(temp->getEmergencyCheck() ? "Yes" : "No"));
		draw("Speed: " + std::to_string(temp->getSpeed()) + " knots");
		draw("Fuel: " + std::to_string(temp->getFuel()) + "00 liters");
		draw("Direction: " + std::string(temp->getDirection() == 0 ? "Arriving" : "Departing"));

		draw(temp->is_runway_assigned() ? ("Runway: " + std::string(
			temp->GetOnRunway() == 0 ? "A" :
			temp->GetOnRunway() == 1 ? "B" : "C")) : "Runway not assigned yet");

		std::string phase = "Flight Phase: ";
		switch (temp->getFlightPhase()) {
		case 0: phase += "Holding"; break;
		case 1: phase += "Approach"; break;
		case 2: phase += "Landing"; break;
		case 3: phase += "Taxiing (Arrival)"; break;
		case 4: phase += "At Gate (Arrival)"; break;
		case 6: phase += "At Gate (Departure)"; break;
		case 7: phase += "Taxiing (Departure)"; break;
		case 8: phase += "Takeoff Roll"; break;
		case 9: phase += "Climb"; break;
		case 10: phase += "Cruise"; break;
		default: phase += "Unknown Phase"; break;
		}
		draw(phase);

		draw("Avionics Violation: " + std::string(temp->getAVN() ? "ON" : "OFF"));
	}

	void ShowActiveFlights() {
		system("cls");  // For Windows (clears the console)
		std::cout << "========== ACTIVE FLIGHTS ==========\n\n";
		cout << "Active Flight count " << activeFlightCount << endl << endl;
		for (int i = 0; i < 14; i++) {
			if (activeFlights[i] != NULL)
				activeFlights[i]->displayInfo();
		}

		//std::cout << "\n(Press Enter to return to simulation...)\n";
		//std::cin.ignore();
	}

	void simlutionThreadFunction(mutex& crt_flight) {

		//cout << "Running Simulation Thread" << endl;
		while (simulationRunning) {

			crt_flight.lock();
			sim->simulationLoop(runways, activeFlights);
			crt_flight.unlock();

			this_thread::sleep_for(chrono::seconds(1));

		}
	}

	void flightGen(mutex& crt_flight) {

		//cout << "Running Flight Gen Thread" << endl;

		int northTimer = 0;
		int southTimer = 0;
		int eastTimer = 0;
		int westTimer = 0;

		while (simulationRunning) {
			//cout << "Inside" << endl;
			crt_flight.lock();
			if (activeFlightCount >= 14) {
				crt_flight.unlock();
				return;
			}

			int direction = -1;
			int flight_type = -1;  //0-->international or 1-->domestic

			if (northTimer >= 3) {
				direction = 0; // North arrival
				northTimer = 0;
				flight_type = 0;
			}
			else if (southTimer >= 2) {
				direction = 0; // South arrival
				southTimer = 0;
				flight_type = 1;
			}
			else if (eastTimer >= 2.5) {
				direction = 1; // East departure
				eastTimer = 0;
				flight_type = 0;
			}
			else if (westTimer >= 4) {
				direction = 1; // West departure
				westTimer = 0;
				flight_type = 1;
			}
			else {
				// Skip generation this minute
				crt_flight.unlock();
				this_thread::sleep_for(chrono::seconds(1));

				// Update timers
				northTimer++;
				southTimer++;
				eastTimer++;
				westTimer++;
				continue;
			}

			uniform_int_distribution<int> dist(0, 5);
			int random_number = dist(rng);
			//int airline_index = rand() % 6;
			int airline_index = random_number;
			if (airlines[airline_index]->getCurrentActiveAircrafts() < airlines[airline_index]->getTotalActiveAircrafts() && activeFlightCount < 14) {

				/*cout << northTimer << endl;
				cout<<southTimer<<endl;
				cout<<eastTimer<<endl;
				cout<<westTimer<<endl;
				cout << flush;
				this_thread::sleep_for(chrono::seconds(1));*/


				const string flight_number = airlines[airline_index]->getName() + "-" + to_string(airline_index) + "-" + to_string(100) + to_string(activeFlightCount);
				int type = airlines[airline_index]->getType();
				int speed = -1;
				int flight_phase = -1;
				bool emergency_check = false;

				uniform_int_distribution<int> dist(0, 99);
				int random_number = dist(rng);
				int prob_val = random_number;

				if (direction == 0) {

					if (flight_type == 0) {

						if (prob_val < 10) {
							emergency_check = true;
						}

					}
					else if (flight_type == 1) {

						if (prob_val < 5) {
							emergency_check = true;
						}

					}

				}
				else if (direction == 1) {

					if (flight_type == 0) {

						if (prob_val < 15) {
							emergency_check = true;
						}

					}
					else if (flight_type == 1) {

						if (prob_val < 20) {
							emergency_check = true;
						}

					}

				}

				if (direction == 0) {
					flight_phase = 0;
					//uniform_int_distribution<int> dist(400, 600);
					//int random_number = dist(rng);
					//speed = random_number;
					speed = 600;
				}
				else if (direction == 1) {
					flight_phase = 6;
					speed = 0;
				}

				/*if (direction == 0) {
					uniform_int_distribution<int> dist(0, 4);
					int random_number = dist(rng);
					flight_phase = random_number; //[0,4]
				}
				else if (direction == 1) {
					uniform_int_distribution<int> dist(6, 10);
					int random_number = dist(rng);
					flight_phase = random_number; //[6,10]
				}

				switch (flight_phase) {
				case 0:{
					uniform_int_distribution<int> dist(400, 600);
					int random_number = dist(rng);
					speed = random_number;
					//speed = rand() % (600 - 400 + 1) + 400;
					break;}
				case 1:{
					uniform_int_distribution<int> dist(240, 290);
					int random_number = dist(rng);
					speed = random_number;
					//speed = rand() % (290 - 240 + 1) + 240;
					break;}
				case 2:{
					speed = 240;
					break;}
				case 3:{
					uniform_int_distribution<int> dist(15, 30);
					int random_number = dist(rng);
					speed = random_number;
					//speed = rand() % (30 - 15 + 1) + 15;
					break;}
				case 4:{
					speed = 0;
					break;}
				case 6:{
					speed = 0;
					break;}
				case 7:{
					uniform_int_distribution<int> dist(15, 30);
					int random_number = dist(rng);
					speed = random_number;
					//speed = rand() % (30 - 15 + 1) + 15;
					break;}
				case 8:{
					speed = 0;
					break;}
				case 9:{
					uniform_int_distribution<int> dist(250, 463);
					int random_number = dist(rng);
					speed = random_number;
					//speed = rand() % (463 - 250 + 1) + 250;
					break; }
				case 10: {
					uniform_int_distribution<int> dist(800, 900);
					int random_number = dist(rng);
					speed = random_number;
					//speed = rand() % (900 - 800 + 1) + 800;
					break;
				}
				}*/

				activeFlights[activeFlightCount] = airlines[airline_index]->createAircraft(flight_number, type, direction, flight_phase, speed, airline_index, flight_type, emergency_check);

				/*if (activeFlights[activeFlightCount]->getDirection() == 0 && (activeFlights[activeFlightCount]->getFlightPhase() == 3 || activeFlights[activeFlightCount]->getFlightPhase() == 4)) {
					activeFlights[activeFlightCount]->setHasLanded(1);
				}

				if (activeFlights[activeFlightCount]->getDirection() == 1 && (activeFlights[activeFlightCount]->getFlightPhase() == 9 || activeFlights[activeFlightCount]->getFlightPhase() == 10)) {
					activeFlights[activeFlightCount]->setHasDepartured(1);
				}*/

				activeFlightCount++;
				cout << endl << "Total Aircraft Count: " << activeFlightCount << endl;
				activeFlights[activeFlightCount - 1]->displayInfo();
				cout << endl;
			}
			crt_flight.unlock();
			this_thread::sleep_for(chrono::seconds(1));
		}

	}

	void radarCtr(mutex& crt_flight) {

		//cout << "Running Radar Thread" << endl;

		while (simulationRunning) {

			crt_flight.lock();
			radar->RadarMainLoop(activeFlights, activeFlightCount, airlines);
			crt_flight.unlock();

			this_thread::sleep_for(chrono::seconds(10));
		}

	};

	void atcDis(mutex& crt_flight) {

		//cout << "Running Dispatcher Thread" << endl;


		while (simulationRunning) {

			crt_flight.lock();
			dispatcher->DispatcherMainLoop(activeFlights, activeFlightCount, runways);
			crt_flight.unlock();

			this_thread::sleep_for(chrono::seconds(5));

		}
	}

	void faultHnd(mutex& crt_flight) {

		//cout << "Running Fault Thread" << endl;


		while (simulationRunning) {

			crt_flight.lock();
			for (int i = 0; i < 6; i++) {
				if (airlines[i] != NULL) {
					airlines[i]->payFines();
				}
			}
			fault->FaultMainLoop(activeFlights, activeFlightCount);
			crt_flight.unlock();

			this_thread::sleep_for(chrono::seconds(10));

		}

	}

	void freeRunwayFunction(mutex& crt_flight) {

		while (simulationRunning) {

			crt_flight.lock();
			dispatcher->freeRunway(activeFlights, runways);
			crt_flight.unlock();

			this_thread::sleep_for(chrono::seconds(2));

		}
	}
};

/*Thread	Responsibility
flightGeneratorThread	Spawns random flights (arrival/departure/emergency/cargo), adds to activeFlights[]
radarThread	Constantly checks aircraft speed vs. phase, triggers AVNs
atcDispatcherThread	Assigns runways, handles land/takeoff permission, reschedules if runways busy
faultThread	Randomly triggers ground faults during taxi/gate phase
mainThread	Timer, UI/console updates, terminates after 5 minutes*/


int main() {
	//sf::RenderWindow window(VideoMode::getDesktopMode(), "ATC System");
	sf::RenderWindow window(sf::VideoMode(1366, 768), "Airport Simulation");
	//window.setActive(false);  // Deactivate in main thread

	srand(time(0));
	ATC atc(window);

	std::thread simThread([&]() {
		atc.simulation();
		});

	sf::Clock clock;
	std::mutex render_mutex;

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
		}

		if (clock.getElapsedTime().asMilliseconds() > 100) {
			render_mutex.lock();
			atc.mainScreen();
			render_mutex.unlock();

			clock.restart();
		}
	}

	// Stop simulation when window is closed
	atc.stopSimulation(); // A method you add to signal shutdown to all threads

	simThread.join(); // Wait for all simulation threads to finish

	return 0;
}
