// ThreadTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <thread>
#include <chrono>

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
	done:
	}
}

void Lock2()
{
	__asm {
		mov eax, 1;
	retry:
		xchg eax, [egis]
		test eax, eax
		jz done
		retry_mov :
		mov ebx, [egis]
		test ebx, ebx
		jnz retry_mov
		jmp retry
	done:
	}
}

void Unlock()
{
	__asm {
		mov eax, 0;
		xchg eax, [egis]
	}
}

void Unlock2()
{
	__asm {
		mov[egis], 0;
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

double GetTime()
{
	static auto start = std::chrono::high_resolution_clock::now();
	auto now = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(now - start).count();
}

int main()
{
	double begin = GetTime();
	std::thread t1(ThreadMain);
	std::thread t2(ThreadMain);
	std::thread t3(ThreadMain);
	t1.join();
	t2.join();
	t3.join();
	double end = GetTime();

	printf("elapsed: %f", end - begin);

	return 0;
}
