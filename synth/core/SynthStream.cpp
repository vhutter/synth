#include "SynthStream.h"

#include <algorithm>
#include <exception>
#include <iostream>
#include <ctime>

namespace {
    void ErrorCheck(const PaError& err)
    {
        if( err != paNoError )
            throw std::runtime_error(std::string("PortAudio exception: ")+Pa_GetErrorText(err));
    }
}

int SynthStream::PaStreamCallbackData::callbackFunction(
    const void*                     inputBuffer,
    void*                           outputBuffer,
    unsigned long                   framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags           statusFlags,
    void*                           userData)
{
    auto* data = static_cast<PaStreamCallbackData*>( userData );
    auto* out = static_cast<float*>( outputBuffer );
	auto* in = static_cast< const float* >(inputBuffer);

    for (unsigned i=0; i<framesPerBuffer; i++) {
		float input = *in++;
		data->inputGenerator(input);
        float sample1 = data->generator1(data->sampleTime);
		float sample2 = data->generator2(data->sampleTime);
        *out++ = sample1;
		*out++ = sample2;
        data->sampleTime += data->sampleTimeDif;
    }

    return 0;
}

SynthStream::SynthStream(
	unsigned sampleRate, 
	unsigned bufferSize, 
	CallbackFunction g1, 
	CallbackFunction g2,
	InputCallback g3)
    :callbackData(g1, g2, g3, 1./sampleRate)
{
    ErrorCheck(Pa_Initialize());
	inputParameters.device = Pa_GetDefaultInputDevice();
	if (inputParameters.device == paNoDevice) {
		throw std::runtime_error("PortAudio error: No default input device.\n");
	}
    outputParameters.device = Pa_GetDefaultOutputDevice();
    if (outputParameters.device == paNoDevice) {
        throw std::runtime_error("PortAudio error: No default output device.\n");
    }
    outputParameters.channelCount = 2;                     /* stereo output */
    outputParameters.sampleFormat = paFloat32;
	outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

	inputParameters.channelCount = 1;                     /* Mono input */
	inputParameters.sampleFormat = paFloat32;
	inputParameters.suggestedLatency = 0.01; Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
	inputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream( &stream,
                         &inputParameters,
                         &outputParameters,
                         sampleRate,
                         bufferSize,               /* Frames per buffer. */
                         paNoFlag,         /* Automatically clip samples out of [-1, 1] */
                         PaStreamCallbackData::callbackFunction,
                         &callbackData );
    ErrorCheck(err);
}

SynthStream::~SynthStream()
{
	if (running) {
		stop();
	}
    ErrorCheck(Pa_Terminate());
}

void SynthStream::play()
{
    ErrorCheck(Pa_StartStream( stream ));
	running = true;
}

void SynthStream::stop()
{
    ErrorCheck(Pa_CloseStream( stream ));
	running = false;
}
