#ifndef GENERATORS_H_INCLUDED
#define GENERATORS_H_INCLUDED
 

#define _USE_MATH_DEFINES
#include <cmath>
#include <functional>
#include <vector>
#include <algorithm>
#include <numeric>
#include <atomic>
#include <limits>
#include <mutex>
#include <array>
#include <optional>
#include <SFML/System.hpp>

#include "../gui/SynthKeyboard.h"

namespace waves
{
    using wave_t = std::function<double(double,double,double,double)>;

    double sine(double time, double amp, double freq, double phase=0);
    double square(double time, double amp, double freq, double phase);
    double triangle(double time, double amp, double freq, double phase);
    double sawtooth(double time, double amp, double freq, double phase);
}

class ADSREnvelope
{
	static constexpr double inf = std::numeric_limits<double>::infinity();
    public:

        ADSREnvelope(double a=.01, double d=0.01, double s=1., double r=.01, double st = inf);
		ADSREnvelope(const ADSREnvelope& rhs) = default;
        ~ADSREnvelope() = default;

        void start(double t);
        void stop(double t);
        double getAmplitude(double t) const;
        bool isNonZero() const {return nonzero;}

    private:
		void calculateTimePoints();

		mutable double attackTime, decayDur, sustainDur, releaseDur, sustainLevel;
		mutable double decayTime, sustainTime, releaseTime;

        mutable bool nonzero = false, isHeld = false;
        mutable double currentAmp = 0, beginTime = 0, startAmp = 0, lastStopTime=0;
};

class ContinuousFunction
{
    public:
        ContinuousFunction(double initConst=0);
        void setValueLinear(double newVal, double begin, double duration);
        double getValue(double t);

    private:
        double value;
		double startTime{ 0. }, endTime{ 0. }, startValue{ 0. }, endValue{ 0. };
        double m;
};

template<class T>
class SaveInitialValue
{
public:
	SaveInitialValue(const T& val) : value(val), initial(val) {}
	const T& getInitial() const { return initial; }

	operator const T&() const { return value; }
	SaveInitialValue<T>& operator=(const SaveInitialValue<T>& other)
	{
		value = other.value;
		return *this;
	}
	SaveInitialValue<T>& operator=(const T& other)
	{
		value = other;
		return *this;
	}

	T value;
private:
	const T initial;
};

class Note : public SaveInitialValue<double>
{
    public:
		using SaveInitialValue<double>::SaveInitialValue;
		using SaveInitialValue<double>::operator=;

		static const Note& A()  ;
		static const Note& Ais();
		static const Note& B()  ;
		static const Note& C()  ;
		static const Note& Cis();
		static const Note& D()  ;
		static const Note& Dis();
		static const Note& E()  ;
		static const Note& F()  ;
		static const Note& Fis();
		static const Note& G()  ;
		static const Note& Gis();

		static const std::array<Note, 12> baseNotes();
};


class WaveGenerator;

template<class T>
class SampleGenerator
{
public:
	using after_t = std::function<void(double, double&)>;

	SampleGenerator(
		std::function<void(double, T&)> before = {},
		after_t after = {}
	)
		:beforeSample(before),
		afterSample(after)
	{}
	double getSample(double t) const
	{
		T& input = *const_cast<T*>(static_cast<const T*>(this));
 		if (beforeSample) {
			beforeSample(t, input);
		}
		double result = input.getSampleImpl(t);
		if (afterSample) {
			afterSample(t, result);
		}
		return result;
	}
	constexpr double getIntensity() const
	{
		if constexpr (std::is_same_v<T, WaveGenerator>) {
			return static_cast<const T*>(this)->intensity;
		}
		else {
			return 1;
		}
	}
	void modifyMainPitch(double t, double f2)
	{
		static_cast<T*>(this)->modifyMainPitchImpl(t, f2);
	}
	
	const double getMainFreq() const
	{
		return static_cast<const T*>(this)->getMainFreqImpl();
	}
protected:
	std::function<void(double, T&)> beforeSample;
	after_t afterSample;
};

class SumGenerator : public SampleGenerator<SumGenerator>
{
	friend class SampleGenerator<WaveGenerator>;
	using callback_t = std::function<double(double)>;

public:
	using before_t = std::function<void(double, SumGenerator&)>;

	template<class T>
	SumGenerator(
		before_t before,
		after_t after,
		T&& instruments
	)
		:SampleGenerator(before, after),
		callback{ [instruments = std::move(instruments)] (double t) mutable {
			return std::apply([t](auto& ... args) {
				return (args.getGenerator().getSample(t) + ...);
				}, instruments);
			} 
		}
	{
	}

	double getSampleImpl(double t) const { return callback(t); }
	void modifyMainPitchImpl(double t, double f2) { return; }
	double getMainFreq() const { return 1; }


private:
	callback_t callback;
};

class WaveGenerator : public SampleGenerator<WaveGenerator>
{
		friend class SampleGenerator<WaveGenerator>;

    public:
		using before_t = std::function<void(double, WaveGenerator&)>;

        Note freq;
        SaveInitialValue<double> intensity;
        double phase = 0;
        waves::wave_t waveform;

        WaveGenerator(
            const double& note,
            double intensity,
            waves::wave_t waveform,
			before_t before = {},
			after_t  after = {}
		);


    private:
		void modifyMainPitchImpl(double t, double dest);
        double getSampleImpl(double t) const;
		const double getMainFreqImpl() const;
};

template<class T>
class Composite : public SampleGenerator<Composite<T>>
{
	friend class SampleGenerator<Composite<T>>;
    public:
		using before_t = std::function<void(double, Composite<T>&)>;

		Composite(
			const std::vector<T>&,
			before_t before = {},
			typename SampleGenerator<Composite<T>>::after_t after = {}
		);

		const T& operator[](std::size_t idx) const;

    protected:
        void modifyMainPitchImpl(double t, double dest);
        double getSampleImpl(double t) const;
		const double getMainFreqImpl() const;

        const std::vector<T> initialComponents;
        std::vector<T> components;
		double intensitySum;
};

template<class T>
Composite<T>::Composite(
	const std::vector<T>& comps,
	before_t before,
	typename SampleGenerator<Composite<T>>::after_t after)
	: SampleGenerator<Composite>(before, after),
	initialComponents(comps.begin(), comps.end()),
	components(initialComponents),
	intensitySum(std::accumulate(
		components.begin(),
		components.end(),
		0.,
		[](double acc, const auto& component) {
			return component.getIntensity() + acc;
		}
	))
{
}

template<class T>
void Composite<T>::modifyMainPitchImpl(double t, double dest)
{
	const double changeRate = dest / initialComponents.front().getMainFreq();
	for (unsigned i = 0; i < components.size(); ++i) {
		auto& component = components[i];
		double f1 = component.getMainFreq();
		double f2 = initialComponents[i].getMainFreq() * changeRate;
		component.modifyMainPitch(t, f2);
	}
}

template<class T>
double Composite<T>::getSampleImpl(double t) const
{
	double result = 0.;
	for (auto& c : components) {
		const double& sample = c.getSample(t);
		if (sample != 0) {
			result += sample;
		}
	}
	return result/intensitySum;
}

template<class T>
const T& Composite<T>::operator[](std::size_t idx) const
{
	return components[idx];
}

template<class T>
const double Composite<T>::getMainFreqImpl() const
{
	return initialComponents.front().getMainFreq();
}

template<class BaseGenerator>
class Dynamic: public BaseGenerator
{
	public:
		Dynamic(
			const BaseGenerator& baseGen,
			ADSREnvelope env = {}
		);
		void start(double t);
		void stop(double t);
		std::optional<double> getSample(double t) const;

	private:
		ADSREnvelope envelope;
};

template<class T>
Dynamic<T>::Dynamic(
	const T& baseGen,
	ADSREnvelope env
) :
	T(baseGen),
	envelope(env)
{}

template<class T>
void Dynamic<T>::start(double t)
{
	envelope.start(t);
}

template<class T>
void Dynamic<T>::stop(double t)
{
	envelope.stop(t);
}

template<class T>
std::optional<double> Dynamic<T>::getSample(double t) const
{
	if (envelope.isNonZero()) {
		return T::getSample(t) * envelope.getAmplitude(t);
	}
	else {
		return std::nullopt;
	}
}

struct TimbreModel
{
	struct ToneSkeleton {
		double relativeFreq;
		double intensity;
		waves::wave_t waveform;
	};

	TimbreModel(
		std::vector<ToneSkeleton> components,
		WaveGenerator::before_t         beforeTone = {},
		WaveGenerator::after_t          afterTone = {},
		Composite<WaveGenerator>::before_t before = {},
		Composite<WaveGenerator>::after_t  after = {}
	);
	Composite<WaveGenerator> operator()(const double& baseFreq) const;
	Dynamic<Composite<WaveGenerator>> operator()(const double& baseFreq, const ADSREnvelope& env) const;

	std::vector<ToneSkeleton> components;
	WaveGenerator::before_t         beforeTone;
	WaveGenerator::after_t          afterTone;
	Composite<WaveGenerator>::before_t before;
	Composite<WaveGenerator>::after_t  after;
};

class DynamicToneSum : public Composite<Dynamic<Composite<WaveGenerator>>>
{
	using base_t = Composite<Dynamic<Composite<WaveGenerator>>>;

	public:
		friend class std::lock_guard<DynamicToneSum>;
		using before_t = std::function<void(double, DynamicToneSum&)>;

		DynamicToneSum(
			const TimbreModel& timbreModel,
			const ADSREnvelope& env,
			const std::vector<Note>& notes, 
			unsigned maxTones
		);
		DynamicToneSum(const DynamicToneSum& that);

		double time() const;
		double getSample(double t) const;
		unsigned getMaxTones() const;
		std::vector<Note> getNotes() const;
		unsigned getNotesCount() const;
		const TimbreModel& getTimbreModel() const;

		unsigned addAfterCallback(after_t callback);
		void removeAfterCallback(unsigned id);
		unsigned addBeforeCallback(before_t callback);
		void removeBeforeCallback(unsigned id);

		void onKeyEvent(unsigned key, SynthKey::State keyState);

		void lock() const;
		void unlock() const;

	private:

		template<class T>
		struct give_id
		{
			unsigned id;
			T value;
		};

		template<class param_t>
		using callback_t = std::function<void(double, param_t&)>;

		template<class param_t> // what kind of callback do we add
		unsigned addCallback(
			callback_t<param_t>& raw_callback, // on what object did we call the callbacks so far
			std::vector<give_id<callback_t<param_t>>>& arr, // all callbacks so far
			callback_t<param_t>& callback // new callback
		)
		{
			if (!raw_callback) {
				raw_callback = [this, &arr](double t, param_t& sample) {
					for (const auto& cb : arr) cb.value(t, sample);
				};
			}
			const unsigned id = arr.size();
			arr.push_back(std::move(give_id<std::function<void(double, param_t&)>>{ id, callback }));
			return id;
		}

		template<class callback_t>
		void removeCallback(std::vector<give_id<callback_t>>& arr, unsigned id)
		{
			auto found = std::find_if(arr.begin(), arr.end(), [id](const auto& elem) {
				return elem.id == id;
			});
			if (found == arr.end()) {
				throw std::logic_error("The given callback was not registered before.");
			}
			else {
				arr.erase(found);
				if (arr.size() == 0) {
					afterSample = {};
				}
			}
		}

		const unsigned maxTones;
		mutable std::atomic<double> lastTime{ 0 };
		mutable std::mutex mtx;
		std::vector<give_id<after_t>> afterSampleCallbacks;
		std::vector<give_id<before_t>> beforeSampleCallbacks;
		TimbreModel timbreModel;
		ADSREnvelope env;
		before_t beforeSample; // shadowing base class' beforeSample, because of its type
};


#endif // GENERATORS_H_INCLUDED
