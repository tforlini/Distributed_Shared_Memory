# Distributed_Shared_Memory

The goal of this project is to create a system responsible for sharing the virtual memory across several processes shared on several physical machines.

The memory (address space) is split into different several memory pages which are 4KB memory blocs. 

All the processes are allowed to work on those pages (read and write) but only one process is owner of the page at a given moment.

This means that the owner has the page in his address space and other processes willing to access this page will provoke a segfault.

When such a signal is caught, the process responsible sends a request to the proprietary process to become the new owner of the page.

The default page allocation is round-robin fashioned.

# Client

The client is responsible for submitting the different processes to launch on the different physical machines. The dsmexec.c file is representing the client of the application and uses two arguments :
- a file containing the machines names on which the processes will be run
- a binary file to execute on the remotes machines and its potential arguments.

Here is an exemple of the command line to run the client:

```
dsmexec machinefile binary arg1 arg2 arg3
```

The dsmwrap file is a wrapper responsible for "cleaning" the original command line on the local machine into a new command line which will be used on the remotes machines to launch the different processes using the ssh command.

The client is also responsible for retrieving the standard and error outputs from all the machines and centralizing it into a single output on the local machine using sockets to communicate with the remotes machines.

# Distributed shared memory API

The dsm file is responsible for the implementation of the dsm API which handles the communication across the diferent processes to access the memory pages and also update the state of every process. 
