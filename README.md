Smart City Digital Twin – QNX RTOS

A real-time Smart City Digital Twin synchronization engine built with QNX RTOS on Raspberry Pi 4. The project demonstrates real-time communication, synchronization, scheduling, and fault-monitoring concepts using QNX.

Overview

The system models four smart-city subsystems:

🌡️ Environment
🚗 Traffic
⚡ Energy
💧 Water

Sensor data is currently software-simulated to validate the real-time QNX architecture.

Architecture
              Smart City Digital Twin
                        │
                        ▼
                  QNX RTOS
                        │
       ┌────────────────┼────────────────┐
       ▼                ▼                ▼
 Acquisition      Synchronizer      Fault Monitor
  Priority 20     Priority 30       Priority 25
       │                │                │
       │ MsgSend()      │                │
       └───────────────►│                │
                        ▼                │
                  TwinState             │
              POSIX Shared Memory       │
                 + Mutex                │
                        ▲                │
                        └────────────────┘
Key Features
QNX native IPC using MsgSend(), MsgReceive() and MsgReply()
Multi-threaded sensor acquisition
Priority-based SCHED_RR scheduling
POSIX shared memory
Process-shared mutex synchronization
Real-time synchronization latency measurement
Sequence-gap detection
Stale-data detection
QNX process-death notification using Pulses
Deadlock avoidance demonstration
QNX Momentics System Profiler support
Technologies
Component	Technology
RTOS	QNX 8.0
Hardware	Raspberry Pi 4 Model B
Language	C
IDE	QNX Momentics
IPC	QNX Native Message Passing
Synchronization	POSIX Mutex
Shared State	POSIX Shared Memory
Scheduling	SCHED_RR
Project Structure
smart-city-digital-twin-qnx/
│
├── include/
│   ├── common.h
│   ├── messages.h
│   ├── twin_state.h
│   └── fault_monitor.h
│
├── src/
│   ├── acquisition/
│   ├── synchronizer/
│   ├── fault_monitor/
│   ├── deadlock/
│   ├── dashboard/
│   ├── cli/
│   └── smart_city_digital_twin.c
│
├── dashboard/
├── docs/
├── Makefile
└── README.md
Build

For the Raspberry Pi AArch64 target:

make -j4 all

Executable:

build/aarch64le-debug/smart_city_digital_twin
Run on QNX

Start the modules in separate terminals.

Synchronizer
./smart_city_digital_twin sync
Acquisition
./smart_city_digital_twin acq
Fault Monitor
./smart_city_digital_twin fault
Deadlock Demonstration
./smart_city_digital_twin deadlock
Real-Time Data Flow
Sensor Simulation
       ↓
Acquisition Threads
       ↓
MsgSend()
       ↓
Synchronizer
       ↓
MsgReceive()
       ↓
TwinState
       ↓
Fault Monitor
Fault Monitoring

The Fault Monitor detects:

Stale sensor data
Sequence gaps
Process-death events

Sensor data is considered stale when no update is received within the configured timeout.

Current Status

✅ QNX RTOS application running on Raspberry Pi 4
✅ Multi-threaded acquisition
✅ QNX IPC implemented
✅ Shared Digital Twin state implemented
✅ Fault monitoring implemented
✅ Death Pulse mechanism integrated
✅ Deadlock avoidance demonstration implemented
🚧 Physical sensor integration and dashboard enhancements are future extensions

A digital twin is useful only when its real-time state can be trusted.

