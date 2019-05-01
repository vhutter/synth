#include "gui.h"
#include "gui/GuiElement.h"
#include "gui/Window.h"
#include "gui/Slider.h"
#include "instruments/Instrument.h"

#include <unordered_map>
#include <optional>
#include <vector>

namespace
{
	std::vector<SampleGenerator<void*>::after_t> afterEffects;
	std::unordered_map<std::string, std::optional<unsigned>> settings = {
		{"sampleRate", 44100},
		{"bufferSize", 64},
		{"maxNoteCount", 5},
	};

	void addAfterEffects(std::shared_ptr<Window> mainWindow)
	{
		if (afterEffects.size()) {
			throw std::logic_error("This function should not be called more than once.");
		}

		auto gui = mainWindow->getContentFrame();
		auto menu = mainWindow->getMenuFrame();
		using pos_t = MenuOption::OptionList::ChildPos_t;

		auto volume = VolumeControl();
		gui->addChildAutoPos(volume.getFrame());

		auto delay = DelayEffect(settings["sampleRate"].value(), 1., 0.6);
		auto delayWindow = std::make_shared<Window>(delay.getFrame());
		delayWindow->setHeader(30, "Delay");
		delayWindow->setVisibility(false);
		gui->addChildAutoPos(delayWindow);

		auto debugEffect = DebugEffect();
		auto debugWindow = std::make_shared<Window>(debugEffect.getFrame());
		debugWindow->setHeader(30, "Debug");
		debugWindow->setVisibility(false);
		gui->addChildAutoPos(debugWindow);

		auto configFrame = std::make_shared<Frame>();
		configFrame->setBgColor(sf::Color::Black);
		configFrame->setChildAlignment(15);
		configFrame->addChildAutoPos(volume.getConfigFrame());
		configFrame->addChildAutoPos(delay.getConfigFrame());
		configFrame->fitToChildren();
		auto configWindow = std::make_shared<Window>(configFrame);
		configWindow->setHeader(30, "Effect config");
		configWindow->setVisibility(false);
		gui->addChildAutoPos(configWindow);

		menu->addChildAutoPos(MenuOption::createMenu(
			100, 30, 15, {
				"View", pos_t::Down, {{
					"Debug", debugWindow}, {
					"Effects", {{
						"Delay", delayWindow},
					}},
					{"Settings", configWindow}
				}
			}
		));

		afterEffects.push_back(delay);
		afterEffects.push_back(volume);
		afterEffects.push_back(debugEffect);
	}	

	template<class T>
	void attachAfterCallbacks(T& inst)
	{
		for (auto callback : afterEffects) {
			inst.getGenerator().addAfterCallback(callback);
		}
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
	addAfterEffects(mainWindow);

	static Instrument1 inst{
		settings["sampleRate"].value(),
		settings["bufferSize"].value(),
		Sine13,
		generateNotes(2, 5),
		settings["maxNoteCount"].value()
	};

	attachAfterCallbacks(inst);
	gui->addChild(inst.getGuiElement(), 50, 50);
	inst.getGuiElement()->focus();
	inst.play();
}
