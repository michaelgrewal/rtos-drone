# rtos-drone
RTOS Drone Quadcopter project COMP4900 with QNX Neutrino

- Main spawns all necessary processes: Flight Controller, Display, Properllers, Sensors.
- Flight Controller (Server) shares memory chunk with Display Process
- Display periodically reads from shared memory and updates HUD
- 4x Propeller (Client) threads each allocate small shared memory to share with respective Sensor
- Each Propeller writes it's speed to it's shared memory, then the Sensors read it and transmit a Pulse back to Flight Controller (Server).
- Flight Controller writes the received speeds into the shared memory, such that Display can update the values.
