// Формулировка задачи:
// Как только появился хоть один писатель, читателей больше не пускать. При этом читатели могут простаивать.

#include <stdio.h>
#include <windows.h>
#include <conio.h>

#define WRITERS_COUNT 3
#define READERS_COUNT 5

#define READER_SLEEP_1 300
#define READER_SLEEP_2 300
#define WRITER_SLEEP_1 500
#define WRITER_SLEEP_2 500

HANDLE 
mutex,			// Мютекс, защищающий критическую секцию записи
writer_leave,	// Событие выхода писателя из критической секции
reader_leave;	// Событие выхода всех читателей из критической секции

int value = 0;

volatile LONG active_readers = 0;

void StartRead()
{
	WaitForSingleObject(mutex, INFINITE);	// Ждем, пока писатель освободит ресурс
	ReleaseMutex(mutex);
	
	ResetEvent(reader_leave);				// Сбрасываем событие выхода из критической секции.

	InterlockedIncrement(&active_readers);	// Поток стал активным читателем
}

void StopRead()
{
	InterlockedDecrement(&active_readers);	// Поток больше не активный читатель
	//printf("Active readers: %d\n", active_readers);
	if (active_readers == 0)
		SetEvent(reader_leave);
}

void StartWrite()
{
	WaitForSingleObject(mutex, INFINITE);	// Ждем, пока другой писатель освободит ресурс
	WaitForSingleObject(reader_leave, INFINITE);
}

void StopWrite()
{
	ReleaseMutex(mutex);
}


DWORD WINAPI ReadThread()
{
	int thread_id = GetCurrentThreadId();

	printf("Reader thread %d started\n", thread_id);

	while(1)
	{
		StartRead();
		//printf("R\t%d\tEntered critical section.\n", thread_id);
		Sleep(READER_SLEEP_1);
		printf("R\t%d\tRdValue:%d\n", thread_id, value);
		Sleep(READER_SLEEP_2);
		StopRead();
	}

	return 0;
}

DWORD WINAPI WriteThread()
{
	int thread_id = GetCurrentThreadId();

	printf("Writer thread %d started\n", thread_id);

	while (1)
	{
		StartWrite();
		//printf("W\t%d\tEntered critical section.\n", thread_id);
		Sleep(WRITER_SLEEP_1);
		printf("W\t%d\tWrValue:%d\n", thread_id, ++value);
		Sleep(WRITER_SLEEP_2);
		StopWrite();
	}

	return 0;
}

int main()
{
	// Создадим мютекс и события
	mutex = CreateMutex(NULL, FALSE, NULL);
	writer_leave = CreateEvent(NULL, FALSE, TRUE, NULL);	// Событие "писатель выходит из критической секции".
															// Изначальное состояние TRUE, автоматический сброс отключен.
	reader_leave = CreateEvent(NULL, FALSE, TRUE, NULL);	// Аналогично.

	// Создадим потоки
	for(int i = 0; i < WRITERS_COUNT; i++)
	{
		CreateThread(NULL, 0, WriteThread, NULL, 0, NULL);
	}

	for (int i = 0; i < READERS_COUNT; i++)
	{
		CreateThread(NULL, 0, ReadThread, NULL, 0, NULL);
	}

	_getch();
	return 0;
}