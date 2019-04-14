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
	void addChildrenAutoPos(const std::vector<std::shared_ptr<GuiElement>>& children);
	void addMenuOption(std::shared_ptr<MenuOption> option);
	
	void setHeader(unsigned size, const std::string& title, unsigned textSize=0);
	void setMenuBar(unsigned size);
	void setChildAlignment(unsigned a);
	void setCursor(unsigned x, unsigned y);

	virtual SynthRect AABB() const override;
	virtual bool needsEvent(const SynthEvent& event) const;
	virtual sf::View childrenView(const sf::RenderTarget& target, const sf::RenderStates& states) const override;
	virtual bool forwardsEvent(const SynthEvent& event) const override;

private:

	unsigned defaultTextSize(unsigned frameSize);
	virtual void drawImpl(sf::RenderTarget& target, sf::RenderStates states) const override;
	virtual void onSfmlEvent(const sf::Event& event) override;
	const sf::FloatRect globalMainRect() const;

	unsigned childAlignment{ 0 }, cursorX{ 0 }, cursorY{ 0 }, rowHeight{ 0 }; // variables for automatic positioning of children
	std::unique_ptr<TextDisplay> header;
	std::unique_ptr<MenuBar> menuBar;
	sf::RectangleShape mainRect;

	sf::Vector2i lastMousePos;
	bool moving{ false };
};

class MenuBar : public Window
{
public:
	using Window::Window;
	virtual sf::View childrenView(const sf::RenderTarget& target, const sf::RenderStates& states) const override;
};

#endif //WINDOW_H_INCLUDED