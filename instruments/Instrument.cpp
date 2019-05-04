#include "Instrument.h"

void Instrument1::init()
{
	using pos_t = MenuOption::OptionList::ChildPos_t;
	auto gui = window->getContentFrame();
	gui->setChildAlignment(10);
	gui->setCursor(10, 10);

	keyboard.outputTo(
		generator,
		glider
	);

	gui->addChildAutoPos(pitchBender.getFrame());
	gui->addChildAutoPos(glider.getFrame());
	gui->newLine();

	auto inputConfigFrame = std::make_shared<Frame>(0,0);
	inputConfigFrame->setBgColor(sf::Color::Black);

	inputConfigFrame->addChildAutoPos(pitchBender.getConfigFrame());
	inputConfigFrame->addChildAutoPos(glider.getConfigFrame());

	const auto& timbre = generator.getTimbreModel();
	for (unsigned i = 0; i < timbre.components.size(); ++i) {
		auto cFrame = std::make_shared<Frame>();
		auto cSlider = std::shared_ptr(Slider::DefaultSlider("Component" + std::to_string(i), 0, 1, [this, i](const Slider& slider) {
			std::lock_guard lock(generator);
			for (std::size_t j = 0; j < generator.size(); ++j) {
				auto& g = generator[j];
				g[i].intensity = slider.getValue();
			}
		}));
		cSlider->setValue(timbre.components[i].intensity);
		
		auto cValueInput = std::make_shared<InputField>(InputField::Double, 100, 30, 16);
		cValueInput->setTextCentered(std::to_string(timbre.components[i].relativeFreq));
		cValueInput->setOnEnd([this, cValueInput, i]() {
			std::string valStr = cValueInput->getText();
			double val;
			if (i == 0) {
				cValueInput->setTextCentered(std::string("Base freq"));
			}
			else try {
				val = std::stod(valStr);
				std::lock_guard lock(generator);
				for (std::size_t j = 0; j < generator.size(); ++j) {
					auto& g = generator[j];
					g[i].modifyMainPitch(generator.time(), g[0].getMainFreq() * val);
				}
			}
			catch (...) {}
		});

		cFrame->setChildAlignment(2);
		cFrame->addChildAutoPos(cSlider);
		cFrame->addChildAutoPos(cValueInput);
		cFrame->fitToChildren();

		gui->addChildAutoPos(cFrame);
		inputConfigFrame->addChildAutoPos(cSlider->getConfigFrame());
	}

	generator.addAfterCallback(glider);

	auto kbAABB = keyboard.getSynthKeyboard()->AABB();
	gui->newLine();
	gui->addChild(keyboard.getSynthKeyboard(), 0, wHeight - kbAABB.height);
	gui->addChildAutoPos(keyboard.getSynthKeyboard());
	gui->fitToChildren();

	window->addEmptyListener(std::make_unique<EmptyGuiElement>([this](const sf::Event & event) {
		if (event.type == sf::Event::KeyPressed) {

			auto synthKeyboard = keyboard.getSynthKeyboard();
			unsigned shift = synthKeyboard->getOctaveShift();
			switch(event.key.code) {
				case sf::Keyboard::Up:
					keyboard.stopAll();
					++shift;
					break;
				case sf::Keyboard::Down:
					keyboard.stopAll();
					--shift;
					break;
				default:
					break;
			};
			synthKeyboard->setOctaveShift(shift);
		}
	}));

	inputConfigFrame->fitToChildren();
	auto inputConfigWindow = std::make_shared<Window>(inputConfigFrame);
	inputConfigWindow->setHeader(30, "Input config");
	inputConfigWindow->setVisibility(false);
	gui->addChild(inputConfigWindow, 100, 100);

	gui->fitToChildren();
	window->setSize(SynthVec2(gui->getSize()));
	window->setMenuBar(menuHeight);
	window->setOnClose([this]() {keyboard.stopAll(); });
	window->getMenuFrame()->addChildAutoPos(MenuOption::createMenu(
		30, 15, {
			"View", pos_t::Down, {
				{"Input settings", inputConfigWindow}
			}
		}
	));
}
