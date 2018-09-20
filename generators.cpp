#include "generators.h"

#define _USE_MATH_DEFINES
#include <cmath>


namespace
{
    static const double TAU = M_PI*2;
}

namespace waves
{
    double sine(double time, double amp, double freq)
    {
        return amp*::sin(time*freq*TAU);
    }

    double square(double time, double amp, double freq)
    {
        const double period = 1/freq;
        if (fmod(time, period) < period/2)
            return amp;
        else
            return -amp;
    }

    double triangle(double time, double amp, double freq)
    {
        const double period = 1/freq;
        const double half_period = period/2;
        const double t = fmod(time, period);
        if (t < half_period)
            return -t*amp*2/half_period + amp;
        else
            return (t-half_period)*amp*2/half_period - amp;
    }

    double sawtooth(double time, double amp, double freq)
    {
        const double period = 1/freq;
        const double t = fmod(time, period);
        return -(t*amp*2/period - amp);
    }
}

ADSREnvelope::ADSREnvelope(double a, double d, double s, double r)
    :attack(a), decay(d), sustain(s), release(r) {}

ADSREnvelope::ADSREnvelope(const ADSREnvelope& rhs)
    :attack(rhs.attack), decay(rhs.decay), sustain(rhs.sustain), release(rhs.release) {}

void ADSREnvelope::start(double t)
{
    startAmp = getAmplitude(t);
    beginTime = t;
    isHeld = true;
    nonzero = true;
}

void ADSREnvelope::stop(double t)
{
    beginTime = t;
    isHeld = false;
}

double ADSREnvelope::getAmplitude(double t) const
{
    t = t - beginTime;
    if (isHeld) {
        if (t <= attack)
            return t*(1-startAmp)/attack + startAmp;
        else if (t < attack+decay)
            return 1 - (t-attack) * (1 - sustain) / decay;
        else
            return sustain;
    }
    else {
        if (t < release)
            return sustain - t * sustain/release;
        else {
            nonzero = false;
            return 0;
        }
    }

    return 0;
}
