#include <Windows.h>
#include <stdio.h>
#include <conio.h>

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
	CanRead = CreateEvent(NULL, FALSE, FALSE, NULL);
	CanWrite = CreateEvent(NULL, FALSE, TRUE, NULL);
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