# rtos-drone
RTOS Drone Quadcopter project COMP4900 with QNX Neutrino

Instructions:
1. Open Momentics with QNX Neutrino Target VM ready and connected.
2. Import source code into Momentics project.
3. Right click on "rtos-drone" folder in Project Explorer.
4. Click "Build Project".
5. Copy all executable binaries into target VM.
6. Run "./main" on target VM.

Explanation:

- Main spawns all necessary processes: Flight Controller, Display, Properllers, Sensors.
- Flight Controller (Server) shares memory chunk with Display Process
- Display periodically reads from shared memory and updates HUD
- 4x Propeller (Client) threads each allocate small shared memory to share with respective Sensor
- Propellers request target speed from Flight Controller, and Flight Controller replies.
- Propellers adjust speed to match target and react accordingly.
- Each Propeller writes it's speed to it's shared memory, then the Sensors read it and transmit a Pulse back to Flight Controller (Server).
- Flight Controller writes the received speeds into the shared memory, such that Display can update the values.
- Random gusts of wind are delivered to propellers that changes their speeds, and the system reacts accordingly to restore target speeds.
