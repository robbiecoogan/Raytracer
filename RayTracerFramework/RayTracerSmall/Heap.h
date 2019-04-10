#pragma once

//#define NAMELENGTH 7
#include <iostream>
#include <thread>
#include <mutex>


class Heap
{
public:

	Heap(const char * name);

	const char* GetName() const;
	void Init(char* name);

	void AddAllocation(size_t size);
	void RemoveAllocation(size_t size);
	size_t TotalAllocation() { return m_allocatedBytes; }

	void* firstMem = nullptr;
	void* lastMem = nullptr;

private:
	size_t m_allocatedBytes;
	char* m_name;
};