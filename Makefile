all: *.c
	  gcc -Wall -DMEMWATCH -DMW_STDIO Logging.c Main.c Process.c ProgramIO.c ProcessManager.c Utils.c memwatch.c -o procnanny

clean:
	$(RM) procnanny *.log