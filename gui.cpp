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
					gui.move(3., 3.);
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

	gui.addChildren({ setup });
}
