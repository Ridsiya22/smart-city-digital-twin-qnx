# Smart City Digital Twin – QNX RTOS

## 1. Project Overview

The **Smart City Digital Twin – Real-Time Synchronization Engine** is a QNX RTOS-based application developed on a **Raspberry Pi 4 Model B**.

The project creates a real-time digital representation of multiple smart-city subsystems and demonstrates core **QNX RTOS concepts**, including:

- Real-time thread scheduling
- Inter-process communication
- POSIX shared memory
- Mutex-based synchronization
- Fault monitoring
- Process-death detection
- Sequence-gap detection
- Real-time latency measurement
- Deadlock avoidance

The current prototype uses **software-simulated sensor data** to validate the real-time architecture. Physical sensor integration can be added without changing the higher-level synchronization and monitoring architecture.

---

## 2. Smart City Subsystems

The Digital Twin represents four major subsystems:

### Environment
- Temperature
- Atmospheric pressure

### Traffic
- Distance
- Vehicle count
- Traffic level

### Energy
- Voltage
- Current
- Power

### Water
- Water level
- Flow rate

---

## 3. Objectives

The main objectives of the project are:

- Build a real-time Smart City Digital Twin using QNX RTOS.
- Demonstrate QNX native message-passing IPC.
- Implement priority-based real-time scheduling.
- Maintain a synchronized digital twin state.
- Measure sensor synchronization latency.
- Detect stale sensor data.
- Detect missing sensor messages using sequence numbers.
- Detect process-death events using QNX process-manager notifications.
- Demonstrate safe deadlock avoidance.
- Analyze real-time system behavior using QNX Momentics System Profiler.

---

## 4. QNX RTOS Concepts Used

The project demonstrates the following QNX and POSIX concepts:

- QNX microkernel architecture
- POSIX threads
- `pthread_create()`
- `pthread_join()`
- `SCHED_RR` scheduling
- Priority-based execution
- QNX Native IPC
- `MsgSend()`
- `MsgReceive()`
- `MsgReply()`
- QNX Channels
- QNX Pulses
- Process-death notification
- POSIX shared memory
- Process-shared POSIX mutex
- `mmap()`
- `clock_gettime()`
- Real-time latency measurement

---

## 5. Process and Thread Priorities

The application uses different priorities according to the importance of each function:

| Component | Priority |
|-----------|----------|
| Twin Synchronizer | 30 |
| Fault Monitor | 25 |
| Sensor Acquisition | 20 |
| Dashboard | 15 |
| CLI | 15 |

The **Twin Synchronizer** is given the highest application priority because it is responsible for receiving sensor messages and updating the Digital Twin state.

---

## 6. Inter-Process Communication

QNX native message passing is used for communication between the sensor acquisition layer and the Twin Synchronizer.

The implementation uses:

- `MsgSend()` to send sensor data.
- `MsgReceive()` to receive sensor data.
- `MsgReply()` to complete the request/reply transaction.

Each sensor message contains:

- Sensor ID
- Sensor name
- Sensor values
- Sequence number
- Timestamp

Sequence numbers are used to identify missing sensor updates.

---

## 7. Digital Twin State

The latest state of the smart-city system is maintained in a shared `TwinState` structure.

The shared state contains:

- Environment information
- Traffic information
- Energy information
- Water information
- Total message count
- Sequence-gap count
- Synchronization latency
- Individual sensor fault status
- Acquisition process status
- Synchronizer process status

A **process-shared POSIX mutex** is used to protect access to the shared Digital Twin state.

---

## 8. Sensor Acquisition

The acquisition layer contains separate threads for each subsystem:

- Environment thread
- Traffic thread
- Energy thread
- Water thread

Each thread:

1. Generates or reads sensor data.
2. Adds a sequence number.
3. Records a timestamp.
4. Sends the data to the Synchronizer using QNX IPC.
5. Waits for the next acquisition period.

The acquisition threads currently use `SCHED_RR` scheduling with priority 20.

---

## 9. Sensor Simulation

The current implementation uses **software-simulated sensor values**.

The simulated values are:

- Dynamically updated.
- Bounded within predefined limits.
- Generated to represent realistic changes in sensor conditions.

Simulation is used to validate the QNX real-time architecture independently of physical sensor communication.

The architecture allows the simulated acquisition functions to be replaced by physical sensor interfaces in future development without changing the IPC, synchronization, fault-monitoring, or Digital Twin layers.

---

## 10. Physical Hardware Interface

The prototype is developed for:

- Raspberry Pi 4 Model B
- 2 GB RAM
- QNX RTOS

Physical sensors prepared for integration include:

- **BMP280** – Environment monitoring
- **INA219** – Voltage and current monitoring
- **HC-SR04** – Traffic/distance monitoring

The current prototype does not claim the simulated values as physical sensor measurements.

---

## 11. Real-Time Monitoring

The system monitors important real-time parameters such as:

- Synchronization latency
- Message-processing timing
- Sensor sequence numbers
- Sensor data freshness
- Fault status
- Process status
- Thread execution
- CPU utilization

QNX Momentics System Profiler can be used to analyze the runtime behavior of the system.

---

## 12. Fault Monitoring

The Fault Monitor is responsible for detecting abnormal system conditions.

### Stale Data Detection

A sensor is considered stale when its data has not been updated within the configured timeout period.

The current timeout is:

```text
3000 ms
