#include "Window.h"
#include "TextDisplay.h"

#include <limits>

Window::Window(SynthFloat sx, SynthFloat sy, const sf::Color & fillColor)
	:mainRect(sf::Vector2f(sx, sy))
{
	setPosition(0, 0);
	mainRect.setFillColor(fillColor);
	mainRect.setOutlineThickness(0);
}

const sf::FloatRect Window::globalMainRect() const
{
	return globalTransform.transformRect(sf::FloatRect(
		mainRect.getPosition(),
		mainRect.getSize()
	));
}

bool Window::needsEvent(const SynthEvent & event) const
{
	if (auto pEvent = std::get_if<sf::Event>(&event)) {
		if (pEvent->type == sf::Event::MouseMoved) {
			const auto& rect = globalMainRect();
			if (!rect.contains(sf::Vector2f(pEvent->mouseMove.x, pEvent->mouseMove.y)) &&
				rect.contains(sf::Vector2f(lastMousePos))) {
				return false;
			}
		}
	}
	return true;
}

bool Window::forwardsEvent(const SynthEvent& event) const
{
	if (auto pEvent = std::get_if<sf::Event>(&event)) {
		if (pEvent->type != sf::Event::MouseButtonPressed)
			return true;
		else if (globalMainRect().contains(sf::Vector2f(pEvent->mouseButton.x, pEvent->mouseButton.y)))
			return true;
		else
			return false;
	}
	else 
		return true;
}

void Window::onSfmlEvent(const sf::Event & event)
{
	if (!dynamic) return;

	switch (event.type) {
	case sf::Event::MouseButtonPressed: {
		const auto mousePos = sf::Vector2i(event.mouseButton.x, event.mouseButton.y);

		auto headerRect = SynthRect(
			SynthVec2(globalTransform * header->getPosition()),
			SynthVec2(mainRect.getSize().x, header->getFrameSize().y)
		);
		lastMousePos = mousePos;
		if (headerRect.contains(SynthVec2(mousePos))) {
			moving = true;
		}

		if (menuBar) {
			auto menuRect = globalTransform.transformRect(sf::FloatRect(menuBar->AABB()));
			if (menuRect.contains(sf::Vector2f(mousePos))) {
				menuBar->forwardEvent(event, globalTransform);
			}
		}
		break;
	}
	case sf::Event::MouseButtonReleased: {
		moving = false;
		break;
	}
	case sf::Event::MouseMoved: {
		if (moving) {
			const auto& current = sf::Vector2i(event.mouseMove.x, event.mouseMove.y);
			const auto& dif = sf::Vector2f(current - lastMousePos);
			move(dif);
			lastMousePos = current;
		}
		break;
	}
	}
	if (event.type == sf::Event::MouseButtonPressed) {
		SynthRect rect = { SynthVec2(globalTransform * header->getPosition()), SynthVec2(mainRect.getSize().x, header->getFrameSize().y) };

		if (rect.contains(event.mouseButton.x, event.mouseButton.y)) {
			moving = true;
		}
	}
	else if (event.type == sf::Event::MouseButtonReleased) {
		moving = false;
	}
}

void Window::drawImpl(sf::RenderTarget & target, sf::RenderStates states) const
{
	target.draw(mainRect, states);
	if(menuBar) target.draw(*menuBar, states);
	if(header) target.draw(*header, states);
}

sf::View Window::childrenView(const sf::RenderTarget& target, const sf::RenderStates& states) const
{
	return getCroppedView(
		target.getView(),
		SynthVec2(states.transform.transformPoint(mainRect.getPosition())),
		SynthVec2(mainRect.getSize())
	);
}

void Window::setSize(const SynthVec2 & size)
{
	mainRect.setSize(sf::Vector2f(size));
	if (header) {
		header->setFixedSize(false);
		header->setFrameSize(SynthVec2(size.x, header->getFrameSize().y));
		header->setFixedSize(true);
	}
	if (menuBar) {
		menuBar->setSize(SynthVec2(size.x, menuBar->getSize().y));
	}
}

SynthVec2 Window::getSize() const
{
	return SynthVec2(mainRect.getSize());
}

void Window::addChildrenAutoPos(const std::vector<std::shared_ptr<GuiElement>>& children)
{
	addChildren(children);
	for (auto child : children) {
		child->setPosition(cursorX, cursorY);
		const auto& aabb = child->AABB();
		cursorX += aabb.width + childAlignment;
		rowHeight = std::max(rowHeight, unsigned(aabb.height));
		if (cursorX > getSize().x) {
			cursorY += rowHeight + childAlignment;
			rowHeight = 0;
		}
	}
}

void Window::addMenuOption(std::shared_ptr<MenuOption> option)
{
	if (!menuBar) return;
	option->setFrameSize({ option->getFrameSize().x, menuBar->getSize().y });
	option->centralize();
	menuBar->addChildrenAutoPos({ option });
}

unsigned Window::defaultTextSize(unsigned frameSize)
{
	if (frameSize > 10)
		return frameSize - 10;
	else
		return frameSize;
}

void Window::setHeader(unsigned size, const std::string & title, unsigned textSize)
{
	if (!header) header = std::make_unique<TextDisplay>();

	// The window will be movable
	setDynamic(true);

	header->setPosition(0, 0);
	header->setTextColor(sf::Color::Black);
	header->setBgColor(sf::Color::White);

	header->setFixedSize(false);
	header->setFrameSize(SynthVec2( mainRect.getSize().x, size ));
	header->setFixedSize(true);
	header->setText(title);
	if (textSize)
		header->setTextSize(textSize);
	else
		header->setTextSize(defaultTextSize(size));
	header->centralize();

	mainRect.setPosition(sf::Vector2f(0, size));
	if (menuBar) {
		menuBar->setPosition(sf::Vector2f(0, size));
		mainRect.move(sf::Vector2f(0, menuBar->getSize().y));
	}
}

void Window::setMenuBar(unsigned size)
{
	if (!size) {
		menuBar.reset(nullptr);
	}
	mainRect.move(0, size);
	menuBar = std::make_unique<MenuBar>(mainRect.getSize().x, size, sf::Color(128, 128, 128, 255));
	menuBar->setPosition(0, header ? header->getFrameSize().y : 0);
}

void Window::setChildAlignment(unsigned a)
{
	childAlignment = a;
}

SynthRect Window::AABB() const
{
	return {
		SynthVec2(getPosition()),
		SynthVec2(mainRect.getPosition() - (header ? header->getPosition() : sf::Vector2f(0,0)) + mainRect.getSize())
	};
}

sf::View MenuBar::childrenView(const sf::RenderTarget & target, const sf::RenderStates & states) const
{
	return GuiElement::childrenView(target, states);
}
