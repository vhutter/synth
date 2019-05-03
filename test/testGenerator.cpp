#include "test.h"

void testGenerator()
{
	static Instrument1 inst(
		"Sdfasd",
		Sines1(),
		ADSREnvelope(),
		generateNotes(2, 5),
		15
	);
	auto& gen = inst.getGenerator();


	std::this_thread::sleep_for(std::chrono::seconds(30));
}