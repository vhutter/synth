#include "generators.h"


namespace
{
    static const double TAU = M_PI*2;
}

namespace waves
{
    double sine(double time, double amp, double freq, double phase)
    {
        return amp*::sin((time+phase)*freq*TAU);
    }

    double square(double time, double amp, double freq, double phase)
    {
        const double period = 1/freq;
        if (fmod(time+phase, period) < period/2)
            return amp;
        else
            return -amp;
    }

    double triangle(double time, double amp, double freq, double phase)
    {
        const double period = 1/freq;
        const double half_period = period/2;
        const double t = fmod(time+phase, period);
        if (t < half_period)
            return -t*amp*2/half_period + amp;
        else
            return (t-half_period)*amp*2/half_period - amp;
    }

    double sawtooth(double time, double amp, double freq, double phase)
    {
        const double period = 1/freq;
        const double t = fmod(time+phase, period);
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
            currentAmp = t*(1-startAmp)/attack + startAmp;
        else if (t < attack+decay)
            currentAmp = 1 - (t-attack) * (1 - sustain) / decay;
        else
            currentAmp = sustain;
    }
    else {
        if (t < release)
            // we saved currentAmp for this case:
            // the key may have been released before reaching the sustain level,
            // in which case we have to calculate the appropiate decay level by
            // considering the last available amplitude
            return currentAmp - t * currentAmp/release;
        else {
            nonzero = false;
            currentAmp = 0;
        }
    }
    return currentAmp;
}


ContinuousFunction::ContinuousFunction(double initConst)
    : value(initConst), m(0), reach(0,0) {}

double ContinuousFunction::getValue(double t)
{
    if (m != 0 && t >= startTime) {
        value = startValue + m * (t-startTime);

        if (t >= endTime)
        {
            value = endValue;
            m = 0;
        }
    }
    return value;
}

void ContinuousFunction::setValueLinear(double newVal, double btime, double duration)
{
    startValue = value;
    endValue = newVal;
    startTime = btime;
    endTime = btime + duration;
    m = (endValue - startValue) / (endTime - startTime);
    reach = sf::Vector2f(endTime, endValue);
}
