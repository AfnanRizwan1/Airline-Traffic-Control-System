🛫 AirControlX
AirControlX is a fully-featured simulation of an Automated Air Traffic Control System (ATCS) developed using C++ as part of an Operating Systems course project.

The system mimics the operation of a multi-runway international airport — managing flight scheduling, runway allocation, airspace violation monitoring, and animated flight transitions — while implementing core OS concepts such as multithreading, synchronization, inter-process communication, and process scheduling.

✨ Key Features
✈️ Flight Scheduling & Priority Management
Handles incoming and outgoing flights based on direction, aircraft type, and urgency (e.g., emergency, VIP).

🛬 Runway Allocation with Concurrency Control
Ensures one-at-a-time access to runways using mutexes and semaphores.

⚠️ Violation Detection System
Monitors aircraft speeds and phases to detect and generate Airspace Violation Notices (AVNs).

💳 Payment Processing Simulation
Simulates fine payments for violations using a mock StripePay system with communication between processes.

📊 Analytics Dashboard
Displays real-time flight data, violation tracking, and aircraft status updates.

🎮 Graphical Simulation
Built using SFML, the simulation visually represents aircraft states (Taxiing, Takeoff, Landing, etc.), runway usage, and ATC operations in real-time.

🧠 Concepts & Technologies Used
C++
POSIX Threads
Semaphores & Mutexes
Inter-Process Communication (Pipes)
Custom Scheduling Algorithms
SFML (Simple and Fast Multimedia Library)
Modular Architecture
Real-Time Simulation
