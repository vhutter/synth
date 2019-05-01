#include "test.h"

void testGenerator()
{
	static Instrument1 inst(
		44100,
		16,
		Sine13(),
		generateNotes(2, 5),
		15
	);
	auto& gen = inst.getGenerator();

	gen[20].start(0.);
	gen[21].start(0.);
	gen[22].start(0.);
	gen[23].start(0.);
	gen[24].start(0.);
	gen[25].start(0.);
	gen[26].start(0.);
	gen[10].start(0.);
	gen[11].start(0.);
	gen[12].start(0.);
	gen[13].start(0.);
	gen[14].start(0.);
	gen[15].start(0.);
	gen[16].start(0.);
	inst.play();


	std::this_thread::sleep_for(std::chrono::seconds(30));
}