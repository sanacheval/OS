CC=gcc
CFLAGS=

mysh: shell.c interpreter.c shellmemory.c
	$(CC) $(CFLAGS) -D FRAME_STORE_SIZE=$(framesize) -D VAR_STORE_SIZE=$(varmemsize) -c shell.c interpreter.c shellmemory.c kernel.c paging.c pcb.c ready_queue.c -lpthread
	$(CC) $(CFLAGS) -o mysh shell.o interpreter.o shellmemory.o kernel.o paging.o pcb.o ready_queue.o

clean: 
	rm mysh; rm *.o