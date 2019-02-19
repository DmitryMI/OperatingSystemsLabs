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
	InterlockedIncrement(&waitingReadersCount);			// ����������� ���������� ��������� ���������

	WaitForSingleObject(writeMutex, INFINITE);			// ���-�� ����� ����������� ������. ���������� ��������� ���������� ������.
														// ����� �� ������� ���������� �����, ����� ����� ������� ����� �������� ��������
														// ������������ �������� ����� WAIT_OBJECT_0
														// ��� �� ����� ����������� ������, �������...

	ReleaseMutex(writeMutex);							// ...��������� mutex.

	if(waitingWritersCount > 0)
	{
		WaitForSingleObject(canReadEvent, INFINITE);	// ������ ������� ��������� ��������. ��������, ���� ��� ������ ������� "canReadEvent"
	}

	InterlockedDecrement(&waitingReadersCount);			// ���� ����� ��� �� ��������� ��������. ��������� ���������� ���������...
	InterlockedIncrement(&activeReadersCount);			// ...� ����������� ���������� ��������. 

	ResetEvent(canWriteEvent);							// ��� � �������� �����. ���� ������ � ������� �������� ������.

	if(waitingReadersCount > 0)
	{
		SetEvent(canReadEvent);							// ���� ��� ��������� ��������. ������������� � ���, ��� ������ ����� ����� ���������� � ������.
	}
}

void StopRead()
{
	InterlockedDecrement(&activeReadersCount);			// ��������� ���������� �������� ���������.
	if(waitingReadersCount == 0)						// ��������� ��������� ���. ����� ��������������� � ���, ��� ����� ����� ������.
	{
		SetEvent(canWriteEvent);
	}
}

void StartWrite()
{
	InterlockedIncrement(&waitingWritersCount);			// ����������� ���������� ��������� ���������.

	if(activeReadersCount > 0)
	{
		WaitForSingleObject(canWriteEvent, INFINITE);	// ���� ������, ������� � ������ ������ ������. �� ������ ������, ������� �������� �� ��� ������� ����������.
	}

	WaitForSingleObject(writeMutex, INFINITE);			// ��������, ���� ��� ����� �������� ������.

	ResetEvent(canReadEvent);							// ��� � �������� �����. ���� ������ � ������� �������� ������

	InterlockedDecrement(&waitingWritersCount);			// ������� �� ������ ��������� ���������.
}

void StopWrite()
{
	ReleaseMutex(writeMutex);							// ����� �������� �������� ������. ����� ���������� ������.
	if(waitingReadersCount > 0)
	{
		SetEvent(canReadEvent);							// ���� � ������� ���� ��������� ��������, �� �������� �� ������
	}
	else
	{
		SetEvent(canWriteEvent);						// ���� �� �� ���, �� �������� ������ ������ �������.
	}
}



DWORD WINAPI Writer(PVOID pvParam)
{
	while (1)
	{
		StartWrite();										// ������ � ������� ������.
		Sleep(rand() / 75);									// ��������� ���������� ������� ������.
		printf("Writer %d: %d\n", (int)pvParam, ++value);	// �������� ����������� ����� ����������.
		StopWrite();										// ������������� � ����� ��������.
		Sleep(rand() / 7);									// ��������� �����
	}
	return 0;
}

DWORD WINAPI Reader(PVOID pvParam)
{
	while (1)
	{
		StartRead();										// ������ � ������� ������.
		Sleep(rand() / 100);								// ��������� ���������� ������� ������
		printf("Reader %d: %d\n", (int)pvParam, value);		// ������� ����� ���������� �� �����.
		StopRead();											// ������������� � ����� ������
		Sleep(rand() / 10);									// ��������� �����
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