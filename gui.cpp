#include "gui.h"
#include "gui/GuiElement.h"
#include "gui/Window.h"
#include "gui/Slider.h"
#include "instruments/Instrument.h"

namespace
{

	template<class T>
	void addAfterEffects(std::shared_ptr<Window> mainWindow, Instrument<T>& inst)
	{
		auto gui = mainWindow->getContentFrame();
		auto menu = mainWindow->getMenuFrame();
		using pos_t = MenuOption::OptionList::ChildPos_t;

		static auto volume = VolumeControl();
		gui->addChildAutoPos(volume.getFrame());

		auto delay = DelayEffect(inst.getSampleRate(), 1., 0.6);
		auto delayWindow = std::make_shared<Window>(delay.getFrame());
		delayWindow->setHeader(30, "Delay");
		delayWindow->setVisibility(false);
		gui->addChildAutoPos(delayWindow);

		auto debugFilter = DebugFilter();
		auto debugWindow = std::make_shared<Window>(debugFilter.getFrame());
		debugWindow->setHeader(30, "Debug");
		debugWindow->setVisibility(false);
		gui->addChildAutoPos(debugWindow);

		menu->addChildAutoPos(MenuOption::createMenu(
			100, 30, 15, {
				"View", pos_t::Down, {{
					"Debug", debugWindow}, {
					"Effects", {{
						"Delay", delayWindow},
					}}
				}
			}
		));

		inst.getGenerator().addAfterCallback(delay);
		inst.getGenerator().addAfterCallback(volume);
		inst.getGenerator().addAfterCallback(debugFilter);
	}	

}

void setupGui(std::shared_ptr<Window> mainWindow, sf::RenderWindow& renderWindow)
{
	renderWindow.setKeyRepeatEnabled(false);
	renderWindow.setVerticalSyncEnabled(true);
	auto setup = std::make_shared<EmptyGuiElement>([&](const sf::Event& event) {
		switch (event.type) {
			case sf::Event::Closed: {
				renderWindow.close();
				break;
			}
			case sf::Event::KeyPressed: {
				switch (event.key.code) {
				case sf::Keyboard::Escape: {
					renderWindow.close();
					break;
				}
				default:
					break;
				}
				break;
			}
			case sf::Event::Resized: {
				sf::View view(sf::FloatRect(0, 0, event.size.width, event.size.height));
				renderWindow.setView(view);
				break;
			}
			default:
				break;
		}
	});

	mainWindow->addEmptyListener(setup);

	auto gui = mainWindow->getContentFrame();
	auto menu = mainWindow->getMenuFrame();
	using pos_t = MenuOption::OptionList::ChildPos_t;

	const unsigned maxNotes{ 5 }, sampleRate{ 44100 }, bufferSize{ 16 };
	auto notes = generateNotes(0, 2);

	static Instrument1 inst(sampleRate, bufferSize, Sine13, notes, maxNotes);
	gui->addChild(inst.getGuiElement(), 50, 50);

	addAfterEffects(mainWindow, inst);

	inst.play();
}
