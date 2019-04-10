#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED

#include "GuiElement.h"

class TextDisplay;

class Window : public GuiElement
{
public:
	Window(const std::string& title, SynthFloat px, SynthFloat py, SynthFloat sx, SynthFloat sy, const sf::Color& fillColor);

	void setSize(const SynthVec2& size);
	const SynthVec2& getSize();
	void addChildrenAutoPos(const std::vector<std::shared_ptr<GuiElement>>& children);

	virtual SynthRect AABB() const override;
	virtual bool needsEvent(const SynthEvent& event) const;
	virtual sf::View childrenView(const sf::RenderTarget& target, const sf::RenderStates& states) const override;
	virtual bool forwardsEvent(const SynthEvent& event) const override;

private:
	virtual void drawImpl(sf::RenderTarget& target, sf::RenderStates states) const override;
	virtual void onSfmlEvent(const sf::Event& event) override;
	const sf::FloatRect globalMainRect() const;

	SynthFloat childAlignment{ 10 }, cursorX{ 0 }, cursorY{ 0 }, headerSize{ 30 };
	std::unique_ptr<TextDisplay> header;
	sf::RectangleShape mainRect;

	sf::Vector2i lastMousePos;
	bool moving{ false };
};

#endif //WINDOW_H_INCLUDED