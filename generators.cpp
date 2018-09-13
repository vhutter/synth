#include "generators.h"

#define _USE_MATH_DEFINES
#include <cmath>

namespace waves
{
    double sine(double time, double amp, double freq)
    {
        return amp*::sin(time*freq*M_PI*2);
    }

    double square(double time, double amp, double freq)
    {
        return sine(time, amp, freq) > 0. ? 1. : 0.;
    }

}

ADSREnvelope::ADSREnvelope(double a, double d, double s, double r, double srate)
    :attack(a), decay(d), sustain(s), release(r) {}

void ADSREnvelope::start(double t)
{
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
            return 1 * t/attack;
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
