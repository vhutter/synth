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

    for (unsigned i=0; i<framesPerBuffer; i++) {
        float sample1 = std::clamp(data->generator1(data->sampleTime), -1., 1.);
        float sample2 = sample1;
        *out++ = sample1;
		*out++ = sample2;
        data->sampleTime += data->sampleTimeDif;
    }

    return 0;
}

SynthStream::SynthStream(unsigned sampleRate, unsigned bufferSize, CallbackFunction g1, CallbackFunction g2)
    :callbackData(g1, g2, double(1)/sampleRate)
{
    ErrorCheck(Pa_Initialize());
    outputParameters.device = Pa_GetDefaultOutputDevice();
    if (outputParameters.device == paNoDevice) {
        throw std::runtime_error("PortAudio error: No default output device.\n");
    }
    outputParameters.channelCount = 2;                     /* stereo output */
    outputParameters.sampleFormat = paFloat32;             /* 32 bit floating point output */
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream( &stream,
                         NULL,              /* No input. */
                         &outputParameters, /* As above. */
                         sampleRate,
                         bufferSize,               /* Frames per buffer. */
                         paClipOff,         /* No out of range samples expected. */
                         PaStreamCallbackData::callbackFunction,
                         &callbackData );
    ErrorCheck(Pa_Initialize());
}

SynthStream::~SynthStream()
{
	if (running) {
		stop();
	}
    Pa_Terminate();
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