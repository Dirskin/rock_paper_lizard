#pragma once

#ifndef THREAD_HANDLE
#define THREAD_HANDLE

#include <winsock2.h>

int FindFirstUnusedThreadSlot(HANDLE *ThreadHandles);
void CleanupWorkerThreads(HANDLE *ThreadHandles, SOCKET *ThreadInputs);

#endif // !THREAD_HANDLE

