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
        ADSREnvelope(double a=.005, double d=0.01, double s=1., double r=.01);
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
        mutable double currentAmp = 0;
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

class Note
{
    public:
        Note(double freq);
        double getFifth() const {return freq*3/2;}
        operator double() const {return freq;}

    private:
        double freq;
};

class Tone
{
    public:
        Note note;
        double intensity;
        double phase = 0;
        waves::wave_t waveform;

        Tone(
            const Note& note,
            double intensity,
            waves::wave_t waveform,
            std::initializer_list<std::function<void(double, Tone&)>> before = {},
            std::initializer_list<std::function<void(double, double&)>> after = {}
        )
            :note(note),
            intensity(intensity),
            waveform(waveform),
            beforeEffects(before.begin(), before.end()),
            afterEffects(after.begin(), after.end())
        {}
        double getSample(double t);
    private:

        const std::vector<std::function<void(double, Tone&)>> beforeEffects;
        const std::vector<std::function<void(double, double&)>> afterEffects;
};

class CompoundTone
{
    public:

        CompoundTone();
        CompoundTone(std::initializer_list<Tone>, const ADSREnvelope& env = ADSREnvelope());

        void addComponent(const Tone& desc);
        void normalize();
        void modifyMainPitch(double t, double dest);
        void shiftOctave(double time, double n);
        double getSample(double t);
        const Note& getMainNote() const {return mainNote;}

        ADSREnvelope envelope;
    private:
        const std::vector<Tone> initialComponents;
        std::vector<Tone> components;
        Note mainNote = 0;
};




extern const Note A  ;
extern const Note Ais;
extern const Note B  ;
extern const Note C  ;
extern const Note Cis;
extern const Note D  ;
extern const Note Dis;
extern const Note E  ;
extern const Note F  ;
extern const Note Fis;
extern const Note G  ;
extern const Note Gis;

#endif // GENERATORS_H_INCLUDED
