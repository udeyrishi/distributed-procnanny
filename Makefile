all: *.c
	  gcc -Wall -DMEMWATCH -DMW_STDIO Logging.c Main.c MonitorRequest.c Process.c ProcessManager.c ProgramIO.c RegisterEntry.c Utils.c memwatch.c -o procnanny

clean:
	$(RM) procnanny