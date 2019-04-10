#pragma once

#include "Heap.h"
#include <vector>
#include <cassert>

#define MEMSYSTEM_SIGNATURE 0xDEADC0DE
#define MEMSYSTEM_ENDMARKER 0xDEADBEEF

using namespace std;

struct AllocHeader
{
	int iSignature;
	Heap *pHeap;
	int iSize;
	AllocHeader* pNext = nullptr;
	AllocHeader* pPrev = nullptr;
};

class HeapFactory
{
public:
	HeapFactory();
	static void Destruct();

	static Heap* CreateHeap(char* szName);

	static Heap* GetDefaultHeap();

	static void CreateDefaultHeap();

	static int GetTotalAllocatedBytes(int i);

	static bool WalkHeap(int index);//goes through an indexed heap and outputs details. Returns true if the heap is empty


private:
	static vector<Heap*> HeapList;

	static Heap* defaultHeap;

};