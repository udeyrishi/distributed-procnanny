# Description
distributed-procnanny is a fork from [procnanny](https://github.com/udeyrishi/procnanny), and it provides similar functionality as the latter does, but it allows system administrators to monitor processes on multiple machines and configure and control this monitoring, remotely. This is a simple implementation of a client-server based process monitoring system on a UNIX based OS (tested on Linux, should work on OS X with slight modifications) meant for personal use; it is NOT built with security in mind, so use with caution.

### procnanny-client
The procnanny-client monitors the specified processes in the user's space for their execution for a specified amount of real time, and if any of them exceeds that period, sends them a ```SIGKILL```. The list of processes in the user's space is retrieved using the unix command ```ps -u```. If the monitored process dies on its own before specified time, procnanny-client does nothing.

procnanny-client talks to procnanny-server to get the config file (i.e., list of processes to be monitored and their respective monitoring durations), send it the monitoring event logs, and for getting new configurations or self-termination requests. procnanny-client should not be controlled directly on the client node, but instead through procnanny-server.

### procnanny-server
The procnanny-server listens for requests from procnanny-client to initiate monitoring on a particular node. On this request, procnanny-server sends the last read config file to the procnanny-client. procnanny-server then keeps listening for events that the different clients might send, and logs them to the log file pointed by PROCNANNYLOGS environment variable.

# Config File
The config file should be a plain text file containing the name of the processes to be monitored along with the monitoring duration in seconds. For example:

```
proca 600
procb 1800
./fooproc 60
```
Note that the process names in the config file should be EXACTLY identical to their names in the ```COMMAND``` column when ```ps -u``` is run. Partially matching names will not be monitored. For example, if the process to be monitored is "proca", ensure the config lists "proca", and not "./proca" or a variant, and vice versa.

# Compiling
The project uses GNU Make, so just compile like this:

```sh
# Make client + server (default)
$ make

# Same as just make
$ make all

# Delete the executables and any object files
$ make clean

# Make client
$ make client

# Make Server
$ make server

# Clean only the server executables and object files
$ make clean-server

# Clean only the client executables and object files
$ make clean-client

# Clean only the log files (files pointed by PROCNANNYSERVERINFO
# and PROCNANNYLOGS env vars, and the memwatch.log file)
$ make clean-logs

# Clean everything (make clean && make clean-logs)
$ make clean-all

# Clean everything and build from scratch (make clean-all && make all)
$ make rebuild
```
On compiling, executables named "procnanny.server" and "procnanny.client" would be put in the project root directory.

# Usage

### Server Initialization
1. Ensure that the environment variables ```PROCNANNYLOGS``` and ```PROCNANNYSERVERINFO``` are set up and that they contain a path (relative or absolute) where procnanny.server should output logs and the server host and port info respectively.

2. Start the server like this:

    ```sh
    $ ./procnanny.server /path/to/configfile.txt
    ```

3. Open the ```PROCNANNYSERVERINFO``` file and note down the host name and the port number. The client will need it.

### Client Initialization
1. Start the client like this, where the hostname and port of the server were noted in step 3 above:

    ```sh
    $ ./procnanny.client hostname port
    ```

    procnanny.client will request the procnanny.server located at the given hostname and port to give it a copy of the latest read config file, and will register itself as a client.

### Usage notes
1. The processes to be monitored need not be running when the procnanny.client first starts. If the client notices that a process listed in the config started running, it will start monitoring it.
2. It is possible to change the config file after deployment. Modify the config file and send a ```SIGHUP``` to the server. The server will re-read the config and send it to all the clients. The processes that were already being monitored by a client will continue to be monitored as per the old config; the new config will be applied to everything else.
3. To shut down the system, send a ```SIGINT``` to the server. It will ask all the clients to safely shut down, and then will exit.