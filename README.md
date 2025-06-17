AirControlX ✈️
A complete simulation of an Automated Air Traffic Control System (ATCS), built from scratch in C++ as a course project. AirControlX replicates the complex operations of a multi-runway international airport — from real-time flight scheduling to emergency handling and animated simulation.

Features 🚀
Multi-Runway Scheduling: Assigns flights to RWY-A, RWY-B, or RWY-C based on direction and type.
Emergency & Priority Handling: Flights like MedEvac or VIPs are prioritized and preempt lower-tier traffic.
Violation Detection System (AVN): Monitors aircraft speed, position, and behavior — auto-generates fines for rule violations.
StripePay Simulation: Simulates a payment system for airspace violations with process communication.
Live Aircraft Animation: Aircraft states like taxiing, takeoff, landing, and parking are visualized with SFML.
Real-Time Flight States: Tracks each aircraft’s transition from holding to approach, landing, and departure.

Technologies Used 🛠️
Language: C++
Graphics Library: SFML (Simple and Fast Multimedia Library)
Concurrency: POSIX Threads, Semaphores, Mutexes
IPC: Named Pipes for inter-process communication
System Design: Modular simulation with real-time transitions and analytics

How It Works 🎮
Clone the repository:

bash
Copy
Edit
git clone https://github.com/AfnanRizwan1/Airline-Traffic-Control-System.git  
Build the project in your preferred IDE or compile from terminal using g++.
Ensure SFML is correctly linked for the graphical modules.

Simulate a full 5-minute airport cycle with takeoffs, landings, violations, and payments — all visualized and logged in real time.

Components 🧩
ATC Controller: Core logic for flight tracking, scheduling, and rule enforcement

AVN Generator: Monitors violations, calculates penalties, and generates challans

Airline Portal: View and manage active/historical AVNs

StripePay: Simulates payment processing for AVN fines

SFML Visualizer: Animates runway states and aircraft positions in real time

Requirements 📦
SFML Library: Required for rendering graphics

C++ Compiler: GCC, Clang, or MSVC

Unix-based OS (Recommended): Due to usage of POSIX threads and pipes

Future Enhancements 🌟
Real-time map with zoom and radar view

API integration with real-world flight data

Web-based airline dashboard

Dynamic weather effects and runway visibility changes

Contributing 🤝
Open to contributions and improvements! Fork the repository and submit a pull request with your ideas or fixes.

License 📜
This project is licensed under the MIT License – see the LICENSE file for details.
