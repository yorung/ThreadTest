// ThreadTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <thread>
#include <chrono>
#include <set>
#include <shared_mutex>
#include <atomic>

int egis = 0;
class AsmLock {
public:
	void Lock()
	{
		__asm {
			mov eax, 1;
		retry:
			xchg eax, [egis];
			test eax, eax;
			jnz retry;
		}
	}

	void Lock2()
	{
		__asm {
			mov eax, 1;
		retry:
			xchg eax, [egis];
			test eax, eax;
			jz done;
		retry_mov:
			mov ebx, [egis];
			test ebx, ebx;
			jnz retry_mov;
			jmp retry;
		done:
		}
	}

	void Unlock()
	{
		__asm {
			mov eax, 0;
			xchg eax, [egis];
		}
	}

	void Unlock2()
	{
		__asm {
			mov[egis], 0;
		}
	}
};

class SharedMutex {
	std::shared_timed_mutex mutex;
public:
	void Lock()
	{
		mutex.lock();
	}

	void Unlock()
	{
		mutex.unlock();
	}
};

class Atomic {
	std::atomic<int> val = 0;
public:
	void Lock()
	{
		int expected = 0;
		while (!std::atomic_compare_exchange_weak(&val, &expected, 1)) {
			expected = 0;
		}
	}

	void Unlock()
	{
		std::atomic_exchange(&val, 0);
	}
};

//static AsmLock lock;
//static SharedMutex lock;
static Atomic lock;

void IncDecThreadMain()
{
	static int a = 0;
	for (int i = 0; i < 1000000; i++) {
		lock.Lock();
		a++;
		if (a != 1)
		{
			printf("%d ", a);
		}
		a--;
		lock.Unlock();
		printf("");
	}
}

void StlContainerThreadMain()
{
	static std::set<int> c;
	for (int i = 0; i < 1000000; i++) {
		int r = rand() % 1000;
		lock.Lock();
		auto it = c.find(r);
		if (it == c.end()) {
			c.insert(r);
		} else {
			c.erase(it);
		}
		lock.Unlock();
	}
}

double GetTime()
{
	static auto start = std::chrono::high_resolution_clock::now();
	auto now = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(now - start).count();
}

void IncDecTest()
{
	printf("IncDecTest\n");
	double begin = GetTime();
	std::thread t1(IncDecThreadMain);
	std::thread t2(IncDecThreadMain);
	std::thread t3(IncDecThreadMain);
	t1.join();
	t2.join();
	t3.join();
	double end = GetTime();
	printf("elapsed: %f\n", end - begin);
}

void StlContainerTest()
{
	printf("StlContainerTest\n");
	double begin = GetTime();
	std::thread t1(StlContainerThreadMain);
	std::thread t2(StlContainerThreadMain);
	std::thread t3(StlContainerThreadMain);
	t1.join();
	t2.join();
	t3.join();
	double end = GetTime();
	printf("elapsed: %f\n", end - begin);
}

int main()
{
	IncDecTest();
	StlContainerTest();
	return 0;
}
