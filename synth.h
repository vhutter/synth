#ifndef SYNTH_H_INCLUDED
#define SYNTH_H_INCLUDED

#include <functional>
#include <portaudio.h>


class SynthStream final
{
public:

    SynthStream(unsigned sampleRate, const unsigned bufferSize, std::function<double(double)> generator);
    ~SynthStream();
    void play();
    void stop();

private:

    struct PaStreamCallbackData
    {
        std::function<double(double)> generator;
        double sampleTime = 0;
        const double sampleTimeDif;

        explicit PaStreamCallbackData(decltype(generator) g, decltype(sampleTimeDif) s)
            :generator(g), sampleTimeDif(s) {}

        static int callbackFunction(
            const void*                     inputBuffer,
            void*                           outputBuffer,
            unsigned long                   framesPerBuffer,
            const PaStreamCallbackTimeInfo* timeInfo,
            PaStreamCallbackFlags           statusFlags,
            void*                           userData);
    };

    PaStreamCallbackData callbackData;

    PaStreamParameters outputParameters;
    PaStream *stream;
    PaError err;
};

#endif // SYNTH_H_INCLUDED
