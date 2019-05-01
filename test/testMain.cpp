#include "test.h"
#include "../instruments/tones.h"

#include <thread>

int testMain(int argc, char** argv)
{
	//testGui();
	testGenerator();

	std::this_thread::sleep_for(std::chrono::seconds(30));

	return 0;
}