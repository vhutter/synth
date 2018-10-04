#ifndef GENERATORS_H_INCLUDED
#define GENERATORS_H_INCLUDED


#define _USE_MATH_DEFINES
#include <cmath>
#include <chrono>
#include <functional>
#include <SFML/System.hpp>

namespace waves
{
    typedef std::function<double(double,double,double,double)> wave_t;

    double sine(double time, double amp, double freq, double phase=0);
    double square(double time, double amp, double freq, double phase);
    double triangle(double time, double amp, double freq, double phase);
    double sawtooth(double time, double amp, double freq, double phase);
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

class ContinuousFunction
{
public:
    ContinuousFunction(double initConst=0);
    void setValueLinear(double newVal, double begin, double duration);
    double getValue(double t);

private:
    double value;
    double startTime, endTime, startValue, endValue;
    double m;

    sf::Vector2f reach;
};

#endif // GENERATORS_H_INCLUDED
