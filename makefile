EXEC    = knap_omp knap_mpi knap_omp_debug
OBJS    = timer.o
H_FILES = timer.h

all:	$(EXEC)

knap_omp: knap_omp.c $(H_FILES) $(OBJS)
	gcc -fopenmp -O3 $< -o $@ $(OBJS)

knap_mpi: knap_mpi.c $(H_FILES) $(OBJS)
	mpicc -O3 $< -o $@ $(OBJS)

knap_omp_debug: knap_omp.c $(H_FILES) $(OBJS)
	gcc -fopenmp -O3 -DDEBUG $< -o $@ $(OBJS)

timer.o:	timer.c
	gcc -o $@ -c timer.c

clean:
	rm -f $(OBJS) $(EXEC)
