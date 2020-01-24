/* This Module holds all kind of thread functionalities*/

#include <stdio.h>
#include "thread_handle.h"
#include "../shared/common.h"


/*Returns the index of the first available handle in our clients-thread-array*/
/*input: array of thread handle ThreadHandles*/
int FindFirstUnusedThreadSlot(HANDLE *ThreadHandles)
{
	int Ind;

	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++) {
		if (ThreadHandles[Ind] == NULL)
			break;
		else {
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(ThreadHandles[Ind], 0);

			if (Res == WAIT_OBJECT_0) // this thread finished running
			{
				CloseHandle(ThreadHandles[Ind]);
				ThreadHandles[Ind] = NULL;
				break;
			}
		}
	}

	return Ind;
}


void CleanupWorkerThreads(HANDLE *ThreadHandles, SOCKET *ThreadInputs)
{
	int Ind = 0;

	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++) {
		if (ThreadHandles[Ind] != NULL) {
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(ThreadHandles[Ind], INFINITE);

			if (Res == WAIT_OBJECT_0) {
				closesocket(ThreadInputs[Ind]);
				CloseHandle(ThreadHandles[Ind]);
				ThreadHandles[Ind] = NULL;
				break;
			}
			else {
				printf("Waiting for thread failed. Ending program\n");
				return;
			}
		}
	}
}

/*initialize named semaphore with starting value OF ZERO */
HANDLE init_game_semp() {
	HANDLE game_semp = NULL;
	game_semp = CreateSemaphore(NULL,	/* Default security attributes */
		0,		/* Initial Count - STARTING WHEN SEMAPHORE IS ON ZERO, CANNOT CATCH UNTIL RELEASE! */
		2,		/* Maximum Count */
		GAME_SEMP_NAME); /* Named semaphore for multi-thread access*/
	if(game_semp == NULL) {
		printf("Error acquiring game semaphore\n");
	}
	return game_semp;
}

HANDLE init_file_mutex() {
	HANDLE file_mutex = NULL;
	file_mutex = CreateMutex(
		NULL,   /* default security attributes */
		FALSE,	/* don't lock mutex immediately */
		FILE_MUTEX_NAME); /* un-named */
	if (file_mutex == NULL) {
		printf("Error acquiring file mutex\n");
	}
	return file_mutex;
}
