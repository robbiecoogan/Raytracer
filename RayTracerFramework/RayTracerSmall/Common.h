#pragma once

#include <vector>
#include "HeapFactory.h"
#include <cassert>
#include <chrono>
#include <mutex>

struct FreePointer//contains a pointer and a data size to reference free memory
{
	void* loc = nullptr;
	size_t dataSize = 0;
};

class MemoryPool
{
public:
	MemoryPool();
	void* allocate(size_t size);
	void freeMem(void* pMem, size_t size);
	bool checkHeadReposition(void* pMem, size_t size);

private:
	int poolSize;
	char* pool;
	char* head;

	int freeDataMaxSize = 3000;
	FreePointer freeData[3000];
	void CheckDefrag();
};

void * operator new(size_t size, Heap * pHeap);
void * operator new(size_t size);
void operator delete(void * pMem);
static MemoryPool myPool;




template<typename T>
class Vec3
{
public:

	void* operator new(size_t size);
	void* operator new[](size_t size);
	void operator delete(void* p, size_t size);
	void operator delete[](void* p, size_t size);

	T x, y, z;
	Vec3() : x(T(0)), y(T(0)), z(T(0)) {}
	Vec3(T xx) : x(xx), y(xx), z(xx) {}
	Vec3(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {}
	Vec3& normalize()
	{
		T nor2 = length2();
		if (nor2 > 0) {
			T invNor = 1 / sqrt(nor2);
			x *= invNor, y *= invNor, z *= invNor;
		}
		return *this;
	}
	Vec3<T> operator * (const T &f) const { return Vec3<T>(x * f, y * f, z * f); }
	Vec3<T> operator * (const Vec3<T> &v) const { return Vec3<T>(x * v.x, y * v.y, z * v.z); }
	T dot(const Vec3<T> &v) const { return x * v.x + y * v.y + z * v.z; }
	Vec3<T> operator - (const Vec3<T> &v) const { return Vec3<T>(x - v.x, y - v.y, z - v.z); }
	Vec3<T> operator + (const Vec3<T> &v) const { return Vec3<T>(x + v.x, y + v.y, z + v.z); }
	Vec3<T>& operator += (const Vec3<T> &v) { x += v.x, y += v.y, z += v.z; return *this; }
	Vec3<T>& operator *= (const Vec3<T> &v) { x *= v.x, y *= v.y, z *= v.z; return *this; }
	Vec3<T> operator - () const { return Vec3<T>(-x, -y, -z); }
	T length2() const { return x * x + y * y + z * z; }
	T length() const { return sqrt(length2()); }
	friend std::ostream & operator << (std::ostream &os, const Vec3<T> &v)
	{
		os << "[" << v.x << " " << v.y << " " << v.z << "]";
		return os;
	}

	static Heap* vecHeap;
};

typedef Vec3<float> Vec3f;

template<typename T>
inline void * Vec3<T>::operator new(size_t size)
{
	std::cout << "\nVEC 3 (" << typeid(T).name() << ") CREATED";
	if (vecHeap == NULL)
	{
		return ::operator new(size, vecHeap = HeapFactory::CreateHeap((char*)"Vec3"));
	}
	return ::operator new(size, vecHeap);
}

template<typename T>
inline void * Vec3<T>::operator new[](size_t size)
{
	return operator new(size);
}

template<typename T>
inline void Vec3<T>::operator delete(void * p, size_t size)
{
	std::cout << "\nVEC 3 (" << typeid(T).name() << ") DELETED";
	return ::operator delete(p);
}
template<typename T>
inline void Vec3<T>::operator delete[](void * p, size_t size)
{
	
	return operator delete(p, size);
}
/////
struct keyframe
{
public:
	int timelineLoc;
	double xPos;
	double yPos;
	double zPos;
	double scale;
	Vec3f kColor;
};

