#ifndef FRAME_H_INCLUDED
#define FRAME_H_INCLUDED

#include "GuiElement.h"

class Frame : public GuiElement
{
public:
	Frame() = default;
	Frame(const SynthFloat sx, const SynthFloat sy);
	Frame(const SynthVec2& size);

	const sf::Color& getBgColor() const;
	const sf::Color& getOutlineColor() const;
	const SynthFloat getOutlineThickness() const;
	const SynthVec2 getSize() const;

	void setBgColor(const sf::Color& color);
	void setOutlineColor(const sf::Color& color);
	void setOutlineThickness(SynthFloat thickness);
	void setSize(const SynthVec2& size);

	virtual SynthRect AABB() const override;

protected:
	virtual void drawImpl(sf::RenderTarget& target, sf::RenderStates states) const override;

	sf::RectangleShape frame;
};

#endif // FRAME_H_INCLUDED
