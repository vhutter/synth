#include "Window.h"
#include "TextDisplay.h"

#include <limits>

Window::Window(SynthFloat sx, SynthFloat sy, const sf::Color & fillColor)
	:content(std::make_shared<Frame>(sx, sy)),
	headerPart(std::make_shared<Frame>()),
	header(std::make_shared<TextDisplay>()),
	menuBar(std::make_shared<Frame>()),
	exitButton(std::make_shared<Button>("X", 0, 0, 0, 0, 0, [this]() {
		this->setVisibility(false);
	}))
{
	setPosition(0, 0);

	content->setSize({ sx, sy });
	content->setBgColor(fillColor);
	content->setOutlineThickness(0);
	content->setCropping(true);

	headerPart->addChildren({ menuBar, header });
	addChildren({ content, headerPart });
	header->addChildren({ exitButton });
	header->setOutlineColor(sf::Color::Black);
	header->setOutlineThickness(-1);

	content->setEventCallback([this](const sf::Event& event) {
		if (event.type == sf::Event::MouseButtonPressed) {
			for (auto child : getChildren(menuBar)) {
				if (auto menuOpt = dynamic_cast<MenuOption*>(child.get())) {
					if (menuOpt->isActive()) {
						menuOpt->toggle();
					}
				}
			}
		}
	});
	content->setFocusable(false);
}

void Window::onSfmlEvent(const sf::Event & event)
{
	if (!focusable) return;

	switch (event.type) {
	case sf::Event::MouseButtonPressed: {
		const auto mousePos = sf::Vector2f(event.mouseButton.x, event.mouseButton.y);

		auto headerRect = sf::FloatRect(
			globalTransform * header->getPosition(),
			sf::Vector2f(header->getSize())
		);
		lastMousePos = mousePos;
		if (headerRect.contains(mousePos)) {
			moving = true;
			break;
		}
		break;
	}
	case sf::Event::MouseButtonReleased: {
		moving = false;
		break;
	}
	case sf::Event::MouseMoved: {
		if (moving) {
			const auto& current = sf::Vector2f(event.mouseMove.x, event.mouseMove.y);
			const auto& dif = sf::Vector2f(current - lastMousePos);
			move(dif);
			lastMousePos = current;
		}
		break;
	}
	}
	if (event.type == sf::Event::MouseButtonPressed) {
		SynthRect rect = { SynthVec2(globalTransform * header->getPosition()), SynthVec2(content->getSize().x, header->getSize().y) };

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
}

void Window::setSize(const SynthVec2 & size)
{
	content->setSize(size);
	if (header) {
		header->setFixedSize(false);
		header->setSize(SynthVec2(size.x, header->getSize().y));
		header->setFixedSize(true);
	}
	if (menuBar) {
		menuBar->setSize(SynthVec2(size.x, menuBar->getSize().y));
	}
}

SynthVec2 Window::getSize() const
{
	return SynthVec2(content->getSize());
}

void Window::addMenuOption(std::shared_ptr<MenuOption> option)
{
	if (!menuBar) return;
	option->setSize({ option->getSize().x, menuBar->getSize().y });
	option->centralize();
	menuBar->addChildrenAutoPos({ option });
}

void Window::addMenuOptions(const std::vector<std::shared_ptr<MenuOption>>& options)
{
	for (auto option : options) {
		addMenuOption(option);
	}
}

void Window::fixLayout()
{
	header->setPosition(0, 0);
	menuBar->setPosition(0, header->getSize().y);
	headerPart->fitToChildren();
	content->setPosition(0, headerPart->getSize().y);
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
	// The window will be movable
	setFocusable(true);

	header->setTextColor(sf::Color::Black);
	header->setBgColor(sf::Color::White);

	header->setFixedSize(false);
	header->setSize(SynthVec2( content->getSize().x, size ));
	header->setFixedSize(true);
	header->setText(title);
	if (!textSize)
		textSize = defaultTextSize(size);
	header->setTextSize(textSize);
	header->centralize();

	exitButton->setTextSize(textSize);
	exitButton->setSize(SynthVec2( size, size ));
	exitButton->centralize();
	exitButton->setPosition(header->getSize().x - size, 0);

	fixLayout();
}

void Window::setMenuBar(unsigned size)
{
	menuBar->setSize(SynthVec2(content->getSize().x, size));
	menuBar->setPosition(0, header ? header->getSize().y : 0);

	fixLayout();
}

const std::shared_ptr<Frame>& Window::getContentFrame() const
{
	return content;
}

SynthRect Window::AABB() const
{
	return {
		SynthVec2(getPosition()),
		SynthVec2(content->getPosition() - (header ? header->getPosition() : sf::Vector2f(0,0)) + sf::Vector2f(content->getSize()))
	};
}
