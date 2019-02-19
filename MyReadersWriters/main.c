#include <Windows.h>
#include <stdio.h>
#include <conio.h>

volatile LONG
waitingReadersCount,
activeReadersCount,
waitingWritersCount;

HANDLE canReadEvent, canWriteEvent, writeMutex;

volatile int value = 0;

void StartRead()
{
	InterlockedIncrement(&waitingReadersCount);			// Увеличиваем количество ожидающих читателей

	WaitForSingleObject(writeMutex, INFINITE);			// Кто-то может производить запись. Необходимо дождаться завершения записи.
														// Выход из функции произойдет тогда, когда поток получит право владения мютексом
														// Возвращаемое значение будет WAIT_OBJECT_0
														// Нам не нужно блокировать мютекс, поэтому...

	ReleaseMutex(writeMutex);							// ...отпускаем mutex.

	if(waitingWritersCount > 0)
	{
		WaitForSingleObject(canReadEvent, INFINITE);	// Сейчас имеются ожидающие писатели. Подождем, пока она пошлют событие "canReadEvent"
	}

	InterlockedDecrement(&waitingReadersCount);			// Этот поток уже не ожидающий писатель. Уменьшаем количество ожидающих...
	InterlockedIncrement(&activeReadersCount);			// ...и увеличиваем количество активных. 

	ResetEvent(canWriteEvent);							// НЕТ В ИСХОДНОМ ФАЙЛЕ. Даем сигнал о запрете операций записи.

	if(waitingReadersCount > 0)
	{
		SetEvent(canReadEvent);							// Есть еще ожидающие читатели. Сигнализируем о том, что другой поток может готовиться к чтению.
	}
}

void StopRead()
{
	InterlockedDecrement(&activeReadersCount);			// Уменьшаем количество активных писателей.
	if(waitingReadersCount == 0)						// Ожидающих читателей нет. Можно сигнализировать о том, что снова можно писать.
	{
		SetEvent(canWriteEvent);
	}
}

void StartWrite()
{
	InterlockedIncrement(&waitingWritersCount);			// Увеличиваем количество ожидающих писателей.

	if(activeReadersCount > 0)
	{
		WaitForSingleObject(canWriteEvent, INFINITE);	// Есть потоки, которые в данный момент читают. Им нельзя мешать, поэтому подождем от них сигнала разрешения.
	}

	WaitForSingleObject(writeMutex, INFINITE);			// Подождем, пока наш поток захватит мютекс.

	ResetEvent(canReadEvent);							// НЕТ В ИСХОДНОМ ФАЙЛЕ. Даем сигнал о запрете операций чтения

	InterlockedDecrement(&waitingWritersCount);			// Выходим из списка ожидающих писателей.
}

void StopWrite()
{
	ReleaseMutex(writeMutex);							// Поток закончил операцию записи. Можно освободить мютекс.
	if(waitingReadersCount > 0)
	{
		SetEvent(canReadEvent);							// Если в очереди есть ожидающие читатели, то разрешим им чтение
	}
	else
	{
		SetEvent(canWriteEvent);						// Если же их нет, то разрешим запись другим потокам.
	}
}



DWORD WINAPI Writer(PVOID pvParam)
{
	while (1)
	{
		StartWrite();										// Встаем в очередь чтения.
		Sleep(rand() / 75);									// Имитируем длительный процесс записи.
		printf("Writer %d: %d\n", (int)pvParam, ++value);	// Читатель увеличивает общую переменную.
		StopWrite();										// Сигнализируем о конце операции.
		Sleep(rand() / 7);									// Имитируем паузу
	}
	return 0;
}

DWORD WINAPI Reader(PVOID pvParam)
{
	while (1)
	{
		StartRead();										// Встаем в очередь чтения.
		Sleep(rand() / 100);								// Имитируем длительный процесс чтения
		printf("Reader %d: %d\n", (int)pvParam, value);		// Выводим общую переменную на экран.
		StopRead();											// Сигнализируем о конце чтения
		Sleep(rand() / 10);									// Имитируем паузу
	}

	return 0;
}

int main()
{
	HANDLE Writers[3], Readers[3];
	canReadEvent = CreateEvent(NULL, FALSE, FALSE, NULL); // Initial state is FALSE
	canWriteEvent = CreateEvent(NULL, FALSE, TRUE, NULL); // Initial state is TRUE
	writeMutex = CreateMutex(NULL, FALSE, NULL);

	for (int i = 0; i < 3; i++)
	{
		Writers[i] = CreateThread(NULL, NULL, Writer, i, NULL, NULL);
	}

	for (int j = 0; j < 3; j++)
	{
		Readers[j] = CreateThread(NULL, NULL, Reader, j, NULL, NULL);
	}

	_getch();
	return 0;
}