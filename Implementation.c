#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>

#define NUMBER_OF_CUSTOMERS 5
#define NUMBER_OF_RESOURCES 3

int available[NUMBER_OF_RESOURCES];
int allocation[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
//= { { 7, 1, 0 }, { 2,0, 0 }, { 3, 0, 2 }, { 2, 1, 1 }, { 0, 0, 2 } };
int maximum[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
//= { { 7, 5, 3 },{ 3, 2, 2 }, { 9, 0, 2 }, { 2, 2, 2 }, { 4, 3, 3 } };
int need[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
int safeSequence[NUMBER_OF_CUSTOMERS];
int process_n_ran = 0;

pthread_mutex_t lockResources;
pthread_cond_t condition;

// finds and prints the safe sequence
bool printSafeSequence();
// process function
void* Threadings(void*);

int main(int argc, char **argv) {

	srand(time(NULL));
	for (int i = 0; i < NUMBER_OF_RESOURCES; i++)
	{
		available[i] = atoi(argv[i + 1]);
	}
	printf("\n");

	// Resources allocated to process
	for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++)
	{
		printf("\n Please enter Resource allocated to process - %d (R1 R2 ...): ", i + 1);
		for (int j = 0; j < NUMBER_OF_RESOURCES; j++)
			scanf("%d", &allocation[i][j]);
	}
	printf("\n");



	// Max requested resources by process
	for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
		printf("\n Please enter Maximum resource required by process %d (R1 R2 ...): ",
				i + 1);
		for (int j = 0; j < NUMBER_OF_RESOURCES; j++)
			scanf("%d", &maximum[i][j]);
	}
	printf("\n");
	//need matrix calculation
	for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++)
		for (int j = 0; j < NUMBER_OF_RESOURCES; j++)
			need[i][j] = maximum[i][j] - allocation[i][j];

	// get safe sequence

	for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++)
		safeSequence[i] = -1;

	if (!printSafeSequence()) {
		printf("\nUnsafe State!\n");
		exit(-1);
	}

	printf("Safe Sequence Found and the sequence is  : ");
	for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
		printf("%-3d", safeSequence[i] + 1);
	}

	pthread_t processes[NUMBER_OF_CUSTOMERS];
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	int processNumber[NUMBER_OF_CUSTOMERS];
	for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++)
		processNumber[i] = i;

	for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++)
		pthread_create(&processes[i], &attr, Threadings,
				(void*) (&processNumber[i]));

	for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++)
		pthread_join(processes[i], NULL);

	printf("\nAll Processes Finished\n");

}

bool printSafeSequence() {
	// get safe sequence
	int Work[NUMBER_OF_RESOURCES];
	for (int i = 0; i < NUMBER_OF_RESOURCES; i++)
		Work[i] = available[i];

	bool Finish[NUMBER_OF_CUSTOMERS];
	for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++)
		Finish[i] = false;
	int nFinish = 0;
	while (nFinish < NUMBER_OF_CUSTOMERS) {
		bool safe = false;

		for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
			if (!Finish[i]) {
				bool possible = true;

				for (int j = 0; j < NUMBER_OF_RESOURCES; j++)
					if (need[i][j] > Work[j]) {
						possible = false;
						break;
					}

				if (possible) {
					for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
						Work[j] += allocation[i][j];
					}
					safeSequence[nFinish] = i;
					Finish[i] = true;
					++nFinish;
					safe = true;
				}
			}
		}

		if (!safe) {
			for (int k = 0; k < NUMBER_OF_CUSTOMERS; k++)
				safeSequence[k] = -1;
			return false; // no safe sequence found
		}
	}
	return true; // safe sequence found
}

// process code
void* Threadings(void *arg) {
	int p = *((int*) arg);
	// lock available
	pthread_mutex_lock(&lockResources);
	// condition check
	while (p != safeSequence[process_n_ran])
		pthread_cond_wait(&condition, &lockResources);
	for (int i = 0; i < NUMBER_OF_RESOURCES; i++)
		available[i] += allocation[p][i];
	// condition broadcast
	process_n_ran++;
	pthread_cond_broadcast(&condition);
	pthread_mutex_unlock(&lockResources);
	pthread_exit(NULL);
}


