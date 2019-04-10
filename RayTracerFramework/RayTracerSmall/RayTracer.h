#pragma once

// [header]
// A very basic raytracer example.
// [/header]
// [compile]
// c++ -o raytracer -O3 -Wall raytracer.cpp
// [/compile]
// [ignore]
// Copyright (C) 2012  www.scratchapixel.com
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// [/ignore]

//USING	'JSON FOR MODERN C++' by nlohmann (https://github.com/nlohmann/json)
#include <stdlib.h>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <iostream>
#include <cassert>
// Windows only
#include <algorithm>
#include <sstream>
#include <string.h>
////////////////
#include <nlohmann/json.hpp>
using json = nlohmann::json;
////////////////
#include "Sphere.h"
#include <mutex>

typedef std::chrono::milliseconds ms;

#if defined __linux__ || defined __APPLE__
// "Compiled for Linux
#else
// Windows doesn't define these values by default, Linux does
#define M_PI 3.141592653589793
#define INFINITY 1e8
#endif

//[comment]
// This variable controls the maximum recursion depth
//[/comment]
#define MAX_RAY_DEPTH 5

struct frameInfo
{
	unsigned width, height;
	float invWidth, invHeight;
	float fov, aspectratio;
	float angle;
};

class RayTracer
{
public:
	RayTracer();
	~RayTracer();

	void* operator new(size_t size);
	void operator delete(void* p, size_t size);
	void* operator new[](size_t size);
	void operator delete[](void* p, size_t size);

	float mix(const float &a, const float &b, const float &mix){	return b * mix + a * (1 - mix);		}
	Vec3f trace(
		const Vec3f &rayorig,
		const Vec3f &raydir,
		const std::vector<Sphere*> &spheres,
		const int &depth);
	void render(const std::vector<Sphere*> &spheres, int iteration);
	void renderThreaded(const std::vector<Sphere*> &spheres, int iteration);
	void renderThreadBlinds(int threadCount, int threadIndex, Vec3f *image, std::string* writeString);
	void BasicRender();
	void SimpleShrinking();
	void SmoothScaling();
	void JSONAnimate();
	///requires a string of 2 chars to be passed (e.g. FF007600 can be passed in 4 calls of "FF", "00", "76", and "00") this returns a float equal to that hex value from 0-1
	float getfloatfromHex(std::string twoCharHex);
	void ReadJson(char* fileDir);

private:
	int numFrames;
	std::vector<Sphere*> sceneSpheres;
	int numSpheres;

	frameInfo specs;
	//unsigned width, height;

	//float invWidth, invHeight;
	//float fov, aspectratio;
	//float angle;

	int frames;

	static Heap* pHeap;

	int numThreads;

	mutex mtex;

	Vec3f** threadImages;

};