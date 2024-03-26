#include <stdio.h>
#include <math.h>
#include <omp.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#define TASK_SIZE 10
#define min(x, y) (((x) < (y)) ? (x) : (y))
//  Using the MONOTONIC clock
#define CLK CLOCK_MONOTONIC

/* Function to compute the difference between two points in time */
struct timespec diff(struct timespec start, struct timespec end);

/*
   Function to computes the difference between two time instances

   Taken from - http://www.guyrutenberg.com/2007/09/22/profiling-code-using-clock_gettime/

   Further reading:
http://stackoverflow.com/questions/6749621/how-to-create-a-high-resolution-timer-in-linux-to-measure-program-performance
http://stackoverflow.com/questions/3523442/difference-between-clock-realtime-and-clock-monotonic
 */
struct timespec diff(struct timespec start, struct timespec end)
{
	struct timespec temp;
	if ((end.tv_nsec - start.tv_nsec) < 0)
	{
		temp.tv_sec = end.tv_sec - start.tv_sec - 1;
		temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
	}
	else
	{
		temp.tv_sec = end.tv_sec - start.tv_sec;
		temp.tv_nsec = end.tv_nsec - start.tv_nsec;
	}
	return temp;
}

// This function implements the merging step of the Merge Sort algorithm
void mergeSortAux(double *X, int n, double *tmp)
{
	int i = 0;	   // Index of the first half of the array
	int j = n / 2; // Index of the second half of the array
	int ti = 0;	   // Index of the temporary array

	// Merge the two halves of the array into the temporary array
	while (i < n / 2 && j < n)
	{
		if (X[i] < X[j])
		{					// Compare elements from each half
			tmp[ti] = X[i]; // Store the smaller element in the temporary array
			ti++;
			i++; // Increment the temporary and index of first half
		}
		else
		{
			tmp[ti] = X[j]; // Store the smaller element in the temporary array
			ti++;
			j++; // Increment the temporary and index of second half
		}
	}

	// If there are any remaining elements in the first half, copy them to the temporary array
	while (i < n / 2)
	{
		tmp[ti] = X[i];
		ti++;
		i++;
	}

	// If there are any remaining elements in the second half, copy them to the temporary array
	while (j < n)
	{
		tmp[ti] = X[j];
		ti++;
		j++;
	}

	// Copy the sorted elements from the temporary array back to the original array
	memcpy(X, tmp, n * sizeof(double));
}

// This function implements the Merge Sort algorithm using OpenMP to parallelize the recursion
void mergeSort(double *X, int n, double *tmp)
{
	// Base case of the recursion: array of size 1 is already sorted
	if (n < 2)
		return;

// Recursively sort the first half of the array in parallel using OpenMP tasks
#pragma omp task shared(X) if (n > TASK_SIZE)
	mergeSort(X, n / 2, tmp);

// Recursively sort the second half of the array in parallel using OpenMP tasks
#pragma omp task shared(X) if (n > TASK_SIZE)
	mergeSort(X + (n / 2), n - (n / 2), tmp + n / 2);

// Wait for the two tasks to complete before merging the sorted halves
#pragma omp taskwait
	mergeSortAux(X, n, tmp);
}

int main(int argc, char *argv[])
{
	struct timespec start_e2e, end_e2e, start_alg, end_alg, e2e, alg;
	/* Should start before anything else */
	clock_gettime(CLK, &start_e2e);

	/* Check if enough command-line arguments are taken in. */
	if (argc < 3)
	{
		printf("Usage: %s n p \n", argv[0]);
		return -1;
	}

	int N = atoi(argv[1]); /* size of input array */
	int P = atoi(argv[2]); /* number of processors*/
	char *problem_name = "MERGE_SORT";
	char *approach_name = "parallel";
	//	char buffer[10];
	//	FILE* inputFile;
	FILE *outputFile;
	//	inputFile = fopen(argv[3],"r");

	char outputFileName[50];
	sprintf(outputFileName, "output/%s_%s_%s_%s_output.txt", problem_name, approach_name, argv[1], argv[2]);

	//*****
	// initiliazing array of N elements
	double *A = malloc(sizeof(double) * N);
	double *tmp = malloc(sizeof(double) * N);
	int i;

	// N elements in decreasing order

	for (i = 0; i < N; i++)
	{
		A[i] = N - i;
	}

	//*****

	clock_gettime(CLK, &start_alg); /* Start the algo timer */

	//----------------------Core algorithm starts here----------------------------------------------/
	omp_set_num_threads(P);
#pragma omp parallel
	{
#pragma omp single
		{
			mergeSort(A, N, tmp);
		}
	}

	// checking if the array is sorted
	//  for (i = 0; i < N; i++)
	//  	printf("%f", A[i]);

	//----------------------Core algorithm finished--------------------------------------------------/

	clock_gettime(CLK, &end_alg); /* End the algo timer */
	/* Ensure that only the algorithm is present between these two
	   timers. Further, the whole algorithm should be present. */

	/* Should end before anything else (printing comes later) */
	clock_gettime(CLK, &end_e2e);
	e2e = diff(start_e2e, end_e2e);
	alg = diff(start_alg, end_alg);

	/* problem_name,approach_name,n,p,e2e_sec,e2e_nsec,alg_sec,alg_nsec
	   Change problem_name to whatever problem you've been assigned
	   Change approach_name to whatever approach has been assigned
	   p should be 0 for serial codes!!
	 */
	printf("%s,%s,%d,%d,%d,%ld,%d,%ld\n", problem_name, approach_name, N, P, e2e.tv_sec, e2e.tv_nsec, alg.tv_sec, alg.tv_nsec);

	return 0;
}
