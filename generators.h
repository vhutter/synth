#ifndef GENERATORS_H_INCLUDED
#define GENERATORS_H_INCLUDED

#include <chrono>
#include <functional>

namespace waves
{
    typedef std::function<double(double,double,double)> wave_t;

    double sine(double time, double amp, double freq);
    double square(double time, double amp, double freq);
    double triangle(double time, double amp, double freq);
    double sawtooth(double time, double amp, double freq);
}

class ADSREnvelope
{
public:
    ADSREnvelope(double a=.01, double d=0.01, double s=1, double r=.01);
    ADSREnvelope(const ADSREnvelope& rhs);
    ~ADSREnvelope() = default;

    void start(double t);
    void stop(double t);
    double getAmplitude(double t) const;
    bool isNonZero() const {return nonzero;}

private:
    const double attack, decay, sustain, release;
    double beginTime = 0, startAmp = 0;
    bool isHeld = false;
    mutable bool nonzero = false;

};

#endif // GENERATORS_H_INCLUDED
