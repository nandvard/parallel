#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "timer.h"
#include "mpi.h"

#define MAX(a,b)	((a)>(b)?(a):(b))
#define MIN(a,b)    ((a)<(b)?(a):(b))

int N, C;                   // number of objects, capacity 
int *Weights, *Profits;
int *P1, *P2;
char* Solution;
int Block=10000;
int Tile=10;
int maxWeight=-1;
int *sendbuff, *recvbuff;

void block_width(int C, int Procs, int id, int* values_)
{

if(C/Procs >= maxWeight){
	values_[1]=Procs-1;
	values_[2]= C - (C/Procs)*(Procs-1);
	if(id!=Procs-1)
		values_[0]=C/Procs;
	else
		values_[0]= C - (C/Procs)*(Procs-1);
}
else{
	int nprocs = ceil((float)C/maxWeight);
	values_[1]=nprocs-1;
	values_[2]= C - (nprocs-1)*maxWeight;
	if(id<nprocs-1)
		values_[0]=maxWeight;
	else if(id>nprocs-1)
		values_[0]=0;
	else
		values_[0]= C - (nprocs-1)*maxWeight;
}

}


int knap01(int argc, char** argv)
{
int ii,i,j,k,sendj,recvj,optProfit,size,my_id,Procs,my_width,lastProc,lastWidth;
#define Sendbuff(i,j)	sendbuff[(i)*maxWeight+(j)]
#define Recvbuff(i,j)	recvbuff[(i)*maxWeight+(j)]

int* values = (int*)malloc(3*sizeof(int));
int *prev, *curr, *tmp;
int* recvcount = (int*)malloc(Procs*sizeof(int));
int* displs = (int*)malloc(Procs*sizeof(int));
int* lastRow = (int*)calloc(C+1,sizeof(int));

MPI_Status status;
MPI_Init( &argc, &argv );
MPI_Comm_rank( MPI_COMM_WORLD, &my_id );
MPI_Comm_size( MPI_COMM_WORLD, &Procs );

block_width(C,Procs,my_id,values);
my_width=values[0];
if(my_id==0)
	my_width++;
lastProc=values[1];
lastWidth=values[2];
size = my_width * sizeof(int);
prev = (int*)calloc(size,sizeof(int));
curr = (int*)calloc(size,sizeof(int));
//assert(prev != NULL && curr  != NULL && sendbuff!=NULL && recvbuff!=NULL);

//printf("id=%d,my_width=%d, lastProc=%d, lastProc_width=%d, maxW=%d, \n",my_id,my_width,values[1],values[2],maxWeight);

if(my_id==0){
	recvcount[0]=my_width;
	displs[0]=0;
	if(Procs>1){
		for(k=1;k<lastProc;k++){
			displs[k] = displs[k-1]+recvcount[k-1];
			recvcount[k]=my_width-1;
		}
		if(lastProc>0){
			recvcount[lastProc]=lastWidth;
			displs[lastProc] = displs[lastProc-1]+recvcount[lastProc-1];
		}
		for(k=lastProc+1; k<Procs; k++){
			recvcount[k]=0;
			displs[k]=displs[k-1]+recvcount[k-1];
		}
	}

	for(ii=1; ii<=N; ii+=Tile){
		for(i=ii; i<MIN(ii+Tile,N+1); i++){
			for(j=my_width-1,sendj=maxWeight-1; j>=Weights[i]; j--,sendj--){
				curr[j] = MAX( prev[j] , Profits[i] + prev[j-Weights[i]] );
				if(sendj>=0)
					Sendbuff(i-ii,sendj) = prev[j];
			}
			while(j>=0){
				curr[j] = prev[j];
				if(sendj>=0)
					Sendbuff(i-ii,sendj) = prev[j];
				j--; sendj--;
			}

			tmp = prev; prev = curr; curr = tmp;
		}
		if(lastProc!=0)
			MPI_Send(sendbuff, Tile*maxWeight, MPI_INT, my_id+1, 0, MPI_COMM_WORLD);
	}
}
else if(my_width>0){
	for(ii=1; ii<=N; ii+=Tile){
		MPI_Recv(recvbuff, Tile*maxWeight, MPI_INT, my_id-1, 0, MPI_COMM_WORLD, &status);
		for(i=ii; i<MIN(ii+Tile,N+1); i++){
			for(j=my_width-1,sendj=maxWeight-1; j>=Weights[i]; j--,sendj--){
				curr[j] = MAX( prev[j] , Profits[i] + prev[j-Weights[i]] );
				if(sendj>=0)
					Sendbuff(i-ii,sendj) = prev[j];
			}
			recvj= maxWeight - abs(j-Weights[i]);
			while(j>=0){
				curr[j] = MAX( prev[j] , Profits[i] + Recvbuff(i-ii,recvj) );
				if(sendj>=0)
					Sendbuff(i-ii,sendj) = prev[j];
				j--; sendj--; recvj--;
			}

			tmp = prev; prev = curr; curr = tmp;
		}
		if(my_id!=lastProc)
			MPI_Send(sendbuff, Tile*maxWeight, MPI_INT, my_id+1, 0, MPI_COMM_WORLD);
	}
}

MPI_Barrier(MPI_COMM_WORLD);
MPI_Gatherv(prev,my_width,MPI_INT,lastRow,recvcount,displs,MPI_INT,0,MPI_COMM_WORLD);

if(my_id==0)
	optProfit = lastRow[C];
		
free(prev);
free(curr);
free(recvbuff);
free(sendbuff);

return optProfit;
}

int main(int argc, char **argv) {
	FILE   *fp;
	int    i, j, count, size,p=0;
	double time;
	// Read input file (# of objects, capacity, (per line) weight and profit )
	if ( argc > 1 ) {
		fp = fopen(argv[1], "r"); 
		if ( fp == NULL) {
			printf("[ERROR] : Failed to read file named '%s'.\n", argv[1]);
			exit(1);
		}
	} else {
		printf("USAGE : %s [filename].\n", argv[0]);
		exit(1);
	}
	if(argc>2)
		Tile = atoi(argv[2]);

	fscanf(fp, "%d %d", &N, &C);
	size    = (N+1) * sizeof(int);
	Weights = (int *)malloc(size);
	Profits = (int *)malloc(size);
	if ( Weights == NULL || Profits == NULL ) {
		printf("[ERROR] : Failed to allocate memory for weights/profits.\n");
		exit(1);
	}


	P1 = calloc(C+1,sizeof(int));
	P2 = calloc(C+1,sizeof(int));
	Solution =  calloc(N+1,sizeof(char));

	for ( i=1 ; i <= N ; i++ ) {
		count = fscanf(fp, "%d %d", &(Weights[i]), &(Profits[i]));
		if ( count != 2 ) {
			printf("[ERROR] : Input file is not well formatted.\n");
			exit(1);
		}
		if(Weights[i] > maxWeight)
			maxWeight=Weights[i];
	}
	sendbuff = (int*)calloc(Tile*maxWeight,sizeof(int));
	recvbuff = (int*)calloc(Tile*maxWeight,sizeof(int));

	fclose(fp);

initialize_timer ();
start_timer();

p=knap01(argc,argv);

int my_id;
MPI_Comm_rank( MPI_COMM_WORLD, &my_id );
if(my_id==0){
stop_timer();
time = elapsed_time ();

	printf("The number of objects is %d, and the capacity is %d.\n", N, C);
	printf("The optimal profit is %d Time taken : %lf.\n", p, time);

#ifdef DEBUG
	printf("Solution vector is: \n --> ");
	for (i=1 ; i<=N ; i++ ) {
		printf("%d ", Solution[i]);
	}
	printf("\n");
#endif
	free(Solution);
	free(Weights);
	free(Profits);
	free(P1);
	free(P2);
}
MPI_Barrier(MPI_COMM_WORLD);
MPI_Finalize();
return 0;
}
