
CFLAGS = -g -O0
MPICC = mpicc

PROGS = cmsplit_type

all: $(PROGS)

cmsplit_type: cmsplit_type.c
	$(MPICC) cmsplit_type.c $(CFLAGS) -o cmsplit_type

clean:
	rm $(PROGS)

