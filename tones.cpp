#include "tones.h"

DebugFilter::DebugFilter(GuiElement& gui)
{
	gui.addChildren({ oscilloscope });
}

void DebugFilter::operator()(double t, double& sample)
{
	static auto sampleId = 0u;
	static std::vector<double> lastSamples(oscilloscope->getResolution());
	lastSamples[sampleId++] = sample;
	if (sampleId == 500) {
		oscilloscope->newSamples(lastSamples);
		sampleId = 0;
	}
	if (sample > maxSamp) {
		maxSamp = sample;
		std::cout << maxSamp << "\r";
	}
}
