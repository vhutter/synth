#ifndef CONFIGURABLE_H_INCLUDED
#define CONFIGURABLE_H_INCLUDED

#include "Input.h"

class Configurable
{
public:
	std::shared_ptr<Frame> getConfigFrame() const;

protected:
	void setupConfig(
		const std::string& name,
		InputRecord::Type type,
		std::function<void(const SynthEvent&)> onEnd
	);

	std::shared_ptr<EmptyGuiElement> getListener() const;

private:
	std::shared_ptr<Frame> frame;
	std::shared_ptr<EmptyGuiElement> listener;
};

#endif //CONFIGURABLE_H_INCLUDED