#include <stdio.h>
#include <windows.h>

const DWORD sleep_time_for_writer = 50; // milliseconds for writer 
const DWORD sleep_time_for_reader = 30; // milliseconds for reader 

#define READERS_COUNT 5
#define WRITERS_COUNT 3
#define ITERATIONS 5

HANDLE mutex;
HANDLE can_read;
HANDLE can_write;

HANDLE writers[WRITERS_COUNT];
HANDLE readers[READERS_COUNT];

volatile LONG active_readers = 0;
int writing = 0;

int value = 0;

void start_read()
{
	if (writing || WaitForSingleObject(can_write, 0) == WAIT_OBJECT_0)
	{
		WaitForSingleObject(can_read, INFINITE);
	}

	//WaitForSingleObject(mutex, INFINITE);
	InterlockedIncrement(&active_readers);
	SetEvent(can_read);
}

void stop_read()
{
	InterlockedDecrement(&active_readers);

	if (active_readers == 0) {
		SetEvent(can_write);
	}

	//ReleaseMutex(mutex);
}

void start_write()
{
	WaitForSingleObject(mutex, INFINITE);

	if (writing || active_readers > 0) {
		WaitForSingleObject(can_write, INFINITE);
	}

	writing = 1;

	ReleaseMutex(mutex);
}

void stop_write()
{
	writing = 0;

	if (WaitForSingleObject(can_read, 0) == WAIT_OBJECT_0) {
		SetEvent(can_read);
	}
	else {
		SetEvent(can_write);
	}


}

DWORD WINAPI reader(LPVOID par) 
{
	while (value < WRITERS_COUNT * ITERATIONS) {
		start_read();
		//Sleep(sleep_time_for_reader);
		printf("\tReader #%ld <---- %d\n", GetCurrentThreadId(), value);
		stop_read();
		Sleep(sleep_time_for_reader);
	}

	return 0; 
}

DWORD WINAPI writer(LPVOID par)
{
	for (int i = 0; i < ITERATIONS; i++) {
		start_write();
		//Sleep(sleep_time_for_writer);
		printf("Writer #%ld ----> %ld\n", GetCurrentThreadId(), ++value);
		stop_write();
		Sleep(sleep_time_for_writer);
	}

	return 0; 
}

int init_handles()
{

	mutex = CreateMutex(NULL, FALSE, NULL);
	if (mutex == NULL) {
		perror("!!! Can't create mutex");
		return EXIT_FAILURE;
	}

	can_read = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (can_read == NULL) {
		perror("!!! Can't create event can read");
		return EXIT_FAILURE;
	}

	can_write = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (can_write == NULL) {
		perror("!!! Can't create event can write");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int create_threads()
{
	for (int i = 0; i < WRITERS_COUNT; i++) 
	{
		 
		writers[i] = CreateThread(NULL, 0, writer, NULL, 0, NULL);
		if (writers[i] == NULL) {
			perror("!!! Can't create writer");
			return EXIT_FAILURE;
		}
	}

	for (int i = 0; i < READERS_COUNT; i++) {
		readers[i] = CreateThread(NULL, 0, reader, NULL, 0, NULL);
		if (readers[i] == NULL) {
			perror("!!! Can't create reader");
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

int main()
{
	int code = EXIT_SUCCESS;

	if ((code = init_handles()) != EXIT_SUCCESS) {
		return code;
	}

	if ((code = create_threads()) != EXIT_SUCCESS) {
		return code;
	}

	WaitForMultipleObjects(WRITERS_COUNT, writers, TRUE, INFINITE);
	WaitForMultipleObjects(READERS_COUNT, readers, TRUE, INFINITE);

	CloseHandle(mutex);
	CloseHandle(can_read);
	CloseHandle(can_write);

	system("pause");

	return code;
}