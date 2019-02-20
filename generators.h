#ifndef GENERATORS_H_INCLUDED
#define GENERATORS_H_INCLUDED
 

#define _USE_MATH_DEFINES
#include <cmath>
#include <chrono>
#include <functional>
#include <vector>
#include <algorithm>
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

        sf::Vector2<double> reach;
};

class Note
{
    public:
        Note(double freq);
        double getFifth() const {return freq*3/2;}
        operator double() const {return freq;}


		static const Note A;
		static const Note Ais;
		static const Note B;
		static const Note C;
		static const Note Cis;
		static const Note D;
		static const Note Dis;
		static const Note E;
		static const Note F;
		static const Note Fis;
		static const Note G;
		static const Note Gis;
		
    private:
        double freq;
};




template<class T>
class SampleGenerator
{
public:
	using after_t = std::function<void(double, double&)>;

	SampleGenerator(
		std::function<void(double, T&)> before = {},
		after_t after = {}
	)
		:beforeEffect(before),
		afterEffect(after)
	{}
	double getSample(double t) const
	{
		T input = *static_cast<const T*>(this);
 		if (beforeEffect) {
			beforeEffect(t, input);
		}
		double result = input.getSampleImpl(t);
		if (afterEffect) {
			afterEffect(t, result);
		}
		return result;
	}
protected:
	std::function<void(double, T&)> beforeEffect;
	after_t afterEffect;
};

class Tone : public SampleGenerator<Tone>
{
	friend class SampleGenerator<Tone>;
    public:
		typedef std::function<void(double, Tone&)> before_t;

        Note note;
        double intensity;
        double phase = 0;
        waves::wave_t waveform;

        Tone(
            const Note& note,
            double intensity,
            waves::wave_t waveform,
			before_t before = {},
			after_t  after = {}
        )
            :SampleGenerator<Tone>(before, after),
			note(note),
            intensity(intensity),
            waveform(waveform)
		{
		}

    private:
        double getSampleImpl(double t) const;
};

class CompoundTone: public SampleGenerator<CompoundTone>
{
	friend class SampleGenerator<CompoundTone>;
    public:
		typedef std::function<void(double, CompoundTone&)> before_t;

        CompoundTone();
        CompoundTone(
			const std::vector<Tone>&,
			const ADSREnvelope& env = ADSREnvelope(),
			before_t before = {},
			after_t  after = {}
		);

        void addComponent(const Tone& desc);
        void normalize();
        void modifyMainPitch(double t, double dest);
        const Note& getMainNote() const {return mainNote;}

        ADSREnvelope envelope;
    private:
        double getSampleImpl(double t) const;

        const std::vector<Tone> initialComponents;
        std::vector<Tone> components;
        Note mainNote = 0;
};

class CompoundToneModel
{
	public:

		struct ToneSkeleton {
			double relativeFreq;
			double intensity;
			waves::wave_t waveform;
		};

		CompoundToneModel(
			std::initializer_list<ToneSkeleton> components,
			const ADSREnvelope&    env,
			Tone::before_t         beforeTone = {},
			Tone::after_t          afterTone = {},
			CompoundTone::before_t before = {},
			CompoundTone::after_t  after = {}
		)
			:components(components),
			beforeTone(beforeTone),
			afterTone(afterTone),
			before(before),
			after(after)
		{}
		CompoundTone operator()(const double& baseFreq);

		std::vector<ToneSkeleton> components;
		ADSREnvelope envelope;
		Tone::before_t         beforeTone;
		Tone::after_t          afterTone;
		CompoundTone::before_t before;
		CompoundTone::after_t  after;
};

#endif // GENERATORS_H_INCLUDED
