#ifndef OSCILLOSCOPE_H_INCLUDED
#define OSCILLOSCOPE_H_INCLUDED

#include "GuiElement.h"

class Oscilloscope : public GuiElement
{
public:
	Oscilloscope(SynthFloat sx, SynthFloat sy, unsigned resolution);

	virtual SynthRect AABB() const override;

	unsigned getResolution() const { return resolution; }
	void newSamples(const std::vector<double>& samples) const;

private:
	virtual void drawImpl(sf::RenderTarget& target, sf::RenderStates states) const override;

	sf::RectangleShape window;
	mutable std::vector<sf::Vertex> vArray;
	const unsigned resolution;
	double speed;
	double currTime = 0;
};

#endif //OSCILLOSCOPE_H_INCLUDED