#include <stdio.h>
#include "thread_handle.h"
#include "common.h"


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
