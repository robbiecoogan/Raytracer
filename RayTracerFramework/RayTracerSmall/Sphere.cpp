#include "Sphere.h"

typedef std::chrono::milliseconds ms;

Sphere::Sphere(
	const Vec3f &c,
	const float &r,
	const Vec3f &sc,
	const float &refl,
	const float &transp,
	const Vec3f &ec) :
	center(c), radius(r), radius2(r * r), surfaceColor(sc), emissionColor(ec),
	transparency(transp), reflection(refl), totalFrames(100)
{ /* empty, used for construction */
}

Heap* Sphere::pHeap;

void * Sphere::operator new(size_t size)
{
	std::cout << "\nCreated Sphere";
	if (pHeap == NULL)
	{
		return ::operator new(size, pHeap = HeapFactory::CreateHeap((char*)"Sphere"));
	}
	return ::operator new(size, pHeap);
}

void Sphere::operator delete(void * p, size_t size)
{
	std::cout << "\nDeleted Sphere";
	return ::operator delete(p);
}

void * Sphere::operator new[](size_t size)
{
	return operator new(size);
}

void Sphere::operator delete[](void * p, size_t size)
{
	return operator delete(p, size);
}

//[comment]
// Compute a ray-sphere intersection using the geometric solution
//[/comment]
bool Sphere::intersect(const Vec3f &rayorig, const Vec3f &raydir, float &t0, float &t1) const
{
	Vec3f l = center - rayorig;
	float tca = l.dot(raydir);
	if (tca < 0) return false;
	float d2 = l.dot(l) - tca * tca;
	if (d2 > radius2) return false;
	float thc = sqrt(radius2 - d2);
	t0 = tca - thc;
	t1 = tca + thc;
	return true;
}

void Sphere::Update(int frameNum)
{
	//auto startTime = std::chrono::system_clock::now();
	//firstly, dont bother doing anything if we don't have any keyframes
	int keyframeCount = keyframes.size();

	if (keyframeCount <= 0)
		return;

	//now, we need to find out what keyframes we are between, or if we are on a keyframe, so we can interpolate our position, colour, and scale appropriately
	for (size_t i = 0; i < keyframeCount-1; i++)//iterate through keyframes
	{
		//need to scale everything appropriately by the amount of frames (if framerate is 100 with 100 frames it should look the same as framerate 200 with 200 frames)
		float frameRateScale = ((totalFrames - 1) / 100.0f);//this will be used to alter the position of keyframes (101 is predefined as this is the hardcoded output from the editor)


		//details of current keyframe
		int frameLoc = (keyframes[i].timelineLoc * frameRateScale);
		double frameXPos = keyframes[i].xPos;
		double frameYPos = keyframes[i].yPos;
		double frameZPos = keyframes[i].zPos;
		double frameScale = keyframes[i].scale;
		Vec3f frameCol = keyframes[i].kColor;

		//details of the next keyframe
		int NframeLoc = (keyframes[i+1].timelineLoc * frameRateScale);
		double NframeXPos = keyframes[i+1].xPos;
		double NframeYPos = keyframes[i+1].yPos;
		double NframeZPos = keyframes[i+1].zPos;
		double NframeScale = keyframes[i+1].scale;
		Vec3f NframeCol = keyframes[i+1].kColor;

		//need to find out if the current frame is before the first frame, after the last frame, or between frames (or equal to the frames)
		if (frameNum >= frameLoc && frameNum <= NframeLoc)//if current frame is between or on the i keyframe and it's next keyframe
		{
			//interpolate			

			//get total frames between keyframes
			int animFrames = NframeLoc - frameLoc;
			//get how far through the frames we are
			int animCurrentFrame = frameNum - frameLoc;

			//interpolate position
			float xDiff = NframeXPos - frameXPos;//difference between end frame xPos & start frame xPos
			float yDiff = NframeYPos - frameYPos;//difference between end frame yPos & start frame yPos
			float zDiff = NframeZPos - frameZPos;//difference between end frame zPos & start frame zPos
			float xAddPos = (xDiff / animFrames) * animCurrentFrame;//amount that needs to be added to the X position of the start keyframe to reach the correct interpolation pos
			float yAddPos = (yDiff / animFrames) * animCurrentFrame;
			float zAddPos = (zDiff / animFrames) * animCurrentFrame;
			//set the position to the newly interpolated position
			center = Vec3f(frameXPos + xAddPos, frameYPos + yAddPos, frameZPos + zAddPos);

			//interpolate scale
			float scaleDiff = NframeScale - frameScale;
			float scaleAdd = (scaleDiff / animFrames) * animCurrentFrame;
			radius = (frameScale + scaleAdd);
			radius2 = radius * radius;

			//interpolate colour
			float redDiff = NframeCol.x - frameCol.x;
			float greenDiff = NframeCol.y - frameCol.y;
			float blueDiff = NframeCol.z - frameCol.z;
			float redAddCol = (redDiff / animFrames) * animCurrentFrame;
			float greenAddCol = (greenDiff / animFrames) * animCurrentFrame;
			float blueAddCol = (blueDiff / animFrames) * animCurrentFrame;
			surfaceColor = Vec3f(frameCol.x + redAddCol, frameCol.y + greenAddCol, frameCol.z + blueAddCol);

		}
		else if (frameNum >= keyframes[keyframeCount - 1].timelineLoc * frameRateScale)//if the frame number is more than the earliest keyframe, then just set the info to the end keyframe's details.
		{
			float newXPos = keyframes[keyframeCount - 1].xPos;
			float newYPos = keyframes[keyframeCount - 1].yPos;
			float newZPos = keyframes[keyframeCount - 1].zPos;
			center = Vec3f(newXPos, newYPos, newZPos);

			radius = keyframes[keyframeCount - 1].scale;
			radius2 = radius * radius;

			surfaceColor = Vec3f(keyframes[keyframeCount - 1].kColor.x, keyframes[keyframeCount - 1].kColor.y, keyframes[keyframeCount - 1].kColor.z);
		}
		else if (frameNum < keyframes[0].timelineLoc * frameRateScale)//also do the opposite for the start keyframe, and set the info to this if the frame is earlier than the first keyframe
		{
			float newXPos = keyframes[0].xPos;
			float newYPos = keyframes[0].yPos;
			float newZPos = keyframes[0].zPos;
			center = Vec3f(newXPos, newYPos, newZPos);

			radius = keyframes[0].scale;
			radius2 = radius * radius;

			surfaceColor = Vec3f(keyframes[0].kColor.x, keyframes[0].kColor.y, keyframes[0].kColor.z);
		}

	}
	//auto endTime = std::chrono::system_clock::now() - startTime;
	//ms time = std::chrono::duration_cast<ms>(endTime);
	//cout << "Update Took: " << time.count() << " MS\n";
}
