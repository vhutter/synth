#include "guiElements.h"

#include <mutex>

#include "generators.h"
#include "guiElements.h"
#include "gui.h"


KeyboardOutput::KeyboardOutput()
{
	kb = std::make_unique<SynthKeyboard>(50, 700, [this](unsigned keyIdx, SynthKey::State keyState) {
		if (keyState == SynthKey::State::Pressed) {
			(*kb)[keyIdx].pressed = true;
		}
		else {
			(*kb)[keyIdx].pressed = false;
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
	auto setup = std::make_shared<EmptyGuiElement>([&](const sf::Event& event) {
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
	});

	gui.addChildren({ 
		setup,
	});
}
