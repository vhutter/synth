#include "Instrument.h"
#include "utility.h"

Instrument::Instrument(const std::string& title)
	:title(title),
	window{ std::make_shared<Window>(wWidth, wHeight) }
{
	window->setHeader(getConfig("defaultHeaderSize"), title);
}


const std::string& Instrument::getTitle() const 
{ 
	return title; 
}

std::shared_ptr<Window> Instrument::getGuiElement() const 
{ 
	return window; 
}

KeyboardInstrument::KeyboardInstrument(
	const std::string& title,
	const TimbreModel& timbreModel,
	const ADSREnvelope& env,
	const std::vector<Note>& notes,
	unsigned maxTones
)
	:Instrument(title),
	generator{ timbreModel, env, notes, maxTones },
	keyboard{ generator.getNotesCount() },
	pitchBender{ generator },
	glider{ generator.getTimbreModel(), generator.getNotes(), generator.getMaxTones() }
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
				g[i].modifyIntensity(generator.time(), slider.getValue());
			}
		}));
		cSlider->setValue(timbre.components[i].intensity);
		
		auto cValueInput = std::make_shared<InputField>(InputField::Double, 100, getConfig("defaultTextHeight"));
		cValueInput->setTextCentered(std::to_string(timbre.components[i].relativeFreq));
		cValueInput->setOnEnd([this, cValueInput, i, &timbre]() {
			std::string valStr = cValueInput->getText();
			double val;
			if (i == 0) {
				cValueInput->setTextCentered(std::string("Base freq"));
			}
			else try {
				val = std::stod(valStr);
				if (val == 0) {
					throw std::runtime_error("0 is not allowed for this input");
				}
				std::lock_guard lock(generator);
				for (std::size_t j = 0; j < generator.size(); ++j) {
					auto& g = generator[j];
					g[i].modifyMainPitch(generator.time(), g[0].getMainFreq() * val);
				}
			}
			catch (...) {
				cValueInput->setTextCentered(std::to_string(timbre.components[i].relativeFreq));
			}
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
	inputConfigWindow->setHeader(getConfig("defaultHeaderSize"), "Input config");
	inputConfigWindow->setVisibility(false);
	gui->addChild(inputConfigWindow, 100, 100);

	gui->fitToChildren();
	window->setSize(SynthVec2(gui->getSize()));
	window->setMenuBar(menuHeight);
	window->setOnClose([this]() {keyboard.stopAll(); generator.releaseKeys(); });
	window->getMenuFrame()->addChildAutoPos(MenuOption::createMenu(
		getConfig("defaultHeaderSize"), 15, {
			"View", pos_t::Down, {
				{"Input settings", inputConfigWindow}
			}
		}
	));
}

InputInstrument::InputInstrument(const std::string& title)
	:Instrument(title)
{
	auto gui = window->getContentFrame();
	gui->addChildAutoPos(button);
	window->setSize(SynthVec2(300, 50));
}

InputInstrument::GeneratorProxy& InputInstrument::getGenerator()
{
	return generator;
}

void InputInstrument::GeneratorProxy::feedSample(const double& sample)
{
	this->sample = sample;
}

double InputInstrument::GeneratorProxy::getSampleImpl(double t) const
{
	return sample;
}

void InputInstrument::operator()(const double& sample)
{
	generator.feedSample(sample * isOn);
}