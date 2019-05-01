#include "test.h"
#include "../instruments/tones.h"

#include <thread>

int testMain(int argc, char** argv)
{
	//testGui();

	Instrument1 inst(
		44100,
		16,
		Sine13,
		generateNotes(2, 5),
		5
	);
	testGenerator(inst);

	std::this_thread::sleep_for(std::chrono::seconds(30));

	return 0;
}