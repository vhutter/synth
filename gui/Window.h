#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED

#include "GuiElement.h"
#include "MenuOption.h"

#include <memory>

class MenuBar;

class TextDisplay;

class Window : public GuiElement
{
public:
	Window(SynthFloat sx, SynthFloat sy, const sf::Color& fillColor);

	void setSize(const SynthVec2& size);
	SynthVec2 getSize() const;
	void addMenuOption(std::shared_ptr<MenuOption> option);
	
	void setHeader(unsigned size, const std::string& title, unsigned textSize=0);
	void setMenuBar(unsigned size);

	const std::shared_ptr<Frame>& getContentFrame() const;

	virtual SynthRect AABB() const override;

private:
	using GuiElement::addChildren;

	void fixLayout();
	unsigned defaultTextSize(unsigned frameSize);
	virtual void drawImpl(sf::RenderTarget& target, sf::RenderStates states) const override;
	virtual void onSfmlEvent(const sf::Event& event) override;

	std::shared_ptr<Frame> content;
	std::shared_ptr<Frame> headerPart;
	std::shared_ptr<TextDisplay> header;
	std::shared_ptr<Frame> menuBar;
	std::shared_ptr<Button> exitButton;

	sf::Vector2f lastMousePos;
	bool moving{ false };
};

#endif //WINDOW_H_INCLUDED