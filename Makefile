all:
	  gcc -Iclient -Icommon -Iserver -Wall -DMEMWATCH -DMW_STDIO server/Logging.c server/Main.c common/MonitorRequest.c client/Process.c client/ProcessManager.c common/ProgramIO.c client/RegisterEntry.c common/Utils.c common/memwatch.c -o procnanny

clean:
	$(RM) procnanny