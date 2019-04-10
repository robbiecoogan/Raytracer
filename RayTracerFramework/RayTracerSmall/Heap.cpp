#include "Heap.h"

Heap::Heap(const char * name)
{
	m_name = (char*)name;
	m_allocatedBytes = 0;
}

const char* Heap::GetName() const
{
	return m_name;
}

void Heap::Init(char* name)
{
	m_name = name;
	m_allocatedBytes = 0;
	lastMem = nullptr;
	firstMem = nullptr;
}

void Heap::AddAllocation(size_t size)
{
	//std::cout << "\nAllocated on '" << GetName() << "' heap. \t|";
	//std::cout << "\n--Allocated " << size << " Bytes  - " << GetName() << "\n";

	m_allocatedBytes += size;

	//std::cout << "\tTotal Allocated: " << m_allocatedBytes << "\n";
}

void Heap::RemoveAllocation(size_t size)
{
	//std::cout << "\nDeleted from '" << m_name << "' heap. \t|";
	//std::cout << "\n--Removed " << size << " Bytes  - " << GetName() << "\n";
	m_allocatedBytes -= size;
	//std::cout << "\tTotal Allocated: " << m_allocatedBytes << "\n";
}