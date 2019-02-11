#include "generators.h"

namespace waves
{
    namespace
    {
        static const double TAU = M_PI*2;
    }

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

Note::Note(double f)
    :freq(f)
{

}

const Note C  (523.25);
const Note Cis(554.37);
const Note D  (587.33);
const Note E  (659.25);
const Note Dis(622.25);
const Note F  (698.46);
const Note Fis(739.99);
const Note G  (783.99);
const Note Gis(830.61);
const Note A  (880.00);
const Note Ais(932.33);
const Note B  (987.77);

double Tone::getSample(double t)
{
    Tone input = *this;
    for(auto& effect: beforeEffects)
        effect(t, input);
    double result = waveform(t, input.intensity, input.note, input.phase);
    for(auto& effect: afterEffects)
        effect(t, result);
    return result;
}

CompoundTone::CompoundTone(){}

CompoundTone::CompoundTone(std::initializer_list<Tone> comps, const ADSREnvelope& env)
    : envelope(env),
      initialComponents(comps.begin(), comps.end()),
      components(initialComponents),
      mainNote(initialComponents.front().note)
{
    normalize();
}

void CompoundTone::addComponent(const Tone& td)
{
    components.push_back(td);
    normalize();
}

void CompoundTone::normalize()
{
    double max = std::max_element(components.begin(), components.end(),
        [](const auto& d1, const auto& d2) {
            return d1.intensity < d2.intensity;
        }
    )->intensity;
    for(auto& component: components) component.intensity /= max;
}

void CompoundTone::modifyMainPitch(double t, double dest)
{
    if(mainNote == 0) throw "division by zero";
    const double changeRate = dest / mainNote;
    for(unsigned i=0; i<components.size(); ++i) {
        auto& component = components[i];
        double f1 = component.note;
        double f2 = initialComponents[i].note * changeRate;
        double p = (t+component.phase) * f1 / f2 - t;
        component.phase = p;
        component.note = f2;
    }
}

double CompoundTone::getSample(double t)
{
    double result = 0.;
    double intensitySum = 0.;
    for(auto& c: components) {
        result += c.getSample(t);
        intensitySum += c.intensity;
    }
    return result / intensitySum;
}
