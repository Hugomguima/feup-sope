# Tasks

## Plan
- [x] Receive, process and save command line arguments and environment variables
- [ ] Add registration messages as new features are implemented and validate the correction of both
- [x] Start by choosing only files and presenting the desired information (bytes and blocks)
  - [ ] Consider that the `-L` (or `--dereference`) option is active so that it is not necessary to distinguish symbolic links from regular files
- [ ] Distinguish between files and symbolic links and present different results depending on the `-L` option
- [ ] Consider entries that are directories, but limit the analysis to one level (`--max-depth=1`)
- [x] Create a new process by subdirectory and try to pass the correct arguments to it
  - [x] The arguments will be the same except for the path (path / entry) and, eventually, the maximum allowed depth level (`--max depth=N-1`)
  - [ ] Assume that the `-S` (or `--separate-dirs`) option is active so that it is not necessary to cumulatively consider the size of the subdirectories
- [ ] Create pipes to communicate the size of a given subdirectory to the parent process and thus correctly present cumulative results, including for subdirectories

## `du` functionalities
- [ ] shows the space occupied in number of blocks of 1024 bytes
- [ ] only lists directories
- [ ] does not follow symbolic links
- [ ] counts each file only once
- [ ] cumulatively displays the size of included subdirectories and files
- [ ] does not restrict the depth levels in the directory structure

## Architectural requirements
- Each process
  - [ ] analyses only one directory
  - [ ] create a process for each subdirectory
  - [ ] adjust arguments passed to child processes
- [ ] The parent process (first to be executed) must always wait for the termination of all child processes before finishing its execution
- [ ] use the same code (without changes) regardless of being the main process or not
- [ ] the total size of each of the subdirectories must be communicated to the parent process through a pipe (without a name) created for each of the child processes
- [ ] the parent process must send a `SIGSTOP` signal to all child processes that are running when it receives a SIGINT signal  
  The sending of the next signal depends on the confirmation (`SIGTERM`) or not (`SIGCONT`) by the user regarding the termination of the program

## Functional requirements
- [ ] Logs
- Logs functionalities
  - [ ] Log creation and termination of processes
  - [ ] Log sent and received signals
  - [ ] Log sent and received data from pipes
  - [ ] Log files and directories analysed by each process
  - Output format: `instant - pid - action - info`
    - [ ] instant - time immediately preceding the record, measured in milliseconds and to 2 decimal places, with reference to the time when the program started executing
    - [ ] pid - process identifier that registers the line, with a fixed space for 8 digits
    - [ ] action - description of the type of event: `CREATE`, `EXIT`, `RECV_SIGNAL`, `SEND_SIGNAL`, `RECV_PIPE`, `SEND_PIPE` and `ENTRY`
    - info - additional information for each of the actions
      - [ ] `CREATE` - the command line arguments
      - [ ] `RECV_SIGNAL` - the received signal (for example, `SIGINT`)
      - [ ] `SEND_SIGNAL` - the signal sent followed by the pid of the process for which it is intended
      - [ ] `RECV_PIPE` - the message sent
      - [ ] `SEND_PIPE` - the message received
      - [ ] `ENTRY` - number of bytes (or blocks) followed by the path

## Additional Features
#### Interruption by the user
- When user sends `SIGINT` signal (`CTRL + C`):
  - [ ] Entire program suspended
  - Upon confirmation of termination
    - [ ] Finish all pending operations
    - [ ] End program
  - Upon confirmation of continuation
    - [ ] Resume operations immediately
