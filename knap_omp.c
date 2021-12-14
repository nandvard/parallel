#include <stdio.h>
#include <stdlib.h>
#include "timer.h"

#define MAX(a,b)	(a)>(b)?(a):(b)

int N, C;                   // number of objects, capacity 
int *Weights, *Profits;
int *P1, *P2;
char* Solution;

void find_last_row(int Start, int Stop, int C, int* prev)
{
int i,j;
int* curr = (int *)calloc(C+1,sizeof(int));
int* tmp;
for(i=Start; i<=Stop; i++){

	#pragma omp parallel for
	for(j=C; j>=Weights[i]; j--)
		curr[j] = MAX( prev[j] , Profits[i] + prev[j-Weights[i]] );

	while(j>=0){
		curr[j] = prev[j];
		j--;
	}
	tmp = prev; prev = curr; curr = tmp;
}

}


int knap01(int Start, int Stop, int C)
{
	int i,j,k,p;
	// base cases
	if(Start == Stop) {
		if(Weights[Start] <= C)
			Solution[Start]=1;
		return Profits[Start];
	}
	// solve 2 half's
	int mid,opt,max;
	mid = Start + (Stop-Start)/2;

	find_last_row(Start,mid,C,P1);
	find_last_row(mid+1,Stop,C,P2);

	// find c*
	max = P1[0]+P2[C];
	P1[0]=0; P2[C]=0;
	opt = 0;
	for(i=1;i<=C;i++) {
		if(P1[i]+P2[C-i] > max) {
			max = P1[i]+P2[C-i];
			opt = i;
		}
		P1[i]=0;
		P2[C-i]=0;
	}

	// recurse each half
	knap01(Start,mid,opt);
	knap01(mid+1,Stop,C-opt);
	return max;
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
	fscanf(fp, "%d %d", &N, &C);
	printf("The number of objects is %d, and the capacity is %d.\n", N, C);
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
	}

	fclose(fp);

initialize_timer ();
start_timer();
	
p=knap01(1,N,C);
	
stop_timer();
time = elapsed_time ();

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

	return 0;
}
