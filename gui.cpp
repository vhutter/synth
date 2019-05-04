#include "gui.h"
#include "gui/GuiElement.h"
#include "gui/Window.h"
#include "gui/Slider.h"
#include "instruments/Instrument.h"

#include <unordered_map>
#include <optional>
#include <vector>
#include <numeric>


namespace
{
	using pos_t = MenuOption::OptionList::ChildPos_t;

	std::vector<std::function<void(double, double&)>> afterEffects;
	std::unordered_map<std::string, std::optional<unsigned>> settings = {
		{"sampleRate", 44100},
		{"bufferSize", 64},
		{"maxNoteCount", 5},
	};

	Instrument1 inst1{
		"Classic1",
		Sines1(),
		ADSREnvelope(),
		generateNotes(2, 5),
		settings["maxNoteCount"].value()
	};
	
	Instrument1 inst2{
		"Soft bass",
		Sines2(),
		ADSREnvelope(),
		generateNotes(1, 3),
		settings["maxNoteCount"].value()
	};
	
	//Instrument1 inst3{
	//	"Synth 2",
	//	SinesTriangles(),
	//	ADSREnvelope(0.5, 0.2, 0.5, 1., 0.5),
	//	generateNotes(2, 5),
	//	settings["maxNoteCount"].value()
	//};
	//
	//Instrument1 inst4{
	//	"Sine",
	//	Sine(),
	//	ADSREnvelope(),
	//	generateNotes(2, 5),
	//	settings["maxNoteCount"].value()
	//};
	//
	//Instrument1 inst5{
	//	"Square",
	//	Square(),
	//	ADSREnvelope(),
	//	generateNotes(2, 5),
	//	settings["maxNoteCount"].value()
	//};
	//
	//Instrument1 inst6{
	//	"Sawtooth",
	//	Saw(),
	//	ADSREnvelope(),
	//	generateNotes(2, 5),
	//	settings["maxNoteCount"].value()
	//};
	//
	//Instrument1 inst7{
	//	"Triangle",
	//	Triangle(),
	//	ADSREnvelope(),
	//	generateNotes(2, 5),
	//	settings["maxNoteCount"].value()
	//};
	//
	auto instruments = std::forward_as_tuple(inst1, inst2);// , inst3, inst4, inst5, inst6, inst7);

	SumGenerator generator(
		[](double t, double& sample) {
			for (auto f : afterEffects)
				f(t, sample);
		},
		instruments
	);

	SynthStream synthStream{
		settings["sampleRate"].value(),
		settings["bufferSize"].value(),
		[](double t) -> double { 
			return generator.getSample(t);
		},
		[](double t) -> double {
			return generator.getSample(t);
		},
	};

	void addAfterEffects(std::shared_ptr<Window> mainWindow)
	{
		if (afterEffects.size()) {
			throw std::logic_error("This function should not be called more than once.");
		}

		auto gui = mainWindow->getContentFrame();
		auto menu = mainWindow->getMenuFrame();

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
			30, 15, {
				"View", pos_t::Down, {{
					"Debug", debugWindow}, {
					"Effects", {{
						"Delay", delayWindow},
					}},
					{"Input settings", configWindow}
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
			inst.addAfterCallback(callback);
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

	auto toVector = [](auto & tuple) {
		auto toList = [](auto & instrument) -> MenuOption::OptionList {
			return { instrument.getTitle(), instrument.getGuiElement() };
		};

		return std::apply([toList](auto & ... args) -> std::vector<MenuOption::OptionList> {
			return { toList(args)... };
		}, tuple);
	};

	menu->addChildAutoPos(MenuOption::createMenu(
		30, 15, {
			"Instruments", pos_t::Down, toVector(instruments)
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
	}, instruments);

	addAfterEffects(mainWindow);
	synthStream.play();
}
