#include "synth.h"

#include <iostream>

SynthStream::SynthStream(unsigned sr, unsigned bs, std::function<double(double&)> gen)
    :sampleRate(sr),
     bufferSize(bs),
     sampleTimeDif(double(1)/sr),
     buffer(bufferSize),
     generator(gen)
{
    initialize(1, sampleRate);
}

bool SynthStream::onGetData(Chunk& data)
{
    for (unsigned i=0; i<bufferSize; i++) {
		buffer[i] = generator(sampleTime);
        sampleTime += sampleTimeDif;
    }
    data.samples = buffer.data();
    data.sampleCount = bufferSize;

    return true;
}

void SynthStream::onSeek(sf::Time timeOffset)
{
    return;
}
