// ThreadTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <thread>
#include <chrono>
#include <set>
#include <vector>
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
	static const int writeLockBit = 0x40000000;

	void LockInternal(int delta, int testBits)
	{
		int expected;
		do {
			do {
				expected = val;
			} while (expected & testBits);
		} while (!std::atomic_compare_exchange_weak(&val, &expected, expected + delta));
	}

public:
	void ReadLock()
	{
		LockInternal(1, writeLockBit);
	}

	void WriteLock()
	{
		LockInternal(writeLockBit, 0xffffffff);
	}

	void WriteUnlock()
	{
		std::atomic_fetch_sub(&val, writeLockBit);
	}

	void ReadUnlock()
	{
		std::atomic_fetch_sub(&val, 1);
	}
};

//static AsmLock lock;
//static SharedMutex lock;
static Atomic lock;

void IncDecThreadMain()
{
	static int a = 0;
	for (int i = 0; i < 1000000; i++) {
		lock.WriteLock();
		a++;
		if (a != 1)
		{
			printf("%d ", a);
		}
		a--;
		lock.WriteUnlock();
		printf("");
	}
}

void StlContainerThreadMain(int id)
{
	static std::set<int> c;
	int readFound = 0;
	int readNotFound = 0;
	for (int i = 0; i < 1000000; i++) {
		int r = rand() % 10000;
		if (i % 10 == 0) {
			lock.ReadLock();
			auto it = c.find(r);
			if (it != c.end()) {
				lock.ReadUnlock();
			} else {
				lock.ReadUnlock();
				lock.WriteLock();
				auto it = c.find(r);
				if (it == c.end()) {
					c.insert(r);
				}
				lock.WriteUnlock();
			}
		} else {
			lock.ReadLock();
			auto it = c.find(r);
			if (it == c.end()) {
				readNotFound++;
			} else {
				readFound++;
			}
			lock.ReadUnlock();
		}
		printf("");
	}
	printf("threadId=%d readFound=%d readNotFound=%d\n", id, readFound, readNotFound);
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
	std::vector<std::thread> t;
	for (int i = 0; i < 10; i++) {
		t.emplace_back(std::thread(StlContainerThreadMain, i));
	}
	for (int i = 0; i < 10; i++) {
		t[i].join();
	}
	double end = GetTime();
	printf("elapsed: %f\n", end - begin);
}

int main()
{
	IncDecTest();
	StlContainerTest();
	return 0;
}
