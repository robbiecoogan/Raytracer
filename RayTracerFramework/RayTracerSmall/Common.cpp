#include "Common.h"

typedef std::chrono::milliseconds ms;

template <typename T>
Heap* Vec3<T>::vecHeap;
Heap* Vec3<float>::vecHeap;

void * operator new(size_t size, Heap * pHeap)
{

	//auto startTime = std::chrono::system_clock::now();

	const size_t requestedBytes = size + sizeof(AllocHeader) + sizeof(int);
	char* pMem = (char*)myPool.allocate(requestedBytes);
	//char* pMem = (char*)malloc(requestedBytes);
	AllocHeader* pHeader = (AllocHeader *)pMem;
	pHeader->iSignature = MEMSYSTEM_SIGNATURE;
	pHeader->pHeap = pHeap;
	pHeader->iSize = size;
	pHeader->pPrev = nullptr;
	pHeader->pNext = nullptr;

	if (pHeap->firstMem == nullptr)
	{
		pHeap->firstMem = pHeader;
	}

	if (pHeap->lastMem)
	{
		AllocHeader* temp = (AllocHeader*)pHeap->lastMem;
		temp->pNext = pHeader;
		pHeader->pPrev = temp;
		pHeap->lastMem = pHeader;
	}

	pHeap->lastMem = pHeader;

	pHeap->AddAllocation(requestedBytes);
	auto* pStartMemBlock = pMem + sizeof(AllocHeader);
	int* pEndMarker = (int*)(pStartMemBlock + size);
	*pEndMarker = MEMSYSTEM_ENDMARKER;

	//auto endTime = std::chrono::system_clock::now() - startTime;
	//ms time = std::chrono::duration_cast<ms>(endTime);
	//cout << "MemoryAlloc Took: " << time.count() << " MS\n";

	return pStartMemBlock;
}

void * operator new(size_t size)
{
	//auto startTime = std::chrono::system_clock::now();
	Heap* pHeap = HeapFactory::GetDefaultHeap();

	const size_t requestedBytes = size + sizeof(AllocHeader) + sizeof(int);//size is the size of the data requested, sizeof(AllocHeader) is the data size of the alloc header, so we can store allocation details, sizeof(int) is the size of the end marker
	char* pMem = (char*)myPool.allocate(requestedBytes);
	//char* pMem = (char*)malloc(requestedBytes);
	AllocHeader* pHeader = (AllocHeader*)pMem;
	pHeader->iSignature = MEMSYSTEM_SIGNATURE;
	pHeader->pHeap = pHeap;
	pHeader->iSize = size;
	pHeader->pPrev = nullptr;
	pHeader->pNext = nullptr;

	if (pHeap->firstMem == nullptr)
	{
		pHeap->firstMem = pHeader;
	}

	if (pHeap->lastMem)
	{
		AllocHeader* temp = (AllocHeader*)pHeap->lastMem;
		temp->pNext = pHeader;
		pHeader->pPrev = temp;
		pHeap->lastMem = pHeader;
	}

	pHeap->lastMem = pHeader;

	pHeap->AddAllocation(requestedBytes);

	auto* pStartMemBlock = pMem + sizeof(AllocHeader);
	int* pEndMarker = (int*)(pStartMemBlock + size);
	*pEndMarker = MEMSYSTEM_ENDMARKER;

	//auto endTime = std::chrono::system_clock::now() - startTime;
	//ms time = std::chrono::duration_cast<ms>(endTime);
	//cout << "MemoryAlloc Took: " << time.count() << " MS\n";

	return pStartMemBlock;
}

void operator delete(void * pMem)
{
	//auto startTime = std::chrono::system_clock::now();
	AllocHeader* pHeader = (AllocHeader *)((char *)pMem - sizeof(AllocHeader));
	assert(pHeader->iSignature == MEMSYSTEM_SIGNATURE);//if the signature is not equal to the Start Marker, this indicates that memory has been overwritten. As such, an error message should display if this happens

													   //now, reorganize the header's pointers to the next and previous memory addresses to prevent crashing.
													   //the below hurt my head to understand

	//if previous object's next isnt equal to the current object, need to fix
	if (pHeader->pPrev->pNext != pHeader)
	{
		pHeader->pPrev->pNext = pHeader;
	}
	if (pHeader->pNext && pHeader->pNext->pPrev != pHeader)
	{
		pHeader->pNext->pPrev = pHeader;
	}


	if (pHeader->pNext == nullptr) pHeader->pHeap->lastMem = pHeader->pPrev;//if we are the last element, we need to ensure that the lastmem now points to the previous address
	if (pHeader->pNext) pHeader->pNext->pPrev = pHeader->pPrev;
	if (pHeader->pPrev) pHeader->pPrev->pNext = pHeader->pNext;
	if (pHeader->pNext == nullptr && pHeader->pPrev == nullptr) pHeader->pHeap->lastMem = nullptr;

	if (pHeader != nullptr)
	{
		int* pEndMarker = (int*)((char*)pMem + pHeader->iSize);
		assert(*pEndMarker == MEMSYSTEM_ENDMARKER);
		pHeader->pHeap->RemoveAllocation(pHeader->iSize + sizeof(AllocHeader) + sizeof(int));
		myPool.freeMem(pHeader, pHeader->iSize + sizeof(AllocHeader) + sizeof(int));
	}


	//auto endTime = std::chrono::system_clock::now() - startTime;
	//ms time = std::chrono::duration_cast<ms>(endTime);
	//cout << "MemoryDel Took: " << time.count() << " MS\n";
}


///////////////////////////////////////////////MEMORY POOLS
MemoryPool::MemoryPool() : head(pool) 
{
}

void* MemoryPool::allocate(size_t size)
{
	if (head == NULL)
	{
		poolSize = 409600000;
		pool = (char*)malloc(poolSize);
		head = pool;
	}
	//auto startTime = std::chrono::system_clock::now();
	void* newMem;

	//first, check through free memory addresses to see if data can be reused
	for (size_t i = 0; i < freeDataMaxSize; i++)
	{
		if (freeData[i].loc && freeData[i].dataSize == size)//if current free location is free & there is enough room for the data
		{
			newMem = freeData[i].loc;
			//if (size < freeData[i].dataSize)//if there is more than enough room, resize the freedata pointer
			//{
			//	std::cout << "------------------------REDECLARED MEMORY: Less memory needed than full space available------------------------\n" << endl;
			//	freeData[i].loc = (char*)freeData[i].loc + size;
			//	freeData[i].dataSize -= size;
			//}
			if (freeData[i].dataSize == size)//if there is exactly enough room, flag the freedata object as available to reflag
			{
				//std::cout << "\n||||||||||||||||||||REDECLARED MEMORY|||||||||||||||||||| - Memory is exact size of space available. Reusing Free Slot||||||||||||||||||||" << endl;
				freeData[i].loc = nullptr;
				freeData[i].dataSize = 0;
			}
			return newMem;
		}
		else if (i == 9)//if at the end of the free data list without being able to reuse data, we need to defrag memory to make a useable slot somewhere
		{
			//CheckDefrag();
		}
	}

	//if there is no room in the free data, just create new data
	//assert((head + size) <= (pool + poolSize) && "No room left in pool!");//assert that the data being added should not go past the end of the pool (prevent memory corruption)
	if ((head + size) >= pool + poolSize)//if all memory in the pool is used, make another pool.
	{
		pool = (char*)malloc(poolSize);
		head = pool;
	}
	newMem = head;
	head = head + size;
	//cout << "\n||||||||||||||||||||MEMORY IS CURRENTLY AT: " << (char*)head - pool << " Bytes used||||||||||||||||||||";
	//auto endTime = std::chrono::system_clock::now() - startTime;
	//ms time = std::chrono::duration_cast<ms>(endTime);
	//cout << "PoolAlloc Took: " << time.count() << " MS\n";
	return newMem;
}

void MemoryPool::freeMem(void * pMem, size_t size)
{
	//auto startTime = std::chrono::system_clock::now();
	//flag memory as freed
	bool duplicate = false;
	for (size_t i = 0; i < freeDataMaxSize; i++)
	{
		if (i > 0 && freeData[i - 1].loc == pMem) duplicate = true;
		if (i < freeDataMaxSize-1 && freeData[i + 1].loc == pMem) duplicate = true;
	}
	for (size_t i = 0; i < freeDataMaxSize; i++)
	{
		//if (checkHeadReposition(pMem, size))
		//{

		//}
		if (freeData[i].loc == nullptr && duplicate == false)
		{
			freeData[i].loc = pMem;
			freeData[i].dataSize = size;
			break;
		}
	}
	//auto endTime = std::chrono::system_clock::now() - startTime;
	//ms time = std::chrono::duration_cast<ms>(endTime);
	//cout << "FreeMem Took: " << time.count() << " MS\n";
}

bool MemoryPool::checkHeadReposition(void * pMem, size_t size)
{
	//checks whether the freed memory is at the end of the head, and if it is, this function is repeatedly ran to check every freedata to ensure that the pool head is at the correct location
	for (size_t i = 0; i < freeDataMaxSize; i++)
	{
		if ((char*)((char*)freeData[i].loc + freeData[i].dataSize) == head)//is the end of the freedata equal to the head, that means the head needs to be moved back.
		{
			head = (char*)pMem;
			//call this function again, until we can loop without this if statement becoming true
			checkHeadReposition(pMem, size);
			return true;
		}
	}
	return false;

}

void MemoryPool::CheckDefrag()
{
	if (freeData[freeDataMaxSize-1].loc && freeData[0].loc)
	{
		//sort the freeData by RAM position to be able to step through and grab all valid data between free nodes
		FreePointer copy;
		std::cout << "SORTING\n";
		for (size_t i = 0; i < freeDataMaxSize; i++)
		{
			if (i > 0 && freeData[i].loc < freeData[i - 1].loc)//if the data we are currently looking at is earlier in memory than the one before it
			{
				//swap data
				copy = freeData[i - 1];
				freeData[i - 1] = freeData[i];
				freeData[i] = copy;
				i -= 2;//(ends up only going back one as the loop itself adds 1)

			}
		}
		for (size_t i = 0; i < 10; i++)
		{
			std::cout << "Element " << i << ": " << freeData[i].loc << endl;
		}
		//Above has been checked, and does sort the data
		//Starting at the earliest element in RAM's end point, traverse the data until the next free position header and move each part backwards by the necessary amount of room
		char* memPoint;//end of the current free memory. i.e. the start point for the iteration
		size_t moveBytes = 0;//how many bytes each byte in the pool needs to be moved back
		for (size_t j = 0; j < freeDataMaxSize-1; j++)
		{
			if (freeData[j].loc)
			{
				moveBytes = freeData[j].dataSize;
				memPoint = (char*)freeData[j].loc + moveBytes;
				//Cast the object that we know is valid data into an allocHeader, and tell it's next and previous objects that it has moved memory address.
				if (!(memPoint == freeData[j + 1].loc))//if the next position in memory is free, and there isn't useable data, just jump to combining the data
				{
					AllocHeader* temp = (AllocHeader*)memPoint;
					if (temp->pHeap->lastMem == temp) temp->pHeap->lastMem = (memPoint - moveBytes);
					if (temp->pNext)temp->pNext->pPrev = (AllocHeader*)(memPoint - moveBytes);
					if (temp->pPrev)temp->pPrev->pNext = (AllocHeader*)(memPoint - moveBytes);
					int validDataSize = ((char*)freeData[j + 1].loc - ((char*)freeData[j].loc + freeData[j].dataSize));//the size of the valid data that we are moving backwards before reaching the next freeData node.
					for (size_t i = 0; i < validDataSize; i++)//iterate from the end of the current free mem to the start of the next free mem
					{
						*(memPoint - moveBytes) = *memPoint;//change element [x] positions behind to the element we are currently at.			
						memPoint = (char*)memPoint + 1;
					}
				}
				freeData[j + 1].loc = (char*)freeData[j + 1].loc - moveBytes;
				freeData[j + 1].dataSize += moveBytes;//once we reach the end of the current free data, we will meet up with the next piece of free data in the heap, so set it's data size to include ours too
													  //flag the current free data as reusable
				freeData[j].loc = nullptr;
				freeData[j].dataSize = 0;
			}
		}
		//once we have reached the end of all the defragmentation, we can reset the header of the pool to the final location
		head = (char*)freeData[freeDataMaxSize-1].loc;
		freeData[freeDataMaxSize-1].loc = nullptr;
		freeData[freeDataMaxSize-1].dataSize = 0;
	}
}
