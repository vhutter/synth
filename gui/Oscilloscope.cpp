#include "Oscilloscope.h"

Oscilloscope::Oscilloscope(SynthFloat sx, SynthFloat sy, unsigned res, double speed)
	: resolution(res), speed(speed)
{
	window.setSize(sf::Vector2f(sx, sy));
	window.setOutlineColor(sf::Color::White);
	window.setFillColor(sf::Color::Black);
	window.setOutlineThickness(-2.);

	vArray.reserve(resolution * 2); // bigger buffer size for faster incoming samples
	vArray.resize(resolution);
	const auto& pos = window.getPosition();
	const auto& size = window.getSize();
	for (unsigned i = 0; i < resolution; ++i) {
		vArray[i].position.x = pos.x + i * size.x / resolution;
		vArray[i].position.y = pos.y + size.y / 2;
		vArray[i].color = sf::Color::Green;
	}
}

SynthRect Oscilloscope::AABB() const
{
	return { SynthVec2(getPosition()), SynthVec2(window.getSize()) };
}

void Oscilloscope::drawImpl(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(window, states);
	target.draw(vArray.data(), resolution, sf::LineStrip, states);
}

void Oscilloscope::newSamples(const std::vector<double>& samples) const
{
	const double halfY = window.getSize().y / 2;
	const unsigned dif = samples.size();
	if (dif < vArray.size()) {
		for (auto i = vArray.begin(); i < vArray.end() - dif; ++i)
			i->position.y = (i + dif)->position.y;
		const std::size_t shift = vArray.size() + dif;
		for (auto i = vArray.end() - dif; i < vArray.end(); ++i)
			i->position.y = window.getPosition().y + halfY + samples[i - vArray.begin() - shift] * halfY;
	}
	else {
		for (unsigned i = 0; i < vArray.size(); ++i)
			vArray[i].position.y = window.getPosition().y + halfY + samples[i] * halfY;
	}
}