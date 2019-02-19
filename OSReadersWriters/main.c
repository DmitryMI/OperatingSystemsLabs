#include <Windows.h>
#include <stdio.h>
#include <conio.h>


// Help for used API
/*
 * InterlockedIncrement and InterlockedDecrement - atomic ++ and --
 * WaitForSingleObject - Waits until the specified object is in the signaled state or the time-out interval elapses.
 * SetEvent - sets specified object to signaled state
 * ReleaseMutex - Releases ownership of the specified mutex object.
 * CreateMutex - Creates or opens a named or unnamed mutex object.
 * CreateEvent - Creates or opens a named or unnamed event object.
 */



// Working algorithm

/* Global variables
 * ActReadersCount - count of active readers
 * ReadersCount - count of waiting readers
 * WritersCount - count of waiting writers
 * Value - output of each writer. Readers get R/O access to this variable
 * CanRead/CanWrite - events, that determine, if it is allowed to read/write
 * Writing - mutex for writers
 */

/* Staring to read
 * 1) Increase count of waiting readers. Operation must be atomic
 * 2) Wait until mutex is set to signaled state
 * 3) Release this mutex
 * 4) If there are any writers, wait until CanRead is set signaled
 * 5) Decrease count of waiting readers (atomic)
 * 6) Increase count of active readers (atomic)
 * 7) If there are any waiting readers, set CanRead event to signaled state
 */

/* Stopping reading
 * 1) Decrease count of active readers
 * 2) If there is no waiting readers, set CanWrite event to signaled state
 */

/* Starting to write
 * 1) Increase count of waiting writes (atomic)
 * 2) If there are any active readers, wait until CanWrite is set to signaled state
 * 3) Wait until mutex is released
 * 4) Decrease count of waiting writers
 */

/* Stopping writing
 * 1) Release mutex
 * 2) If there are any waiting readers, set event CanRead to signaled state
 * 3) If not, set CanWrite to signaled state
 */

volatile LONG
ActReadersCount = 0,
ReadersCount = 0,
WritersCount = 0;

volatile int Value = 0;
HANDLE CanRead, CanWrite, Writing;

void StartRead()
{
	// Increases value of parametr as an atomic operation
	InterlockedIncrement(&ReadersCount);

	WaitForSingleObject(Writing, INFINITE);
	ReleaseMutex(Writing);
	if (WritersCount)
		WaitForSingleObject(CanRead, INFINITE);
	InterlockedDecrement(&ReadersCount);
	InterlockedIncrement(&ActReadersCount);
	if (ReadersCount != 0)
		SetEvent(CanRead);
}

void StopRead()
{
	InterlockedDecrement(&ActReadersCount);
	if (ReadersCount == 0)
		SetEvent(CanWrite);
}

void StartWrite()
{
	InterlockedIncrement(&WritersCount);
	if(ActReadersCount != 0)
	{
		WaitForSingleObject(CanWrite, INFINITE);
	}
	WaitForSingleObject(Writing, INFINITE);
	InterlockedDecrement(&WritersCount);		
}

void StopWrite()
{
	ReleaseMutex(Writing);
	if (ReadersCount != 0)
		SetEvent(CanRead);
	else
		SetEvent(CanWrite);
}

DWORD WINAPI Reader(PVOID pvParam)
{
	while(1)
	{
		StartRead();
		Sleep(rand() / 100);
		printf("Reader %d: %d\n", (int)pvParam, Value);
		StopRead();
		Sleep(rand() / 10);
	}

	return 0;
}

DWORD WINAPI Writer(PVOID pvParam)
{
	while(1)
	{
		StartWrite();
		Sleep(rand() / 75);
		printf("Writer %d: %d\n", (int)pvParam, ++Value);
		StopWrite();
		Sleep(rand() / 7);
	}
	return 0;
}

int main()
{
	HANDLE Writers[3], Readers[3];
	CanRead = CreateEvent(NULL, FALSE, FALSE, NULL); // Initial state is FALSE
	CanWrite = CreateEvent(NULL, FALSE, TRUE, NULL); // Initial state is TRUE
	Writing = CreateMutex(NULL, FALSE, NULL);

	for(int i = 0; i < 3; i++)
	{
		Writers[i] = CreateThread(NULL, NULL, Writer, i, NULL, NULL);
	}

	for(int j = 0; j < 3; j++)
	{
		Readers[j] = CreateThread(NULL, NULL, Reader, j, NULL, NULL);
	}

	_getch();
	return 0;
}