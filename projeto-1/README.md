# Simpledu
- **Short Description**: Tool to estimate file space usage
- **Environment**: Unix
- **Tools used**: C
- **Institution**: [FEUP](https://sigarra.up.pt/feup/en/web_page.Inicial)
- **Course**: [MIEIC](https://sigarra.up.pt/feup/en/cur_geral.cur_view?pv_curso_id=742&pv_ano_lectivo=2019)
- **Curricular Unit**: [SOPE](https://sigarra.up.pt/feup/en/ucurr_geral.ficha_uc_view?pv_ocorrencia_id=436440) (Operating Systems)

## Group Members
- Diogo Samuel Fernandes (up201806250@fe.up.pt)
- Hugo Guimarães (up201806490@fe.up.pt)
- Telmo Baptista (up201806554@fe.up.pt)

## Program usage

### Compile
```sh
make
```

### Cleanup
```sh
make clean
```

### Run
The executable file is in `./bin/` directory after you run the command `make` in terminal.
Note that the flag `-l` or `--count-links` must be present.
```sh
./bin/simpledu -l [path] [-a] [-b] [-B size] [-L] [-S] [--max-depth=N]
```
or can be run via the symbolic link created by `make`
```sh
./simpledu -l [path] [-a] [-b] [-B size] [-L] [-S] [--max-depth=N]
```

## Description
The aim of the project was to develop a tool to summarize the use of disk space in a file or directory, the information to be made available must include files and subdirectories that may be contained therein.

This tool has as reference the command `du`, which presents information about the disk space used by files and directories.

### Flag Description
- `-a` or `--all` - the displayed information also concerns files
-  `-b` or `--bytes` - displays the actual number of data bytes (files) or allocated (directories)
- `-B`, `--block-size=SIZE` - defines the size (bytes) of the block for representation purposes
- `-l`, `--count-links` - count the same file multiple times;
- `-L`, `--dereference` - follow symbolic links;
- `-S`, `--separate-dirs` - the displayed information does not include the size of the subdirectories;
- `--max-depth=N` - limits the displayed information to N (0.1, ...) levels of directory depth

## Features
Every functionality mentioned bellow is full working.

#### Plan
- [x] Receive, process and save command line arguments and environment variables
- [X] Add registration messages as new features are implemented and validate the correction of both
- [x] Start by choosing only files and presenting the desired information (bytes and blocks)
  - [x] Consider that the `-L` (or `--dereference`) option is active so that it is not necessary to distinguish symbolic links from regular files
- [x] Distinguish between files and symbolic links and present different results depending on the `-L` option
- [x] Consider entries that are directories, but limit the analysis to one level (`--max-depth=1`)
- [x] Create a new process by subdirectory and try to pass the correct arguments to it
  - [x] The arguments will be the same except for the path (path / entry) and, eventually, the maximum allowed depth level (`--max depth=N-1`)
  - [x] Assume that the `-S` (or `--separate-dirs`) option is active so that it is not necessary to cumulatively consider the size of the subdirectories
- [x] Create pipes to communicate the size of a given subdirectory to the parent process and thus correctly present cumulative results, including for subdirectories

#### `du` functionalities
- [x] shows the space occupied in number of blocks of 1024 bytes
- [x] only lists directories
- [x] does not follow symbolic links
- [ ] counts each file only once
- [x] cumulatively displays the size of included subdirectories and files
- [x] does not restrict the depth levels in the directory structure

#### Architectural requirements
- Each process
  - [x] analyses only one directory
  - [x] create a process for each subdirectory
  - [x] adjust arguments passed to child processes
- [x] The parent process (first to be executed) must always wait for the termination of all child processes before finishing its execution
- [x] use the same code (without changes) regardless of being the main process or not
- [x] the total size of each of the subdirectories must be communicated to the parent process through a pipe (without a name) created for each of the child processes
- [x] the parent process must send a `SIGSTOP` signal to all child processes that are running when it receives a SIGINT signal  
  The sending of the next signal depends on the confirmation (`SIGTERM`) or not (`SIGCONT`) by the user regarding the termination of the program

#### Functional requirements
- [x] Logs
- Logs functionalities
  - [x] Log creation and termination of processes
  - [x] Log sent and received signals
  - [x] Log sent and received data from pipes
  - [x] Log files and directories analysed by each process
  - Output format: `instant - pid - action - info`
    - [x] instant - time immediately preceding the record, measured in milliseconds and to 2 decimal places, with reference to the time when the program started executing
    - [x] pid - process identifier that registers the line, with a fixed space for 8 digits
    - [x] action - description of the type of event: `CREATE`, `EXIT`, `RECV_SIGNAL`, `SEND_SIGNAL`, `RECV_PIPE`, `SEND_PIPE` and `ENTRY`
    - info - additional information for each of the actions
      - [x] `CREATE` - the command line arguments
      - [x] `EXIT` – exit status
      - [x] `RECV_SIGNAL` - the received signal (for example, `SIGINT`)
      - [x] `SEND_SIGNAL` - the signal sent followed by the pid of the process for which it is intended
      - [x] `RECV_PIPE` - the message sent
      - [x] `SEND_PIPE` - the message received
      - [x] `ENTRY` - number of bytes (or blocks) followed by the path

#### Additional Features
##### Interruption by the user
- When user sends `SIGINT` signal (`CTRL + C`):
  - [x] Entire program suspended
  - Upon confirmation of termination
    - [x] Finish all pending operations
    - [x] End program
  - Upon confirmation of continuation
    - [x] Resume operations immediately

## Precision errors

- We use `gettimeofday()` to get the clock time. On our system this clock have µs precision, but this is not guaranteed. We used this function instead of using `clock()` because in our system, the first one provided us more accuracy than the second one.

- The results of command simpledu when the flag -B / --block-size is set can have some imprecisions.

## Extra Functionalities

- Flags passed by the command line don't require to be in that order and can also be grouped, such as:
```sh
./simpledu -laLSB 2048 path/random
```

- Multiple paths can also be passed to the tool, resulting the tool to display the information for the paths in the order they are passed
```sh
./simpledu -la ./src ./include
```
results in
```
4       ./src/log.c
20      ./src/main.c
8       ./src/parse.c
4       ./src/sig_handler.c
8       ./src/utils.c
44      ./src
4       ./include/log.h
4       ./include/parse.h
4       ./include/sig_handler.h
4       ./include/utils.h
16      ./include/
```
Note that the position of the path doesn't matter, this is equivalent to the last example:
```sh
./simpledu ./src -la ./include
```
