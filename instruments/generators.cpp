#include "generators.h"
#include "../Instruments/tones.h"

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

void ADSREnvelope::start(double t) const
{
    startAmp = getAmplitude(t);
    beginTime = t;
    isHeld = true;
    nonzero = true;
}

void ADSREnvelope::stop(double t) const
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
    reach = sf::Vector2<double>(endTime, endValue);
}

Note::Note(double f)
    :freq(f), initialFreq(freq)
{}

const Note Note::C  (523.25);
const Note Note::Cis(554.37);
const Note Note::D  (587.33);
const Note Note::Dis(622.25);
const Note Note::E  (659.25);
const Note Note::F  (698.46);
const Note Note::Fis(739.99);
const Note Note::G  (783.99);
const Note Note::Gis(830.61);
const Note Note::A  (880.00);
const Note Note::Ais(932.33);
const Note Note::B  (987.77);

const std::array<Note, 12> Note::baseNotes{ C, Cis, D, Dis, E, F, Fis, G, Gis, A, Ais, B };

WaveGenerator::WaveGenerator(
	const Note& note,
	double intensity,
	waves::wave_t waveform,
	before_t before,
	after_t  after
)
	:SampleGenerator<WaveGenerator>(before, after),
	freq(note),
	intensity(intensity),
	waveform(waveform)
{}

void WaveGenerator::modifyMainPitchImpl(double t, double f2)
{
	auto& component = *this;
	double f1 = component.freq;
	double p = (t + component.phase) * f1 / f2 - t;
	p = fmod(p, 2 * M_PI*f2);
	component.phase = p;
	component.freq = f2;
}

double WaveGenerator::getSampleImpl(double t) const
{
    double result = waveform(t, this->intensity, this->freq, this->phase);
    return result;
}

const double WaveGenerator::getMainFreqImpl() const
{
	return freq.getInitialFreq();
}

DynamicToneSum::DynamicToneSum(
	const TimbreModel& timbreModel,
	const std::vector<Note>& notes,
	unsigned maxTones
)
	:base_t(generateTones<WaveGenerator>(timbreModel, notes)),
	maxTones(maxTones),
	timbreModel(timbreModel)
{}
DynamicToneSum::DynamicToneSum(const DynamicToneSum& that)
	:DynamicToneSum(that.timbreModel, that.getNotes(), that.maxTones)
{}

void DynamicToneSum::lock() const { mtx.lock(); }
void DynamicToneSum::unlock() const { mtx.unlock(); }
double DynamicToneSum::time() const { return lastTime.load(); }

//double DynamicToneSum::getSample(double t) const
//{
//	lastTime.store(t);
//	return Base_t::getSample(t);
//}

double DynamicToneSum::getSample(double t) const
{
	// The application of effects looks ugly
	lastTime.store(t);
	if (beforeSample) beforeSample(t, *const_cast<DynamicToneSum*>(this));
	double result{ 0. };
	std::size_t count{ 0 };
	for (auto& c : components) {
		if (count >= maxTones) break;
		if (auto sample = c.getSample(t)) {
			++count;
			result += sample.value();
		}
	}
	result /= maxTones;
	if (afterSample) afterSample(t, result);
	return result;
}

unsigned DynamicToneSum::getMaxTones() const
{
	return maxTones;
}

std::vector<Note> DynamicToneSum::getNotes() const
{
	std::vector<Note> ret;
	ret.reserve(initialComponents.size());
	for (const auto& component : initialComponents)
		ret.push_back(component.getMainFreq());
	return ret;
}

const TimbreModel& DynamicToneSum::getTimbreModel() const
{
	return timbreModel;
}

unsigned DynamicToneSum::addAfterCallback(after_t callback) 
{ 
	return addCallback(afterSample, afterSampleCallbacks, callback); 
}
void DynamicToneSum::removeAfterCallback(unsigned id)
{ 
	removeCallback(afterSampleCallbacks, id); 
}
unsigned DynamicToneSum::addBeforeCallback(before_t callback)
{ 
	return addCallback(beforeSample, beforeSampleCallbacks, callback);
}
void DynamicToneSum::removeBeforeCallback(unsigned id) 
{ 
	removeCallback(beforeSampleCallbacks, id); 
}

void DynamicToneSum::onKeyEvent(unsigned keyIdx, SynthKey::State keyState)
{
	if (keyState == SynthKey::State::Pressed) {
		components[keyIdx].start(this->time());
	}
	else {
		components[keyIdx].stop(this->time());
	}
}

TimbreModel::TimbreModel(
	std::vector<TimbreModel::ToneSkeleton> components,
	WaveGenerator::before_t         beforeTone,
	WaveGenerator::after_t          afterTone,
	Composite<WaveGenerator>::before_t before,
	Composite<WaveGenerator>::after_t  after
)
	:components(components),
	beforeTone(beforeTone),
	afterTone(afterTone),
	before(before),
	after(after)
{}

Composite<WaveGenerator> TimbreModel::operator()(const double& baseFreq) const
{
	std::vector<WaveGenerator> tones;
	tones.reserve(components.size());
	std::transform(
		components.begin(),
		components.end(),
		std::back_inserter(tones),
		[&baseFreq, this](const ToneSkeleton& component) {
		return WaveGenerator(
			baseFreq * component.relativeFreq,
			component.intensity,
			component.waveform,
			beforeTone,
			afterTone
		);
	}
	);
	return Composite(tones, before, after);
}