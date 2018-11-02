#include "util.h"

#include <array>
#include <cmath>
#include <algorithm>

namespace util
{

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

void CompoundTone::shiftOctave(double t, double n) {
    octave *= std::pow(2, n);
}

double CompoundTone::getSample(double t)
{
    double result = 0.;
    double intensitySum = 0.;
    for(auto& c: components) {
        result +=  c.waveform(t, c.intensity, c.note * octave, c.phase);
        intensitySum += c.intensity;
    }
    return result / intensitySum;
}

}
