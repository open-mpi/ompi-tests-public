
EXES = events_types events_callbacks events_read_data events_meta_data events_source events_dropped events_example

CC= mpicc

CFLAGS= -O2 -g -Wall
LIBS=events_common.o

def: clean ${EXES}

events_types : events_types.o events_common.o
	$(CC) $(CFLAGS) -o $@ $< ${LIBS}

events_callbacks : events_callbacks.o events_common.o
	$(CC) $(CFLAGS) -o $@ $< ${LIBS}

events_read_data : events_read_data.o events_common.o
	$(CC) $(CFLAGS) -o $@ $< ${LIBS}

events_meta_data : events_meta_data.o events_common.o
	$(CC) $(CFLAGS) -o $@ $< ${LIBS}

events_source : events_source.o events_common.o
	$(CC) $(CFLAGS) -o $@ $< ${LIBS}

events_dropped : events_dropped.o events_common.o
	$(CC) $(CFLAGS) -o $@ $< ${LIBS}

events_example : events_example.o
	$(CC) $(CFLAGS) -o $@ $< 

clean:
	rm -f ${EXES} core.* *.o
