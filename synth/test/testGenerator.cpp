#include <AudioFile.h>

#include "test.h"

void testGenerator()
{
	static KeyboardInstrument inst(
		"Sdfasd",
		Sines1(),
		ADSREnvelope(),
		generateNotes(2, 5),
		15
	);
	auto& gen = inst.getGenerator();
	
	const unsigned sampleRate = 44100;
	auto save = SaveToFile("Test", 44100, 2);

	gen.addAfterCallback(save);
	gen[5].start(0.);
	save.start();
	double dt = 1. / double(sampleRate);
	double t = 0.;
	for (unsigned i = 0; i < sampleRate * 2; ++i) {
		gen.getSample(t);
		t += dt;
	}
	save.stop();
	



	//AudioFile<double> audioFile;
	//AudioFile<double>::AudioBuffer buffer;
	//
	//// 2. Set to (e.g.) two channels
	//buffer.resize(2);
	//
	//// 3. Set number of samples per channel
	//buffer[0].resize(100000);
	//buffer[1].resize(100000);
	//
	//
	//int numChannels = 1;
	//int numSamplesPerChannel = 100000;
	//float sampleRate = 44100.f;
	//float frequency = 440.f;
	//
	//for (int i = 0; i < numSamplesPerChannel; i++)
	//{
	//	float sample = sinf(2. * ((float)i / sampleRate) * frequency);
	//
	//	for (int channel = 0; channel < numChannels; channel++)
	//		buffer[channel][i] = sample * 0.5;
	//}
	//
	//// 5. Put into the AudioFile object
	//bool ok = audioFile.setAudioBuffer(buffer);
	//
	//// Set the number of samples per channel
	//audioFile.setNumSamplesPerChannel(numSamplesPerChannel);
	//
	//// Set the number of channels
	//audioFile.setNumChannels(1); 
	//audioFile.setBitDepth(24);
	//audioFile.setSampleRate(44100);
	//
	//audioFile.setBitDepth(24);
	//audioFile.setSampleRate(44100);
	//
	//// Wave file (explicit)
	//audioFile.save("test.wav", AudioFileFormat::Wave);
}