#include "guiElements.h"

#include <frozen/string.h>

#include <mutex>

#include "generators.h"
#include "guiElements.h"
#include "gui.h"


KeyboardOutput::KeyboardOutput()
{
	kb = std::make_unique<SynthKeyboard>(50, 700, [this](unsigned keyIdx, SynthKey::State keyState) {
		if (keyState == SynthKey::State::Pressed) {
			(*kb)[keyIdx].setPressed(true);
		}
		else {
			(*kb)[keyIdx].setPressed(false);
		}
		for (auto callback : callbacks) {
			callback(keyIdx, keyState);
		}
	});
}

std::shared_ptr<SynthKeyboard> KeyboardOutput::getGuiElement() const
{
	return kb;
};

void setupGui(GuiElement& gui, sf::RenderWindow& window)
{
	auto setup = std::make_shared<EmptyGuiElement>([&](const SynthEvent& eventArg) {
		if (std::holds_alternative<sf::Event>(eventArg)) {
			const sf::Event& event = std::get<sf::Event>(eventArg);

			switch (event.type) {
			case sf::Event::Closed: {
				window.close();
				break;
			}
			case sf::Event::KeyPressed: {
				switch (event.key.code) {
				case sf::Keyboard::Escape: {
					window.close();
					break;
				}
				case sf::Keyboard::Space: {
					// For testing
					//sf::View oldView = window.getView();
					sf::View oldView(sf::FloatRect(0, 0, 1600, 1000));
					auto oldSize = oldView.getSize();
					auto size = oldSize /2.f;
					auto ratio = sf::Vector2f(size.x / oldSize.x, size.y / oldSize.y);

					static float x = 0, y = 0;
					x = x - 10;
					y = y - 10;

					sf::View view(sf::FloatRect(0, 0, size.x, size.y));
					view.setCenter(x, y);
					view.setViewport(sf::FloatRect(0,0, ratio.x, ratio.y));
					window.setView(view);

					break;
				}
				default:
					break;
				}
				break;
			}
			case sf::Event::Resized: {
				sf::View view(sf::FloatRect(0, 0, event.size.width, event.size.height));
				window.setView(view);
				break;
			}
			default:
				break;
			}
		}
	});

	//auto test = std::make_shared<TextDisplay>("The quick brown fox jumps over the lazy dog", 600, 200, 300, 300);
	auto test = std::make_shared<TextDisplay>(TextDisplay::Multiline("The quick brown fox jumps over the lazy dog", 600, 200, 70, 150, 24));
	//test->setBgColor(sf::Color::Blue);

	gui.addChildren({ 
		setup, 
		//std::make_shared<Window>(10, 10, 500, 500, sf::Color{ 0x8b, 0, 0, 100 }) ,
		test,
	});
}
