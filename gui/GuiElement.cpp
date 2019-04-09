#include "GuiElement.h"

EmptyGuiElement::EmptyGuiElement(const sfmlCallback_t& sfml, const midiCallback_t& midi)
	:sfmlCallback(sfml),
	midiCallback(midi)
{}

// The return value indicates if the mouse event was used
bool GuiElement::forwardEvent(const SynthEvent& event, const sf::Transform& transform)
{
	if (!visible)
		return false;
	bool ret = false;
	if (auto e = std::get_if<sf::Event>(&event)) {
		if (e->type == sf::Event::MouseButtonPressed && !dynamic_cast<EmptyGuiElement*>(this)) {
			const auto& aabb = transform.transformRect(sf::FloatRect(AABB()));
			if (!aabb.contains(sf::Vector2f(e->mouseButton.x, e->mouseButton.y))) {
				return false;
			}
			else ret = true;
		}
	}
	globalTransform = getTransform() * transform;
	if (needsEvent(event)) {
		if (forwardsEvent(event)) {
			auto clickedChild = children.end();
			for (auto child = children.rbegin(); child != children.rend(); ++child) {
				if ((*child)->forwardEvent(event, globalTransform)) {
					clickedChild = child.base();
					std::advance(clickedChild, -1);
					break;
				}
			}
			if (clickedChild != children.end() && (*clickedChild)->dynamic) {
				// set the element in focus
				auto pElem = *clickedChild;
				children.erase(clickedChild);
				children.push_back(pElem);
			}
		}
		onEvent(event);
	}
	return ret;
}

void GuiElement::addChildren(const std::vector<std::shared_ptr<GuiElement>>& newChildren)
{
	children.insert(children.end(), newChildren.begin(), newChildren.end());
}

void GuiElement::removeChild(const std::shared_ptr<GuiElement>& child)
{
	auto found = std::find(children.begin(), children.end(), child);
	if (found != children.end()) {
		children.erase(found);
	}
}

void GuiElement::onEvent(const SynthEvent & eventArg)
{
	if (std::holds_alternative<sf::Event>(eventArg)) {
		onSfmlEvent(std::get<sf::Event>(eventArg));
	}
	else {
		onMidiEvent(std::get<MidiEvent>(eventArg));
	}
}

void GuiElement::setVisibility(bool v)
{
	visible = v;
}

void GuiElement::setDynamic(bool d)
{
	dynamic = d;
}

void GuiElement::moveAroundPoint(const SynthVec2 & center)
{
	const auto& bounds = AABB();
	auto newPos = center - SynthVec2(bounds.width, bounds.height) / 2.;
	setPosition(std::round(newPos.x), std::round(newPos.y));
}

void GuiElement::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	if (visible) {
		states.transform *= getTransform();
		drawImpl(target, states);
		const auto savedView = target.getView();
		target.setView(childrenView(target, states));
		for (auto& child : children) {
			child->draw(target, states);
		}
		target.setView(savedView);
	}
}

SynthRect GuiElement::AABB() const
{
	return SynthRect();
}