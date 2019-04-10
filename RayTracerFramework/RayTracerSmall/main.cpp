
#include "RayTracer.h"


	//[comment]
	// In the main function, we will create the scene which is composed of 5 spheres
	// and 1 light (which is also a sphere). Then, once the scene description is complete
	// we render that scene, by calling the render() function.
	//[/comment]
	int main(int argc, char **argv)
	{
		RayTracer* RT = new RayTracer();
		RT->ReadJson("ExportSpheres.JSON");
		// This sample only allows one choice per program execution. Feel free to improve upon this
		srand(13);
		//RT.BasicRender();
		//RT.SimpleShrinking();
		//RT.SmoothScaling();
		RT->JSONAnimate();

		return 0;
	}