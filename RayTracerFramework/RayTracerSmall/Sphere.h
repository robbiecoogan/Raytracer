#pragma once

#include "Common.h"

class Sphere
{

public:
	Vec3f center;                           /// position of the sphere
	float radius, radius2;                  /// sphere radius and radius^2
	Vec3f surfaceColor, emissionColor;      /// surface color and emission (light)
	float transparency, reflection;         /// surface transparency and reflectivity
	std::vector<keyframe> keyframes;
	Sphere(const Vec3f &c,	const float &r,	const Vec3f &sc,	const float &refl = 0,	const float &transp = 0,	const Vec3f &ec = 0);
	void* operator new(size_t size);
	void operator delete(void* p, size_t size);
	void* operator new[](size_t size);
	void operator delete[](void* p, size_t size);
	//[comment]
	// Compute a ray-sphere intersection using the geometric solution
	//[/comment]
	bool intersect(const Vec3f &rayorig, const Vec3f &raydir, float &t0, float &t1) const;
	void Update(int frameNum);//this is called to update the position, colour, and scale of this sphere before rendering
	int totalFrames;
private:
	static Heap* pHeap;
};