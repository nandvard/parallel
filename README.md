knap_omp.c - finegrain openmp parallelizaiton

knap_mpi.c - tiled, non-memory efficient, non-solution vector MPI parallelization
(this only prints optimal profit & time, no solution vector)

to compile:
make

to run:
./knap_omp k2000.txt
./knap_omp_debug k2000.txt
mpirun -np 8 ./knap_mpi k2000.txt 5 	// 5 is tile height
