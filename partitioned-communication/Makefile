BIN=./bin/
OBJ=./obj/

# C compiler
CC=gcc

# MPI
# if required, add paths to mpi.h and any other necessary headers (e.g., UCX)
CFLAGS=
# if required, add paths to ompi libraries and any other necessary libraries
LDFLAGS=

.PHONY: clean

all: test_cancel0.x test_datatype0.x test_datatype1.x test_datatype2.x test_datatype3.x test_datatype4.x test_datatype5.x test_example1a.x test_example1b.x test_example2.x test_example3a.x test_example3b.x test_example3c.x test_free0.x test_init0.x test_init1.x test_init2.x test_local0.x test_local1.x test_numparts0.x test_numparts1.x test_order0.x test_parrived0.x test_parrived1.x test_parrived2.x test_pready0.x test_pready1.x test_pready2.x test_pready3.x test_pready4.x test_pready_list0.x test_pready_list1.x test_pready_range0.x test_partitions0.x test_partitions1.x test_partitions2.x test_partitions3.x test_startall0.x test_state0.x test_wildcard0.x test_wildcard1.x test_zerocount0.x test_zerocount1.x

test_cancel0.x: test_cancel0.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_cancel0.o: test_cancel0.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_datatype0.x: test_datatype0.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_datatype0.o: test_datatype0.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_datatype1.x: test_datatype1.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_datatype1.o: test_datatype1.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_datatype2.x: test_datatype2.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_datatype2.o: test_datatype2.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_datatype3.x: test_datatype3.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_datatype3.o: test_datatype3.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_datatype4.x: test_datatype4.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_datatype4.o: test_datatype4.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_datatype5.x: test_datatype5.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_datatype5.o: test_datatype5.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_example1a.x: test_example1a.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_example1a.o: test_example1a.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_example1b.x: test_example1b.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_example1b.o: test_example1b.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_example2.x: test_example2.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) -fopenmp

test_example2.o: test_example2.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS) -fopenmp

test_example3a.x: test_example3a.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) -fopenmp

test_example3a.o: test_example3a.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS) -fopenmp

test_example3b.x: test_example3b.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) -fopenmp

test_example3b.o: test_example3b.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS) -fopenmp

test_example3c.x: test_example3c.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) -fopenmp

test_example3c.o: test_example3c.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS) -fopenmp

test_free0.x: test_free0.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_free0.o: test_free0.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_init0.x: test_init0.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_init0.o: test_init0.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_init1.x: test_init1.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_init1.o: test_init1.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_init2.x: test_init2.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_init2.o: test_init2.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_local0.x: test_local0.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_local0.o: test_local0.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_local1.x: test_local1.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_local1.o: test_local1.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_numparts1.x: test_numparts1.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_numparts1.o: test_numparts1.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_numparts0.x: test_numparts0.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_numparts0.o: test_numparts0.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_order0.x: test_order0.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_order0.o: test_order0.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_parrived0.x: test_parrived0.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_parrived0.o: test_parrived0.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_parrived1.x: test_parrived1.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_parrived1.o: test_parrived1.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_parrived2.x: test_parrived2.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_parrived2.o: test_parrived2.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_partitions0.x: test_partitions0.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_partitions0.o: test_partitions0.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_partitions1.x: test_partitions1.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_partitions1.o: test_partitions1.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_partitions2.x: test_partitions2.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_partitions2.o: test_partitions2.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_partitions3.x: test_partitions3.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_partitions3.o: test_partitions3.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_pready0.x: test_pready0.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_pready0.o: test_pready0.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_pready1.x: test_pready1.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_pready1.o: test_pready1.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_pready2.x: test_pready2.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_pready2.o: test_pready2.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_pready3.x: test_pready3.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_pready3.o: test_pready3.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_pready4.x: test_pready4.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_pready4.o: test_pready4.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_pready_list0.x: test_pready_list0.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_pready_list0.o: test_pready_list0.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_pready_list1.x: test_pready_list1.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_pready_list1.o: test_pready_list1.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_pready_range0.x: test_pready_range0.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_pready_range0.o: test_pready_range0.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_startall0.x: test_startall0.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_startall0.o: test_startall0.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_state0.x: test_state0.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_state0.o: test_state0.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_wildcard0.x: test_wildcard0.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_wildcard0.o: test_wildcard0.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_wildcard1.x: test_wildcard1.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_wildcard1.o: test_wildcard1.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_zerocount0.x: test_zerocount0.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_zerocount0.o: test_zerocount0.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

test_zerocount1.x: test_zerocount1.o
	mkdir -p ${BIN}
	$(CC) -o ${BIN}$@ -L${OBJ} $(LDFLAGS) $(addprefix ${OBJ}, $^) 

test_zerocount1.o: test_zerocount1.c
	mkdir -p ${OBJ}
	$(CC) -c $< -o ${OBJ}$@ $(CFLAGS)

clean:
	rm -f *.o
	rm -f *.x
	rm -f ${OBJ}*.o
	rm -f ${BIN}*.x
	rm -r obj
	rm -r bin
