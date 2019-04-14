#include "gui.h"
#include "gui/GuiElement.h"

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
