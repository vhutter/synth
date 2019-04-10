#include "Window.h"
#include "TextDisplay.h"

Window::Window(const std::string& title, SynthFloat px, SynthFloat py, SynthFloat sx, SynthFloat sy, const sf::Color & fillColor)
	:mainRect(sf::Vector2f(sx, sy)), header(std::make_unique<TextDisplay>())
{
	setDynamic(true);
	setPosition(px, py);
	mainRect.setPosition(0, headerSize);
	mainRect.setFillColor(fillColor);

	mainRect.setOutlineThickness(0);
	mainRect.setOutlineColor(sf::Color::Green);

	header->setPosition(0, 0);
	header->setTextColor(sf::Color::Black);
	header->setText(title);
	header->setBgColor(sf::Color::White);
	header->setFrameSize(SynthVec2(sx, headerSize));
	header->setFixedSize(true);
	header->centralize();
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

		else if (globalMainRect().contains(sf::Vector2f(pEvent->mouseButton.x, pEvent->mouseButton.y))) {
			return true;
		}

		else
			return false;
	}
	else return true;
}

void Window::onSfmlEvent(const sf::Event & event)
{
	switch (event.type) {
	case sf::Event::MouseButtonPressed: {
		SynthRect rect = { SynthVec2(globalTransform * header->getPosition()), SynthVec2(mainRect.getSize().x, headerSize) };
		lastMousePos = sf::Vector2i(event.mouseButton.x, event.mouseButton.y);
		if (rect.contains(event.mouseButton.x, event.mouseButton.y)) {
			moving = true;
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
		SynthRect rect = { SynthVec2(globalTransform * header->getPosition()), SynthVec2(mainRect.getSize().x, headerSize) };

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
	target.draw(*header, states);
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
}

const SynthVec2& Window::getSize()
{
	return std::move(SynthVec2(mainRect.getSize()));
}

void Window::addChildrenAutoPos(const std::vector<std::shared_ptr<GuiElement>>& children)
{
	addChildren(children);
}

SynthRect Window::AABB() const
{
	return {
		SynthVec2(getPosition()),
		SynthVec2(mainRect.getSize() + sf::Vector2f(0, header->AABB().height))
	};
}