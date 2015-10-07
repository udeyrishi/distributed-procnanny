all: *.c
	  gcc -Wall -DMEMWATCH -DMW_STDIO *.c -o procnanny

clean:
	$(RM) procnanny
	$(RM) memwatch.log