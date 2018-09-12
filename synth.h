#ifndef SYNTH_H_INCLUDED
#define SYNTH_H_INCLUDED

#include <SFML/Audio.hpp>
#include <functional>


class SynthStream : public sf::SoundStream
{
public:

    SynthStream(unsigned sampleRate, const unsigned bufferSize, std::function<double(double&)> generator);
    void resetTime() {sampleTime = 0;}

private:

    virtual bool onGetData(Chunk& data) override;
    virtual void onSeek(sf::Time timeOffset) override;

    double sampleTime = 0;
    const unsigned sampleRate, bufferSize;
    const double sampleTimeDif;
    std::vector<sf::Int16> buffer;
    std::function<double(double&)> generator;

};

#endif // SYNTH_H_INCLUDED
