all:
	  gcc -Iclient -Icommon -Iserver -Wall -DMEMWATCH -DMW_STDIO server/Logging.c client/ClientMain.c server/ServerMain.c common/MonitorRequest.c client/Process.c client/ProcessManager.c common/ProgramIO.c client/RegisterEntry.c common/Utils.c common/memwatch.c -o procnanny

clean:
	$(RM) procnanny