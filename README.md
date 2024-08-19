# C-based Air Traffic Control System

## About

A POSIX-compliant C-based multi-threaded air traffic control system supporting multiple concurrent requests, synchronisation and mutual exclusion using semaphores and mutexes for efficiently managing multiple airports, ensuring clash-free arrival, departure of cargo and passenger planes and assigning runways to planes based on their load capacity. Diagrammatic depiction of passenger plane processes:

<img width="581" alt="stack" src="https://github.com/user-attachments/assets/15a90ee1-b77a-4739-b66e-4c075d4261be">

## Key Features
- Multi-threading support : Each multi-threaded airport process creates a new thread that finds a runway the best-fit selection algorithm.
- Synchronization: Synchronization of runways is implemented using semaphore and mutex.
- Concurrency : Can support 10+ concurrent processes with 5+ airports managed in the ATC system.
- Inter-process Communication (IPC) : POSIX compliant IPC via pipes, message queues and shared memory.

## Getting started

### Navigating to root directory
```sh
cd /path/to/the/root
```

### Compiling (GCC compiler) into executable files
```sh
gcc -o airport.out airport.c
gcc -o plane.out plane.c
gcc -o cleanup.out cleanup.c
gcc -o airtrafficcontroller.out airtrafficcontroller.c
```

### Running executable files
```sh
airport.out
plane.out
cleanup.out
airtrafficcontroller.out
```
## How to run?

### For plane process
Upon executing the plane process, enter a positive integer (within the bounds from 1 to 10, both 1 and 10 inclusive) when prompted :
```sh
Enter Plane ID:
Enter Type of Plane: (1: Passenger Plane, 0: Cargo Plane)
Enter Number of Occupied Seats:
Enter Weight of Your Luggage:
```

Upon receiving the intimation that the deboarding/unloading is completed, the plane process displays the following message before terminating itself :
```sh
Plane <Plane ID> has successfully traveled from Airport <Airport Number of Departure Airport> to Airport <Airport Number of Arrival Airport>!
```
### For ATC process
Upon executing the ATC process, enter a positive integer (within the bounds from 2 to 10, both 2 and 10 inclusive) when prompted :
```sh
Enter the number of airports to be handled/managed:
```

When a plane travels from one airport to another, the air traffic controller keeps a track of this in the AirTrafficController.txt file, by appending the following message at the end of the file for each plane journey :
```sh
Plane <Plane ID> has departed from Airport <Airport Number of Departure Airport> and will land at Airport <Airport Number of Arrival Airport>.
```
### For cleanup process
The cleanup process which keeps running along with the other processes and only one instance of this program is executed and will keep displaying a message as :
```sh
Do you want the Air Traffic Control System to terminate? (Y for Yes and N for No)
```

### For airport process
Upon executing the airport process, enter a positive integer when prompted :
```sh
Enter Airport Number: (within the bounds from 1 to 10, both 1 and 10 inclusive)
Enter number of Runways: (within the bounds from 1 to 10, both 1 and 10 inclusive)
Enter loadCapacity of Runways (give as a space separated list in a single line):
```

A message is sent to the air traffic controller specifying that a plane with plane ID has successfully completed onboarding/deboarding :
```
Plane <Plane ID> has completed boarding/loading and taken off from Runway No. X of Airport No. Y. //Departure
Plane <Plane ID> has landed on Runway No. X of Airport No. Y and has completed deboarding/unloading. //Arrival
```
