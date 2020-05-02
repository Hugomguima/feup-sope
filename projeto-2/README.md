# Computer Access to Bathrooms

- **Short Description**: Client-server application capable of handling conflict situations when accessing shared zones
- **Environment**: Linux
- **Tools Used**: C
- **Institution**: [FEUP](https://sigarra.up.pt/feup/en/web_page.Inicial)
- **Course**: [MIEIC](https://sigarra.up.pt/feup/en/cur_geral.cur_view?pv_curso_id=742&pv_ano_lectivo=2019)
- **Curricular Unit**: [SOPE](https://sigarra.up.pt/feup/en/ucurr_geral.ficha_uc_view?pv_ocorrencia_id=436440) (Operating Systems)

## Group Members
- Diogo Samuel Fernandes (up201806250@fe.up.pt)
- Hugo Guimarães (up201806490@fe.up.pt)
- Telmo Baptista (up201806554@fe.up.pt)

## Program usage

### Compile
The compiling part can be done by running the command `make` to compile both server and client, if it is intended to compile only one side of the program then specific rules can be used to do so, as shown below

Compile everything
```sh
make
```
or
```sh
make all
```

Compile server side only
```sh
make server
```

Compiler client side only
```sh
make client
```

Compile shared libraries
```sh
make shared
```

### Cleanup
```sh
make clean
```

### Run
The executable files are `./bin/` directory after you run the command `make` in terminal.

To run the server, you must specify the time the server will be executing in the flag `-t` and the name of the public channel from which the server will accept requests, can additionally specify the maximum number of threads and the capacity of the bathroom
```sh
./bin/Q1 <-t nsecs> [-l nplaces] [-n nthreads] fifoname
```
or can be run via the symbolic link created by `make`
```sh
./Q1 <-t nsecs> [-l nplaces] [-n nthreads] fifoname
```

To run the client, you must specify the time the client will be executing in the flag `-t` and the name of the public channel of the server to which the client will send requests
```sh
./bin/U1 <-t nsecs> fifoname
```
or can be run via the symbolic link created by `make`
```sh
./U1 <-t nsecs> fifoname
```

## Description
The aim of the project was to develop a client-server application capable of dealing with conflict situations when accessing shared areas.  
The shared area is a bathroom with several unisex seats, controlled by a ***Q*** process (server) to which requests for user access are addressed.  
Access requests are sent through a ***U*** multithreaded process (client), and the time required by the interested party to be in a place of the sanitary facilities is indicated by it. Orders will stay in a service queue until they have a turn, and at such time, the respective user accesses a place on the premises during the requested time, under the control of the ***Q*** server, then the resource is released to another user.

### Flag Description
In the server program ***Q***:
- `-t nsecs` - approximate number of seconds that the program should work
- `-l nplaces` - bathroom capacity
- `-n nthreads` – maximum number of thread to fulfil requests
- `fifoname` – name of the public channel (***FIFO***) to be created by the server to fulfil requests

In the client program ***U***:
- `-t nsecs` - approximate number of seconds that the program should work
- `fifoname` – public channel name (***FIFO***) for communication with the server

## Synchronization mechanisms

<img src="./images/sync_system.png" width="800px" align="center">

To solve synchronization issues we opted to use two public semaphores (represented above by the color green) and one private (represented above by the color red).

## Implementation details

## Parsing

In both client and server we receive and parse the input coming from the command line in the file [client/src/parse.c](./client/src/parse.c) or [server/src/parse.c](./server/src/parse.c).

## Alarm

To determine how long each process will be running we set up one alarm in client and server. 
The process will then receive a signal SIGALRM when the elapsed time has equal to the time passed as argument.
In the signal handler function, the one variable will be set to have the value 0, so the main loop will stop and the program will end.
