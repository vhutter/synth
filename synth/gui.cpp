#include "gui.h"
#include "gui/GuiElement.h"
#include "gui/Window.h"
#include "gui/Slider.h"
#include "core/Instrument.h"

#include <unordered_map>
#include <optional>
#include <vector>
#include <numeric>


namespace
{
	using pos_t = MenuOption::OptionList::ChildPos_t;

	std::vector<std::function<void(double, double&)>> afterEffects;

	auto& getInputInstrument()
	{
		static InputInstrument inst3{
			"Microphone input"
		};
		return inst3;
	}

	auto& getInstruments()
	{
		static KeyboardInstrument inst1{
			"Synth 1",
			Sines1(),
			ADSREnvelope(),
			generateNotes(2, 6),
			getConfig("maxNoteCount")
		};

		static KeyboardInstrument inst2{
			"Soft bass",
			Sines2(),
			ADSREnvelope(),
			generateNotes(1, 3),
			getConfig("maxNoteCount")
		};
		static auto& inst3 = getInputInstrument();

		static KeyboardInstrument inst4{
			"Synth 2",
			SinesTriangles(),
			ADSREnvelope(0.5, 0.2, 0.5, 1., 0.5),
			generateNotes(2, 5),
			getConfig("maxNoteCount")
		};

		static auto instruments = std::forward_as_tuple(inst1, inst2, inst3);
		return instruments;
	}

	auto& getSynth()
	{
		static SumGenerator generator(
			[](double t, double& sample) {
				for (auto f : afterEffects)
					f(t, sample);
			},
			getInstruments()
		);
		static SynthStream synthStream{
			getConfig("sampleRate"),
			getConfig("bufferSize"),
			[](double t) -> double {
				return generator.getSample(t);
			},
			[](double t) -> double {
				return generator.getSample(t);
			},
			[](double t) {
				getInputInstrument()(t);
			}
		};
		return synthStream;
	}

	void addAfterEffects(std::shared_ptr<Window> mainWindow)
	{
		if (afterEffects.size()) {
			throw std::logic_error("This function should not be called more than once.");
		}

		auto gui = mainWindow->getContentFrame();
		auto menu = mainWindow->getMenuFrame();

		auto volume = VolumeControl();
		gui->addChildAutoPos(volume.getFrame());

		auto delay = DelayEffect(getConfig("sampleRate"), 1., 0.6);
		auto delayWindow = std::make_shared<Window>(delay.getFrame());
		delayWindow->setHeader(getConfig("defaultHeaderSize"), "Delay");
		delayWindow->setVisibility(false);
		gui->addChildAutoPos(delayWindow);

		auto debugEffect = DebugEffect();
		auto debugWindow = std::make_shared<Window>(debugEffect.getFrame());
		debugWindow->setHeader(getConfig("defaultHeaderSize"), "Debug");
		debugWindow->setVisibility(false);
		gui->addChildAutoPos(debugWindow);

		auto saveEffect = SaveToFile("Test.wav", getConfig("sampleRate"), 2);
		auto saveWindow = std::make_shared<Window>(saveEffect.getFrame());
		saveWindow->setHeader(getConfig("defaultHeaderSize"), "Record");
		saveWindow->setVisibility(false);
		gui->addChildAutoPos(saveWindow);

		auto configFrame = std::make_shared<Frame>();
		configFrame->setBgColor(sf::Color::Black);
		configFrame->setChildAlignment(15);
		configFrame->addChildAutoPos(volume.getConfigFrame());
		configFrame->addChildAutoPos(delay.getConfigFrame());
		configFrame->addChildAutoPos(debugEffect.getConfigFrame());
		configFrame->addChildAutoPos(saveEffect.getConfigFrame());
		configFrame->fitToChildren();
		auto configWindow = std::make_shared<Window>(configFrame);
		configWindow->setHeader(getConfig("defaultHeaderSize"), "Input settings");
		configWindow->setVisibility(false);
		gui->addChildAutoPos(configWindow);

		menu->addChildAutoPos(MenuOption::createMenu(
			getConfig("defaultHeaderSize"), 15, {
				"View", pos_t::Down, {{
					"Debug", debugWindow}, {
					"Effects", {{
						"Delay", delayWindow},
					}},
					{"Input settings", configWindow},
					{"Record", saveWindow},
				}
			}
		));

		afterEffects.push_back(delay);
		afterEffects.push_back(volume);
		afterEffects.push_back(debugEffect);
		afterEffects.push_back(saveEffect);
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
			case sf::Event::Resized: {
				sf::View view(sf::FloatRect(0, 0, event.size.width, event.size.height));
				renderWindow.setView(view);
				mainWindow->setSize(SynthVec2(event.size.width, event.size.height));
				break;
			}
			default:
				break;
		}
	});

	mainWindow->addEmptyListener(setup);

	auto gui = mainWindow->getContentFrame();
	auto menu = mainWindow->getMenuFrame();

	auto toVector = [](auto & tuple) {
		auto toList = [](auto & instrument) -> MenuOption::OptionList {
			return { instrument.getTitle(), instrument.getGuiElement() };
		};

		return std::apply([toList](auto & ... args) -> std::vector<MenuOption::OptionList> {
			return { toList(args)... };
		}, tuple);
	};

	menu->addChildAutoPos(MenuOption::createMenu(
		getConfig("defaultHeaderSize"), 15, {
			"Instruments", pos_t::Down, toVector(getInstruments())
		}
	));


	std::apply([gui](auto && ... args) {
		(
			(
				gui->addChild(args.getGuiElement(), 50, 50),
				args.getGuiElement()->setVisibility(false)
			),
			...
		);
	}, getInstruments());

	addAfterEffects(mainWindow);
	getSynth().play();
}
