#pragma once

#ifndef THREAD_HANDLE
#define THREAD_HANDLE
#include <tchar.h>

#include <winsock2.h>

int FindFirstUnusedThreadSlot(HANDLE *ThreadHandles);
void CleanupWorkerThreads(HANDLE *ThreadHandles, SOCKET *ThreadInputs);
HANDLE init_game_semp();
HANDLE init_file_mutex();


/* global mutex/semaphore name */
static LPCTSTR FILE_MUTEX_NAME = _T("MTMHW4_FILE_MUTEX");
static LPCTSTR GAME_SEMP_NAME = _T("MTMHW4_GAME_SEMP");

#endif // !THREAD_HANDLE

