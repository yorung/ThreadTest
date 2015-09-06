// ThreadTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <thread>

int a = 0;
int egis;
void Lock()
{
	__asm {
		mov eax, 1;
	retry:
		xchg eax, [egis]
		test eax, eax
		jnz retry
	}
}

void Unlock()
{
	__asm {
		mov eax, 0;
		xchg eax, [egis]
	}
}

void ThreadMain()
{
	for (int i = 0; i < 1000000; i++) {
		Lock();
		a++;
		if (a != 1)
		{
			printf("%d ", a);
		}
		a--;
		Unlock();
		printf("");
	}
}

int main()
{
	std::thread t1(ThreadMain);
	std::thread t2(ThreadMain);
	std::thread t3(ThreadMain);
	t1.join();
	t2.join();
	t3.join();
	return 0;
}
