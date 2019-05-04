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

void ADSREnvelope::calculateTimePoints()
{
	decayTime = attackTime + decayDur;
	sustainTime = decayTime + sustainDur;
	releaseTime = sustainTime + releaseDur;
}

ADSREnvelope::ADSREnvelope(double a, double d, double s, double r, double h)
	:attackTime(a), decayDur(d), sustainDur(h), releaseDur(r), sustainLevel(s)
{
}

void ADSREnvelope::start(double t)
{
	calculateTimePoints();
	startAmp = getAmplitude(t);
	beginTime = t;
	lastStopTime = 0;
	nonzero = true;
	isHeld = true;
}

void ADSREnvelope::stop(double t)
{
	// return if it is being called redundantly
	if (!isHeld)
		return;

	// if sustain duration is indefinite, we tweak it a little bit
	// for getAmplitude to be working
	if (sustainDur == inf) {
		sustainTime = t;
		releaseTime = sustainTime + releaseDur;
	}

	double timeUntilSustain = sustainTime - t + beginTime;
	if (timeUntilSustain > 0)
		lastStopTime = sustainTime - t + beginTime;
	else
		lastStopTime = 0;
	isHeld = false;
}

double ADSREnvelope::getAmplitude(double t) const
{
	t = t - beginTime + lastStopTime;
	if (nonzero) {
		if (t <= attackTime)
			currentAmp = t * (1 - startAmp) / attackTime + startAmp;
		else if (t < decayTime)
			currentAmp = 1 - (t - attackTime) * (1 - sustainLevel) / decayDur;
		else if (t < sustainTime)
			currentAmp = sustainLevel;
		else  if (t < releaseTime)
			// we saved currentAmp for this case:
			// the key may have been released before reaching the sustain level,
			// in which case we have to calculate the appropiate decay level by
			// considering the last available amplitude
			return currentAmp * (1 - (t-sustainTime) / releaseDur);
		else {
			nonzero = false;
			currentAmp = 0;
		}
	}
	return currentAmp;
}


ContinuousFunction::ContinuousFunction(double initConst)
	: value(initConst), m(0) {}

double ContinuousFunction::getValue(double t) const
{
	if (m != 0 && t >= startTime) {
		value = startValue + m * (t - startTime);

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
}

const Note& Note::A()   { static const Note n(16.35); return n; }
const Note& Note::Ais() { static const Note n(17.32); return n; }
const Note& Note::B()   { static const Note n(18.35); return n; }
const Note& Note::C()   { static const Note n(19.45); return n; }
const Note& Note::Cis() { static const Note n(20.60); return n; }
const Note& Note::D()   { static const Note n(21.83); return n; }
const Note& Note::Dis() { static const Note n(23.12); return n; }
const Note& Note::E()   { static const Note n(24.50); return n; }
const Note& Note::F()   { static const Note n(25.96); return n; }
const Note& Note::Fis() { static const Note n(27.50); return n; }
const Note& Note::G()   { static const Note n(29.14); return n; }
const Note& Note::Gis() { static const Note n(30.87); return n; }

const std::array<Note, 12> Note::baseNotes()
{
	return { A(), Ais(), B(), C(), Cis(), D(), Dis(), E(), F(), Fis(), G(), Gis() };
}

WaveGenerator::WaveGenerator(
	const double& note,
	double intensity,
	waves::wave_t waveform
)
	:freq(note),
	intensity(intensity),
	waveform(waveform)
{}

void WaveGenerator::modifyMainPitchImpl(double t, double f2)
{
	double f1 = this->freq;
	double p = (t + this->phase) * f1 / f2 - t;
	p = std::fmod(p, 2 * M_PI*f2);
	this->phase = p;
	this->freq = f2;
}

double WaveGenerator::getSampleImpl(double t) const
{
    double result = waveform(t, this->intensity, this->freq, this->phase);
    return result;
}

const double WaveGenerator::getMainFreqImpl() const
{
	return freq.getInitial();
}

DynamicToneSum::DynamicToneSum(
	const TimbreModel& timbreModel,
	const ADSREnvelope& env,
	const std::vector<Note>& notes,
	unsigned maxTones
)
	:base_t(generateTones<WaveGenerator>(timbreModel, notes, env)),
	env(env),
	maxTones(maxTones),
	timbreModel(timbreModel)
{}
DynamicToneSum::DynamicToneSum(const DynamicToneSum& that)
	:DynamicToneSum(that.timbreModel, that.env, that.getNotes(), that.maxTones)
{}

void DynamicToneSum::lock() const { mtx.lock(); }
void DynamicToneSum::unlock() const { mtx.unlock(); }
double DynamicToneSum::time() const { return lastTime.load(); }

double DynamicToneSum::getSample(double t) const
{
	std::lock_guard lock(*this);
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

unsigned DynamicToneSum::getNotesCount() const
{
	return initialComponents.size();
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
		components.at(keyIdx).start(this->time());
	}
	else {
		components.at(keyIdx).stop(this->time());
	}
}

TimbreModel::TimbreModel(
	std::vector<TimbreModel::ToneSkeleton> components
)
	:components(components)
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
			component.waveform
		);
	}
	);
	return Composite(tones);
}

Dynamic<Composite<WaveGenerator>> TimbreModel::operator()(const double& baseFreq, const ADSREnvelope& env) const
{
	return Dynamic{ operator()(baseFreq), env };
}