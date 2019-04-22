#include "GuiElement.h"

EmptyGuiElement::EmptyGuiElement(const sfmlCallback_t& sfml, const midiCallback_t& midi)
	:sfmlCallback(sfml),
	midiCallback(midi)
{
	setFocusable(false);
}

// The return value indicates if the mouse click event was used
bool GuiElement::forwardEvent(const SynthEvent& event, const sf::Transform& transform)
{
	if (!visible)
		return false;
	bool ret = false;
	if (auto e = std::get_if<sf::Event>(&event)) {
		if (e->type == sf::Event::MouseButtonPressed && focusable) {
			const auto& aabb = transform.transformRect(sf::FloatRect(AABB()));
			if (aabb.contains(sf::Vector2f(e->mouseButton.x, e->mouseButton.y))) {
				ret = true;
			}
		}
	}
	globalTransform = getTransform() * transform;
	if (needsEvent(event)) {
		onEvent(event);
		if (forwardsEvent(event)) {
			for (auto child = children.rbegin(); child != children.rend(); ++child) {
				if ((*child)->forwardEvent(event, globalTransform)) {
					auto clickedChild = std::next(child).base();
					if ((*child)->focusable) {
						// set the element in focus
						auto pElem = *child;
						children.erase(clickedChild);
						children.push_back(pElem);
					}
					ret = true;
					break;
				}
			}
		}
	}
	return ret;
}

void GuiElement::addChild(std::shared_ptr<GuiElement> child, unsigned px, unsigned py)
{
	children.push_back(child);
	child->setPosition(px, py);
}

//void GuiElement::addChild(std::shared_ptr<GuiElement> child)
//{
//	children.push_back(child);
//}

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

void GuiElement::setFocusable(bool d)
{
	focusable = d;
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