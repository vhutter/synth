#include "guiElements.h"

#include <frozen/string.h>

#include <mutex>

#include "generators.h"
#include "guiElements.h"


void setupGui(GuiElement& gui, sf::RenderWindow& window)
{
	auto ptr = std::make_shared<EmptyGuiElement>([&](const SynthEvent& eventArg) {
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

	gui.addChildren({ ptr });
}