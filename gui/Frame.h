#ifndef FRAME_H_INCLUDED
#define FRAME_H_INCLUDED

#include "GuiElement.h"

class Frame : public GuiElement
{
public:
	Frame() = default;
	Frame(const SynthFloat sx, const SynthFloat sy);
	Frame(const SynthVec2& size);
	
	void fitToChildren();
	void setChildAlignment(unsigned a);
	void setCursor(unsigned x, unsigned y);
	void addChildrenAutoPos(const std::vector<std::shared_ptr<GuiElement>>& children);
	void setCropping(bool crop);

	const sf::Color& getBgColor() const;
	const sf::Color& getOutlineColor() const;
	const SynthFloat getOutlineThickness() const;
	const SynthVec2 getSize() const;

	void setBgColor(const sf::Color& color);
	void setOutlineColor(const sf::Color& color);
	void setOutlineThickness(SynthFloat thickness);
	void setSize(const SynthVec2& size);

	virtual SynthRect AABB() const override;
	virtual sf::View childrenView(const sf::RenderTarget& target, const sf::RenderStates& states) const override;
	virtual bool needsEvent(const SynthEvent& event) const;
	virtual bool forwardsEvent(const SynthEvent& event) const override;

protected:
	virtual void drawImpl(sf::RenderTarget& target, sf::RenderStates states) const override;

	sf::RectangleShape frame;

private:
	const sf::FloatRect globalFrame() const;

	unsigned childAlignment{ 0 }, cursorX{ 0 }, cursorY{ 0 }, rowHeight{ 0 }; // variables for automatic positioning of children
	bool cropping{ false };
};

#endif // FRAME_H_INCLUDED
