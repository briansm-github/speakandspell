OBJS  = sas.o


CC=gcc 
CFLAGS= -O2
LDFLAGS=  -O -lm

all:    $(OBJS)
	$(CC) -o sas sas.o -lm



clean: 
	rm -f *.o *~

.c.o:	$<
	$(CC) $(CFLAGS) -c $<

