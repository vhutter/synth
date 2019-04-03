#include "test.h"

void testGui(GuiElement& gui)
{
	auto events = std::make_shared<EmptyGuiElement>([&](const SynthEvent& eventArg) {
		if (std::holds_alternative<sf::Event>(eventArg)) {
			const sf::Event& event = std::get<sf::Event>(eventArg);

			switch (event.type) {
			case sf::Event::KeyPressed: {
				switch (event.key.code) {
				case sf::Keyboard::Space: {
					// For testing

					break;
				}
				default:
					break;
				}
				break;
			}
			default:
				break;
			}
		}
	});

	//auto test = std::make_shared<TextDisplay>("The quick brown fox jumps over the lazy dog", 600, 200, 300, 300);
	std::shared_ptr test = TextDisplay::Multiline("The quick brown fox jumps over the lazy dog", 600, 200, 50, 24);
	std::shared_ptr test2 = TextDisplay::DefaultText("teeesztqq", 250, 200, 24);
	std::shared_ptr test3 = std::make_unique<TextDisplay>("qweasd", 700, 700, 100, 100, 24);
	test2->centralize();
	test2->moveAroundPoint({ 70,70 });
	test->moveAroundPoint({ 625,290 });
	test->setBgColor({ 0,0,0,0 });
	test->fitFrame();

	gui.addChildren({
		events,
		//std::make_shared<Window>(10, 10, 500, 500, sf::Color{ 0x8b, 0, 0, 100 }) ,
		test,
		test2,
		test3,
		});
}