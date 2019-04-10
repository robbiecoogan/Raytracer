#include "Heapfactory.h"


vector<Heap*> HeapFactory::HeapList;
Heap *HeapFactory::defaultHeap;

HeapFactory::HeapFactory()
{
}

Heap* HeapFactory::CreateHeap(char* szName)
{
	Heap* newHeap = (Heap*)malloc(sizeof(Heap(szName)));
	newHeap->Init(szName);

	HeapList.emplace_back(newHeap);

	return newHeap;
}

void HeapFactory::Destruct()
{
	for (int i = 0; i < HeapList.size(); i++)
	{
		delete HeapList[i];
	}

	delete defaultHeap;
}

Heap* HeapFactory::GetDefaultHeap()
{
	if (defaultHeap == NULL)
	{
		CreateDefaultHeap();
		return defaultHeap;
	}
	else return defaultHeap;
}

void HeapFactory::CreateDefaultHeap()
{
	defaultHeap = (Heap*)malloc(sizeof(Heap("Default")));
	defaultHeap->Init((char*)"Default");
}

int HeapFactory::GetTotalAllocatedBytes(int i)
{
	return HeapList[i]->TotalAllocation();
}

bool HeapFactory::WalkHeap(int index)
{
	AllocHeader* firstPoint;

	if (index == 0)
	{
		firstPoint = (AllocHeader*)defaultHeap->firstMem;
		std::cout << "\n||||||||||||||||||||\nWalking through " << GetDefaultHeap()->GetName() << " heap...";
	}
	else
	{
		index--;
		firstPoint = (AllocHeader*)HeapList[index]->firstMem;
		std::cout << "\n||||||||||||||||||||\nWalking through " << HeapList[index]->GetName() << " heap...";
	}


	if (firstPoint)
	{
		//now output the data
		int counter = 0;
		int total = 0;
		Heap* thisHeap = firstPoint->pHeap;
		while (firstPoint)
		{
			size_t totalBytes = 0;
			
			//totalbytes should be equal to the size of the data that has been allocated + the size of the header used to store the info + the size of an int (size of the endmarker)
			totalBytes = firstPoint->iSize + sizeof(AllocHeader) + sizeof(int);
			std::cout << "\nSize of element " << counter+1 << " is: " << totalBytes << " Bytes.";
			total += totalBytes;
			counter++;

			assert(firstPoint->iSignature == MEMSYSTEM_SIGNATURE);

			int* pEndMarker = (int*)(int*)((char*)firstPoint + firstPoint->iSize + sizeof(AllocHeader));
			assert(*pEndMarker == MEMSYSTEM_ENDMARKER);

			firstPoint = firstPoint->pNext;

		}
		//std::cout << "\nTotal Bytes Allocated: " << total << "\n";
		std::cout << "Total Bytes Allocated: " << thisHeap->TotalAllocation() << "\n";
		std::cout << "\nNo memory corruption found.";
	}
	else//if firstpoint is a nullptr, i.e there is no data left in the heap
	{

		std::cout << "No data is left in the heap.\n||||||||||||||||||||\n" << endl;
		return true;
	}
	std::cout << "\n||||||||||||||||||||\n\n";

	return false;
}
