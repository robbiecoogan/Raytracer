#include "RayTracer.h"
#include <thread>


RayTracer::RayTracer()
{
	numFrames = 0;
	numSpheres = 0;

	// quick and dirty
	//width = 1920;
	//height = 1080;

	//invWidth = 1 / float(width);
	//invHeight = 1 / float(height);
	//fov = 30;
	//aspectratio = width / float(height);
	//angle = tan(M_PI * 0.5 * fov / 180.);

	//frames = 100;
	specs.width = 1920;
	specs.height = 1080;

	specs.invWidth = 1 / float(specs.width);
	specs.invHeight = 1 / float(specs.height);
	specs.fov = 30;
	specs.aspectratio = specs.width / float(specs.height);
	specs.angle = tan(M_PI * 0.5 * specs.fov / 180.);

	frames = 100;

	numThreads = std::thread::hardware_concurrency();
	threadImages = new Vec3f*[numThreads];
	for (size_t i = 0; i < numThreads; i++)
	{
		threadImages[i] = (new Vec3f[(specs.height / numThreads) * specs.width]);
	}
}

Heap* RayTracer::pHeap;

RayTracer::~RayTracer()
{
	for (size_t i = 0; i < numThreads; i++)
	{
		delete[] threadImages[i];
	}

}

void * RayTracer::operator new(size_t size)
{
	std::cout << "Created RayTracer\n";
	if (pHeap == NULL)
	{
		return ::operator new(size, pHeap = HeapFactory::CreateHeap((char*)"RayTracer"));
	}
	return ::operator new(size, pHeap);
}

void RayTracer::operator delete(void * p, size_t size)
{
	std::cout << "Deleted RayTracer\n";
	return ::operator delete(p);
}

void * RayTracer::operator new[](size_t size)
{
	return operator new(size);
}

void RayTracer::operator delete[](void * p, size_t size)
{
	return operator delete(p, size);
}

Vec3f RayTracer::trace(const Vec3f & rayorig, const Vec3f & raydir, const std::vector<Sphere*> &spheres, const int & depth)
{

	//if (raydir.length() != 1) std::cerr << "Error " << raydir << std::endl;
	float tnear = INFINITY;
	const Sphere* sphere = NULL;
	// find intersection of this ray with the sphere in the scene
	for (unsigned i = 0; i < numSpheres; ++i) {
		float t0 = INFINITY, t1 = INFINITY;
		if (spheres[i]->intersect(rayorig, raydir, t0, t1)) {
			if (t0 < 0) t0 = t1;
			if (t0 < tnear) {
				tnear = t0;
				sphere = spheres[i];
			}
		}
	}
	// if there's no intersection return black or background color
	if (!sphere) return Vec3f(2);
	Vec3f surfaceColor = 0; // color of the ray/surfaceof the object intersected by the ray
	Vec3f phit = rayorig + raydir * tnear; // point of intersection
	Vec3f nhit = phit - sphere->center; // normal at the intersection point
	nhit.normalize(); // normalize normal direction
					  // If the normal and the view direction are not opposite to each other
					  // reverse the normal direction. That also means we are inside the sphere so set
					  // the inside bool to true. Finally reverse the sign of IdotN which we want
					  // positive.
	float bias = 1e-4; // add some bias to the point from which we will be tracing
	bool inside = false;
	float rayDirDot = raydir.dot(nhit);
	if (rayDirDot > 0) nhit = -nhit, inside = true;
	if ((sphere->transparency > 0 || sphere->reflection > 0) && depth < MAX_RAY_DEPTH) {
		float facingratio = -rayDirDot;
		// change the mix value to tweak the effect
		float fresneleffect = mix(pow(1 - facingratio, 3), 1, 0.1);
		// compute reflection direction (not need to normalize because all vectors
		// are already normalized)
		Vec3f refldir = raydir - nhit * 2 * rayDirDot;
		refldir.normalize();
		Vec3f reflection = trace(phit + nhit * bias, refldir, spheres, depth + 1);
		Vec3f refraction = 0;
		// if the sphere is also transparent compute refraction ray (transmission)
		if (sphere->transparency) {
			float ior = 1.1, eta = (inside) ? ior : 1 / ior; // are we inside or outside the surface?
			float cosi = -nhit.dot(raydir);
			float k = 1 - eta * eta * (1 - cosi * cosi);
			Vec3f refrdir = raydir * eta + nhit * (eta *  cosi - sqrt(k));
			refrdir.normalize();
			refraction = trace(phit - nhit * bias, refrdir, spheres, depth + 1);
		}
		// the result is a mix of reflection and refraction (if the sphere is transparent)
		surfaceColor = (
			reflection * fresneleffect +
			refraction * (1 - fresneleffect) * sphere->transparency) * sphere->surfaceColor;
	}
	else {
		// it's a diffuse object, no need to raytrace any further
		for (unsigned i = 0; i < numSpheres; ++i) {
			if (spheres[i]->emissionColor.x > 0) {
				// this is a light
				Vec3f transmission = 1;
				Vec3f lightDirection = spheres[i]->center - phit;
				lightDirection.normalize();
				for (unsigned j = 0; j < numSpheres; ++j) {
					if (i != j) {
						float t0, t1;
						if (spheres[j]->intersect(phit + nhit * bias, lightDirection, t0, t1)) {
							transmission = 0;
							break;
						}
					}
				}
				surfaceColor += sphere->surfaceColor * transmission *
					std::max(float(0), nhit.dot(lightDirection)) * spheres[i]->emissionColor;
			}
		}
	}

	return surfaceColor + sphere->emissionColor;
}

void RayTracer::render(const std::vector<Sphere*> &spheres, int iteration)
{
	Vec3f *image = new Vec3f[specs.width * specs.height], *pixel = image;

	// Trace rays
	for (unsigned y = 0; y < specs.height; ++y) {
		for (unsigned x = 0; x < specs.width; ++x, ++pixel) {
			float xx = (2 * ((x + 0.5) * specs.invWidth) - 1) * specs.angle * specs.aspectratio;
			float yy = (1 - 2 * ((y + 0.5) * specs.invHeight)) * specs.angle;
			Vec3f raydir(xx, yy, -1);
			raydir.normalize();
			*pixel = trace(Vec3f(0), raydir, spheres, 0);

		}
	}
	// Save result to a PPM image (keep these flags if you compile under Windows)
	std::stringstream ss;
	ss << "./spheres" << iteration << ".ppm";
	std::string tempString = ss.str();
	char* filename = (char*)tempString.c_str();

	std::ofstream ofs(filename, std::ios::out | std::ios::binary);
	ofs << "P6\n" << specs.width << " " << specs.height << "\n255\n";
	for (unsigned i = 0; i < specs.width * specs.height; ++i) {
		ofs << (unsigned char)(std::min(float(1), image[i].x) * 255) <<
			(unsigned char)(std::min(float(1), image[i].y) * 255) <<
			(unsigned char)(std::min(float(1), image[i].z) * 255);
	}
	ofs.close();

	delete[] image;
}

void RayTracer::renderThreaded(const std::vector<Sphere*>& spheres, int iteration)
{
	// Save result to a PPM image (keep these flags if you compile under Windows)
	std::stringstream ss;
	ss << "./spheres" << iteration << ".ppm";
	std::string tempString = ss.str();
	char* filename = (char*)tempString.c_str();
	std::stringstream writeStream;

	std::ofstream ofs(filename, std::ios::out | std::ios::binary);
	ofs << "P6\n" << specs.width << " " << specs.height << "\n255\n";
	Vec3f *pixel;

	std::string** threadStrings = new std::string*[numThreads];
	std::thread** activeThreads = new std::thread*[numThreads];

	for (size_t i = 0; i < numThreads; i++)//create a number of threads equal to the number of cores on the machine
	{
		threadStrings[i] = new std::string("");		
		activeThreads[i] = new std::thread(&RayTracer::renderThreadBlinds, this, numThreads, i, threadImages[i], threadStrings[i]);
	}

	for (size_t i = 0; i < numThreads; i++)
	{
		activeThreads[i]->join();

		writeStream << *threadStrings[i];
		delete activeThreads[i];
		delete threadStrings[i];
	}
	//auto startTime = std::chrono::system_clock::now();
	std::string strConv = writeStream.str();
	//ofs.write(strConv.c_str(), (width * height) * 3);
	ofs.flush();//push current data to file before pushing hte rest. This improves speed significantly.
	ofs << strConv;
	ofs.close();

	//auto endTime = std::chrono::system_clock::now() - startTime;

	//ms time = std::chrono::duration_cast<ms>(endTime);
	//cout << "FileSave Took: " << time.count() << " MS\n";
}

void RayTracer::renderThreadBlinds(int threadCount, int threadIndex, Vec3f *image, std::string* writeString)//this is called by renderThreaded once the amount of threads is calculated, and this function is assigned to multiple threads to run simultaneously.
{
	
	Vec3f *pixel = image;
	// Trace rays
	for (unsigned y = 0; y < specs.height/threadCount; ++y) {
		for (unsigned x = 0; x < specs.width; ++x, ++pixel) {
			float xx = (2 * ((x + 0.5) * specs.invWidth) - 1) * specs.angle * specs.aspectratio;
			float yy = (1 - 2 * ((y + (specs.height / threadCount) * threadIndex) + 0.5) * specs.invHeight) * specs.angle;
			Vec3f raydir(xx, yy, -1);
			raydir.normalize();
			
			*pixel = trace(Vec3f(0), raydir, sceneSpheres, 0);

			*writeString += (unsigned char)(std::min(float(1), pixel->x) * 255);
			*writeString += (unsigned char)(std::min(float(1), pixel->y) * 255);
			*writeString += (unsigned char)(std::min(float(1), pixel->z) * 255);

		}
	}

}

void RayTracer::BasicRender()
{
	std::vector<Sphere*> spheres;
	// Vector structure for Sphere (position, radius, surface color, reflectivity, transparency, emission color)

	spheres.push_back(new Sphere(Vec3f(0.0, -10004, -20), 10000, Vec3f(0.20, 0.20, 0.20), 0, 0.0));
	spheres.push_back(new Sphere(Vec3f(0.0, 0, -20), 4, Vec3f(1.00, 0.32, 0.36), 1, 0.5)); // The radius paramter is the value we will change
	spheres.push_back(new Sphere(Vec3f(5.0, -1, -15), 2, Vec3f(0.90, 0.76, 0.46), 1, 0.0));
	spheres.push_back(new Sphere(Vec3f(5.0, 0, -25), 3, Vec3f(0.65, 0.77, 0.97), 1, 0.0));

	// This creates a file, titled 1.ppm in the current working directory
	render(spheres, 1);
}

void RayTracer::SimpleShrinking()
{
	std::vector<Sphere*> spheres;
	// Vector structure for Sphere (position, radius, surface color, reflectivity, transparency, emission color)

	for (int i = 0; i < 4; i++)
	{
		if (i == 0)
		{
			spheres.push_back(new Sphere(Vec3f(0.0, -10004, -20), 10000, Vec3f(0.20, 0.20, 0.20), 0, 0.0));
			spheres.push_back(new Sphere(Vec3f(0.0, 0, -20), 4, Vec3f(1.00, 0.32, 0.36), 1, 0.5)); // The radius paramter is the value we will change
			spheres.push_back(new Sphere(Vec3f(5.0, -1, -15), 2, Vec3f(0.90, 0.76, 0.46), 1, 0.0));
			spheres.push_back(new Sphere(Vec3f(5.0, 0, -25), 3, Vec3f(0.65, 0.77, 0.97), 1, 0.0));

		}
		else if (i == 1)
		{
			spheres.push_back(new Sphere(Vec3f(0.0, -10004, -20), 10000, Vec3f(0.20, 0.20, 0.20), 0, 0.0));
			spheres.push_back(new Sphere(Vec3f(0.0, 0, -20), 3, Vec3f(1.00, 0.32, 0.36), 1, 0.5)); // Radius--
			spheres.push_back(new Sphere(Vec3f(5.0, -1, -15), 2, Vec3f(0.90, 0.76, 0.46), 1, 0.0));
			spheres.push_back(new Sphere(Vec3f(5.0, 0, -25), 3, Vec3f(0.65, 0.77, 0.97), 1, 0.0));
		}
		else if (i == 2)
		{
			spheres.push_back(new Sphere(Vec3f(0.0, -10004, -20), 10000, Vec3f(0.20, 0.20, 0.20), 0, 0.0));
			spheres.push_back(new Sphere(Vec3f(0.0, 0, -20), 2, Vec3f(1.00, 0.32, 0.36), 1, 0.5)); // Radius--
			spheres.push_back(new Sphere(Vec3f(5.0, -1, -15), 2, Vec3f(0.90, 0.76, 0.46), 1, 0.0));
			spheres.push_back(new Sphere(Vec3f(5.0, 0, -25), 3, Vec3f(0.65, 0.77, 0.97), 1, 0.0));
		}
		else if (i == 3)
		{
			spheres.push_back(new Sphere(Vec3f(0.0, -10004, -20), 10000, Vec3f(0.20, 0.20, 0.20), 0, 0.0));
			spheres.push_back(new Sphere(Vec3f(0.0, 0, -20), 1, Vec3f(1.00, 0.32, 0.36), 1, 0.5)); // Radius--
			spheres.push_back(new Sphere(Vec3f(5.0, -1, -15), 2, Vec3f(0.90, 0.76, 0.46), 1, 0.0));
			spheres.push_back(new Sphere(Vec3f(5.0, 0, -25), 3, Vec3f(0.65, 0.77, 0.97), 1, 0.0));
		}

		render(spheres, i);
		// Dont forget to clear the Vector holding the spheres.
		spheres.clear();
	}
}

void RayTracer::SmoothScaling()
{
	std::vector<Sphere*> spheres;
	// Vector structure for Sphere (position, radius, surface color, reflectivity, transparency, emission color)
	for (float r = 0; r <= 100; r++)
	{
		spheres.push_back(new Sphere(Vec3f(0.0, -10004, -20), 10000, Vec3f(0.20, 0.20, 0.20), 0, 0.0));
		spheres.push_back(new Sphere(Vec3f(0.0, 0, -20), r / 10, Vec3f(1.00, 0.32, 0.36), 1, 0.5)); // Radius++ change here
		spheres.push_back(new Sphere(Vec3f(5.0, -1, -15), 2, Vec3f(0.90, 0.76, 0.46), 1, 0.0));
		spheres.push_back(new Sphere(Vec3f(5.0, 0, -25), 3, Vec3f(0.65, 0.77, 0.97), 1, 0.0));
		render(spheres, r);

		render(spheres, r);
		std::cout << "Rendered and saved spheres" << r << ".ppm" << std::endl;
		// Dont forget to clear the Vector holding the spheres.
		spheres.clear();
	}
}

void RayTracer::JSONAnimate()
{

	ms totalTime = std::chrono::duration_values<ms>::zero();
	std::ofstream ofs("renderInfo.txt", std::ios::out | std::ios::binary);

	for (unsigned r = 0; r < numFrames; r++)
	{
		auto startTime = std::chrono::system_clock::now();

		for (size_t i = 0; i < numSpheres; i++)
		{
			sceneSpheres[i]->Update(r);
		}
		///////////////////////////
		renderThreaded(sceneSpheres, r);
		//render(sceneSpheres, r);
		///////////////////////////
		std::cout << "\nRendered and saved spheres" << r << ".ppm" << std::endl;

		auto endTime = std::chrono::system_clock::now() - startTime;

		ms time = std::chrono::duration_cast<ms>(endTime);
		totalTime += time;
		int convTime = time.count();
		cout << "Took: " << convTime << " MS\n";
		cout << "Total Time: " << totalTime.count() << "MS";
		ofs << "Frame " << r << ": " << convTime << "\r\n";
	}
	ofs << "Total Time :" << totalTime.count() << "\r\n";
	ofs.close();
	HeapFactory::WalkHeap(1);
	HeapFactory::WalkHeap(2);
	HeapFactory::WalkHeap(3);
	int asd = 0;
	std::cin >> asd;
}

float RayTracer::getfloatfromHex(std::string twoCharHex)
{
	//only works when passed 2 chars as a string
	if (twoCharHex.size() != 2)
		return -1.0f;

	//hex values can have from 0-255 represented per every 2 char string.
	int totalNum = 0;

	//firstly, get the value of the first char
	char firstChar = twoCharHex[0];

	if ((firstChar - '0') <= 9)
		totalNum += (firstChar - '0');//converts to numerical int (e.g. char '4' will return int 4, rather than returning the ascii for '4') this is multiplied by 16 to count the hex correctly
	else
	{
		if (firstChar == 'a' || firstChar == 'A') totalNum += 10;
		else if (firstChar == 'b' || firstChar == 'B') totalNum += 11;
		else if (firstChar == 'c' || firstChar == 'C') totalNum += 12;
		else if (firstChar == 'd' || firstChar == 'D') totalNum += 13;
		else if (firstChar == 'e' || firstChar == 'E') totalNum += 14;
		else if (firstChar == 'f' || firstChar == 'F') totalNum += 15;
	}

	totalNum *= 16;

	//now get the value of the second char
	char secondChar = twoCharHex[1];

	if ((secondChar - '0') <= 9)
		totalNum += (secondChar - '0') * 16;//converts to numerical int (e.g. char '4' will return int 4, rather than returning the ascii for '4') this is not multiplied by 16 as it counts singularly
	else
	{
		if (secondChar == 'a' || firstChar == 'A') totalNum += 10;
		else if (secondChar == 'b' || firstChar == 'B') totalNum += 11;
		else if (secondChar == 'c' || firstChar == 'C') totalNum += 12;
		else if (secondChar == 'd' || firstChar == 'D') totalNum += 13;
		else if (secondChar == 'e' || firstChar == 'E') totalNum += 14;
		else if (secondChar == 'f' || firstChar == 'F') totalNum += 15;
	}

	//now i should have a number from 0-255 representing the two chars as a color
	//return the adjusted float value
	float returnVal = (1.0f / 255.0f) * totalNum;
	return returnVal;
}

void RayTracer::ReadJson(char * fileDir)
{
	std::ifstream inFile(fileDir, std::ifstream::in);

	json j = json::parse(inFile);

	for (size_t i = 0; i < j.size(); i++)
	{
		Vec3f centrePos = Vec3f(0.0f, 0.0f, 0.0f);//set to default in case no keyframes are provided
		std::string defaultColorJSON = j[i]["defaultCol"];
		int minframe = j[i]["minFrame"];
		int maxFrame = j[i]["maxFrame"];
		//numFrames = maxFrame - minframe;
		//numFrames++;
		numFrames = 100;
		Vec3f col = Vec3f(getfloatfromHex(defaultColorJSON.substr(3, 2)), getfloatfromHex(defaultColorJSON.substr(5, 2)), getfloatfromHex(defaultColorJSON.substr(7, 2)));
		float scale = 1.0f;

		int keyframesCount = j[i]["keyframes"].size();
		double xPos, yPos, zPos;
		xPos = j[i]["keyframes"][0]["xPos"];
		yPos = j[i]["keyframes"][0]["yPos"];
		zPos = (j[i]["keyframes"][0]["zPos"]);
		zPos = -zPos;//zPos needs to be inverted a the z direction is the opposite of the one used in the editor

		if (keyframesCount != 0)
			centrePos = Vec3f(xPos, yPos, zPos - 10);

		Sphere* newSphere = new Sphere(centrePos, scale, col, 1, 0.0f);
		newSphere->totalFrames = numFrames;

		for (size_t k = 0; k < keyframesCount; k++)
		{
			int timelineLoc = j[i]["keyframes"][k]["TimelineLoc"];
			double xPos = j[i]["keyframes"][k]["xPos"];
			double yPos = j[i]["keyframes"][k]["yPos"];
			double zPos = j[i]["keyframes"][k]["zPos"];
			zPos = -zPos;
			scale = j[i]["keyframes"][k]["scale"];
			std::string kColorString = j[i]["keyframes"][k]["kColor"];
			col = Vec3f(getfloatfromHex(kColorString.substr(3, 2)), getfloatfromHex(kColorString.substr(5, 2)), getfloatfromHex(kColorString.substr(7, 2)));
			keyframe newFrame = keyframe{ timelineLoc, xPos, yPos, -zPos - 10, scale, col };
			newSphere->keyframes.emplace_back(newFrame);
		}
		sceneSpheres.emplace_back(newSphere);
		numSpheres++;
	}
}


