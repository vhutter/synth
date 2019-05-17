#include <AudioFile.h>

#include "test.h"

template<class Instrument_t, class Arr_t>
void test(
	Instrument_t& inst,
	const unsigned sampleRate,
	const double seconds,
	const Arr_t& keys
)
{
	static unsigned testId = 0;
	++testId;
	std::cout << "Running test " << testId << " ...\n";

	// Record 
	auto& gen = inst.getGenerator();

	auto save = SaveToFile("Test"s + std::to_string(testId), sampleRate, 2);

	gen.addAfterCallback(save);
	for (auto key : keys) gen[key].start(0.);
	save.start();
	double dt = 1. / double(sampleRate);
	double t = 0.;
	for (unsigned i = 0; i < sampleRate * seconds; ++i) {
		gen.getSample(t);
		t += dt;
	}
	save.stop();
	std::cout << "Test " << testId << " ended.\n";
}

void testGenerator()
{
	static KeyboardInstrument inst1(
		"Test",
		Sines1(),
		ADSREnvelope(.5, .1),
		generateNotes(2, 5),
		15
	);
	static KeyboardInstrument inst2(
		"Test",
		SinesTriangles(),
		ADSREnvelope(.5, .1),
		generateNotes(4, 5),
		15
	);
	static KeyboardInstrument inst3(
		"Test",
		SinesTriangles(),
		ADSREnvelope(),
		generateNotes(1, 6),
		15
	);
	auto maj79 = { 0, 4, 7, 10, 14 };
	auto dim7  = { 0, 3, 6, 9 };
	auto big   = { 0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42 };
	//test(inst1, 44100, 2., maj79);
	//test(inst2, 44100, 2., dim7);
	test(inst3, 96000, 5., big);
}