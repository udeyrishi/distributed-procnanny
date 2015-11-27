Name: Udey Chander Rishi
Student Number: 1375308
Unix ID: udey
Lecture Section: EA1
Instructor's Name: Dr. Paul Lu
Lab Section: ED04
TA's Name: Luke

Description
-----------
procnanny is a process monitoring and cleaning program as required by the Assignment #3 of CMPUT 379, Fall 2015
at the University of Alberta. There are 2 executables: the client (procnanny.client) and the server (procnanny.server).

The client monitors the specified processes in the user's space for their execution for a specified amount of
real time, and if any of them exceeds that period, sends them a SIGKILL. The list of processes in the user's
space is retreived using the unix command "ps -u". If the monitored process dies on its own before specified
time, procnanny does nothing.

The server monitors the log messages from any of the clients and outputs them to a log file,
sends them the configuration info upon connection initialization and on receiving a SIGHUP, and controls when
they need to be killed (on receiving SIGINT).

Please see the assignment spec for details.

Making
------

NOTE: Please put the "memwatch.h" and "memwatch.c" into the directory ./common/ before compiling
the code.

cd into the directory and run "make" to compile. Run "make clean" to delete the executables and
any object files. On compiling, executables named "procnanny.server" and "procnanny.client" would
be put in the directory.

All "make" options:

(Default) make:      make client + make server
make all:            same as just "make"
make client:         builds just the client
make server:         builds just the server
make clean-server:   cleans only the server executables and object files
make clean-client:   cleans only the client executables and object files
make clean-logs:     cleans only the log files (files pointed by PROCNANNYSERVERINFO and
                     PROCNANNYLOGS env vars, and the memwatch.log file)
make clean-all:      make clean + make clean-logs
make rebuild:        make clean-all + make all

Running
-------

1. <Server Machine> Ensure that the environment variables "PROCNANNYLOGS" and
   "PROCNANNYSERVERINFO" are set up and that they contain a path (relative or
   absolute) where procnanny.server should output logs and the server host and port info
   respectively.

2. <Server Machine> Run "procnanny.server" executable with a command line parameter
   that is the (absolute or relative) path to the the config file. See assignment spec
   for the config file format.

3. Open the "PROCNANNYSERVERINFO" file and note down the host name and the port number.

4. <Client Machine> Run "procnanny.client a b", where a is the host name and b is the port
   number of the server (noted in step 3).

Note that the process names in the config file should be EXACTLY identical to their names in the
"COMMAND" column when "ps -u" is run. Partially matching names will not be monitored. For example,
if the process to be monitored is "a.out", ensure the config lists "a.out", and not "./a.out" or a
variant, and vice versa.