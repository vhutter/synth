#ifndef GENERATORS_H_INCLUDED
#define GENERATORS_H_INCLUDED

#include <chrono>
#include <iostream>

namespace waves
{
    double sine(double time, double amp, double freq);
    double square(double time, double amp, double freq);
}

class ADSREnvelope
{
public:
    ADSREnvelope(double a, double d, double s, double r, double srate);

    void start(double t);
    void stop(double t);
    double getAmplitude(double t) const;

    bool isNonZero() const {return nonzero;}

private:
    const double attack, decay, sustain, release;
    double beginTime;
    bool isHeld = false;
    mutable bool nonzero = false;

};

#endif // GENERATORS_H_INCLUDED
