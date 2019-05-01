#ifndef SYNTH_TEST_DEFINED
#define SYNTH_TEST_DEFINED

#include "../instruments/Instrument.h"

int testMain(int argc, char** argv);
void testGui();
void testGenerator(Instrument<DynamicToneSum>& instrument);

#endif