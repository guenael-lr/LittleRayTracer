#include "Utils.h"
#include "CLittleRaytracer.h"



int main(int argc, char** argv)
{
	glm::ivec2 res = glm::ivec2(640, 480);
	if (argc > 2)
	{
		res.x = atoi(argv[1]);
		res.y = atoi(argv[2]);
	}


	LittleRaytracer lrt(res);

	lrt.run();

	return 0;
}