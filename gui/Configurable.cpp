#include "Configurable.h"

std::shared_ptr<Frame> Configurable::getConfigFrame() const
{
	if (!frame) throw std::runtime_error("Config frame is not set up.");
	return frame;
}

std::shared_ptr<EmptyGuiElement> Configurable::getListener() const
{
	if (!listener) throw std::runtime_error("Listener is not set up.");
	return listener;
}

void Configurable::setupConfig(
	const std::string& name,
	InputRecord::Type type,
	std::function<void(const SynthEvent&)> onEnd
)
{
	if (!frame) {
		frame = std::make_shared<Frame>();
		listener = std::make_shared<EmptyGuiElement>();
		frame->addChild(listener);
		frame->setSize(SynthVec2(1000, 100));
		auto input = std::make_shared<InputRecord>(type, 200, 30);
		input->setOnEnd([input, onEnd]() {
			onEnd(input->getLastEvent());
			});
		input->setText(std::string("<Empty>"));
		input->centralize();
		auto title = std::make_shared<TextDisplay>(name, 0, 30, 13);
		title->centralize();
		frame->addChildAutoPos(title);
		frame->addChildAutoPos(input);
		frame->setBgColor(sf::Color::Black);
		frame->fitToChildren();
	}
}