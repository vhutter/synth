#ifndef SYNTH_H_INCLUDED
#define SYNTH_H_INCLUDED

#include <functional>
#include <portaudio.h>
#include "generators.h"


class SynthStream final
{
public:
    typedef std::function<double(double)> CallbackFunction;
	typedef std::function<void(double)> InputCallback;

    SynthStream(
		unsigned sampleRate, 
		const unsigned bufferSize, 
		CallbackFunction generator1, 
		CallbackFunction generator2,
		InputCallback generator3);
    ~SynthStream();
    void play();
    void stop();

private:

    struct PaStreamCallbackData
    {
        CallbackFunction generator1, generator2;
		InputCallback inputGenerator;
        double sampleTime = 0;
        const double sampleTimeDif;

        explicit PaStreamCallbackData(CallbackFunction g1, CallbackFunction g2, InputCallback g3, decltype(sampleTimeDif) s)
            :generator1(g1), generator2(g2), inputGenerator(g3), sampleTimeDif(s) {}

        static int callbackFunction(
            const void*                     inputBuffer,
            void*                           outputBuffer,
            unsigned long                   framesPerBuffer,
            const PaStreamCallbackTimeInfo* timeInfo,
            PaStreamCallbackFlags           statusFlags,
            void*                           userData);
    };

    PaStreamCallbackData callbackData;

	PaStreamParameters outputParameters, inputParameters;
    PaStream *stream;
    PaError err;
	bool running{ false };
};

#endif // SYNTH_H_INCLUDED
