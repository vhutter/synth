#ifndef GENERATORS_H_INCLUDED
#define GENERATORS_H_INCLUDED
 

#define _USE_MATH_DEFINES
#include <cmath>
#include <chrono>
#include <functional>
#include <vector>
#include <algorithm>
#include <atomic>
#include <mutex>
#include <type_traits>
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
		ADSREnvelope(const ADSREnvelope& rhs) = default;
        ~ADSREnvelope() = default;

        void start(double t) const;
        void stop(double t) const;
        double getAmplitude(double t) const;
        bool isNonZero() const {return nonzero;}

    private:
        const double attack, decay, sustain, release;

        mutable bool nonzero = false, isHeld = false;
        mutable double currentAmp = 0, beginTime = 0, startAmp = 0;
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


class Tone;

template<class T>
class SampleGenerator
{
public:
	typedef std::function<void(double, double&)> after_t;

	SampleGenerator(
		std::function<void(double, T&)> before = {},
		after_t after = {}
	)
		:beforeEffect(before),
		afterEffect(after)
	{}
	double getSample(double t) const
	{
		T input = *const_cast<T*>(static_cast<const T*>(this));
 		if (beforeEffect) {
			beforeEffect(t, input);
		}
		double result = input.getSampleImpl(t);
		if (afterEffect) {
			afterEffect(t, result);
		}
		return result;
	}
	constexpr double getIntensity() const
	{
		if constexpr (std::is_same_v<T, Tone>) {
			return static_cast<const T*>(this)->intensity;
		}
		else {
			return 1;
		}
	}
	
	const Note& getMainNote() const
	{
		return static_cast<const T*>(this)->getMainNoteImpl();
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
		);

		void modifyMainPitch(double t, double dest);

    private:
        double getSampleImpl(double t) const;
		const Note& getMainNoteImpl() const;
};

template<class T>
class CompoundGenerator: public SampleGenerator<CompoundGenerator<T>>
{
	friend class SampleGenerator<CompoundGenerator<T>>;
    public:
		typedef std::function<void(double, CompoundGenerator&)> before_t;

        CompoundGenerator(
			const std::vector<T>&,
			before_t before = {},
			typename SampleGenerator<CompoundGenerator<T>>::after_t after = {}
		);

        void addComponent(const T& desc);
        void modifyMainPitch(double t, double dest);
		const T& operator[](std::size_t idx) const;


    protected:
        double getSampleImpl(double t) const;
		const Note& getMainNoteImpl() const;

        const std::vector<T> initialComponents;
        std::vector<T> components;
};

template<class T>
CompoundGenerator<T>::CompoundGenerator(
	const std::vector<T>& comps,
	before_t before,
	typename SampleGenerator<CompoundGenerator<T>>::after_t after)
	: SampleGenerator<CompoundGenerator>(before, after),
	initialComponents(comps.begin(), comps.end()),
	components(initialComponents)
{}

template<class T>
void CompoundGenerator<T>::addComponent(const T& td)
{
	components.push_back(td);
}

template<class T>
void CompoundGenerator<T>::modifyMainPitch(double t, double dest)
{
	const double changeRate = dest / initialComponents.front().getMainNote();
	for (unsigned i = 0; i < components.size(); ++i) {
		auto& component = components[i];
		double f1 = component.getMainNote();
		double f2 = initialComponents[i].getMainNote() * changeRate;
		component.modifyMainPitch(t, f2);
	}
}

template<class T>
double CompoundGenerator<T>::getSampleImpl(double t) const
{
	double result = 0.;
	double intensitySum = 0.;
	for (auto& c : components) {
		const double& sample = c.getSample(t);
		if (sample != 0) {
			result += sample;
			intensitySum += c.getIntensity();
		}
	}
	return result / intensitySum;
}

template<class T>
const T& CompoundGenerator<T>::operator[](std::size_t idx) const
{
	return components[idx];
}

template<class T>
const Note& CompoundGenerator<T>::getMainNoteImpl() const
{
	return initialComponents.front().getMainNote();
}

template<class T>
class DynamicCompoundGenerator: public CompoundGenerator<T>
{
	public:
		DynamicCompoundGenerator(
			const CompoundGenerator<T>& cTone,
			ADSREnvelope env = {}
		);
		void start(double t) const;
		void stop(double t) const;
		//using CompoundGenerator<T>::modifyMainPitch;
		//using CompoundGenerator<T>::getMainNote;
		//using CompoundGenerator<T>::getIntensity;
		//using CompoundGenerator<T>::operator[];
		double getSample(double t) const;

	private:
		ADSREnvelope envelope;
};

template<class T>
DynamicCompoundGenerator<T>::DynamicCompoundGenerator(
	const CompoundGenerator<T>& cTone,
	ADSREnvelope env
) :
	CompoundGenerator<T>(cTone),
	envelope(env)
{}

template<class T>
void DynamicCompoundGenerator<T>::start(double t) const
{
	envelope.start(t);
}

template<class T>
void DynamicCompoundGenerator<T>::stop(double t) const
{
	envelope.stop(t);
}

template<class T>
double DynamicCompoundGenerator<T>::getSample(double t) const
{
	if (envelope.isNonZero()) {
		return CompoundGenerator<T>::getSample(t) * envelope.getAmplitude(t);
	}
	else {
		return 0;
	}
}

template<std::size_t maxTones>
class DynamicToneSum : protected CompoundGenerator<DynamicCompoundGenerator<Tone>>
{
	using Base_t = CompoundGenerator<DynamicCompoundGenerator<Tone>>;

	public:
		DynamicToneSum(const Base_t& tones)
			:Base_t(tones)
		{}
		DynamicToneSum(const DynamicToneSum& that)
			:DynamicToneSum(Base_t(that.initialComponents))
		{}

		void lock() const { mtx.lock(); }
		void unlock() const { mtx.unlock(); }
		double time() const { return lastTime.load(); }

		using Base_t::getMainNote;
		using Base_t::modifyMainPitch;
		using Base_t::operator[];

		double getSample(double t) const
		{
			// The application of effects looks ugly
			lastTime.store(t);
			if (beforeEffect) beforeEffect(t, *const_cast<DynamicToneSum*>(this));
			double result{ 0. };
			std::size_t count{ 0 };
			for (auto& c : components) {
				const double& sample = c.getSample(t);
				if (sample != 0) {
					if (++count > maxTones) break;
					result += sample;
				}
			}
			result /= maxTones;
			if (afterEffect) afterEffect(t, result);
			return result;
		}

	private:
		mutable std::atomic<double> lastTime{ 0 };
		mutable std::mutex mtx;
};


struct TimbreModel
{
	struct ToneSkeleton {
		double relativeFreq;
		double intensity;
		waves::wave_t waveform;
	};

	TimbreModel(
		std::vector<ToneSkeleton> components,
		Tone::before_t         beforeTone = {},
		Tone::after_t          afterTone = {},
		CompoundGenerator<Tone>::before_t before = {},
		CompoundGenerator<Tone>::after_t  after = {}
	);
	CompoundGenerator<Tone> operator()(const double& baseFreq);

	std::vector<ToneSkeleton> components;
	Tone::before_t         beforeTone;
	Tone::after_t          afterTone;
	CompoundGenerator<Tone>::before_t before;
	CompoundGenerator<Tone>::after_t  after;
};

#endif // GENERATORS_H_INCLUDED
