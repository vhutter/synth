#ifndef SLIDER_H_INCLUDED
#define SLIDER_H_INCLUDED

#include <atomic>

#include "GuiElement.h"
#include "Configurable.h"
#include "../core/generators.h"

class TextDisplay;

class Slider : public GuiElement, public Configurable
{
public:
	enum Orientation : bool { Vertical = true, Horizontal = false };

	Slider(
		const std::string& name, 
		double from, double to, 
		SynthFloat sx, SynthFloat sy, 
		unsigned titleSize, 
		Orientation ori, 
		std::function<void()> onMove = {}
	);
	Slider(
		const std::string& name, 
		double from, double to, 
		SynthFloat sx, SynthFloat sy, 
		unsigned titleSize, 
		Orientation ori, 
		std::atomic<double>& val
	);
	Slider& setFixed(bool val) { fixed = val; return *this; }
	virtual bool needsEvent(const SynthEvent& event) const override;

	template<class T = std::function<void()>>
	static std::unique_ptr<Slider> DefaultSlider(const std::string& name, double from, double to, T&& onMoveVal = {})
	{
		const SynthFloat width = getConfig("defaultSliderWidth");
		const SynthFloat height = getConfig("defaultSliderHeight");
		const unsigned titleSize = getConfig("defaultCharSize");

		if constexpr (std::is_constructible_v<std::function<void(const Slider&)>, T>)
		{
			std::unique_ptr<Slider> ptr = std::make_unique<Slider>(
				name, 
				from, to, 
				width, height, 
				titleSize, 
				Slider::Vertical, 
				[]() {}
			);
			ptr->onMove = [onMoveVal, ptr = ptr.get()]() {
				onMoveVal(*ptr);
			};
			return ptr;
		}
		else {
			return std::make_unique<Slider>(
				name, 
				from, to, 
				width, height, 
				titleSize, 
				Slider::Vertical, 
				onMoveVal
			);
		}
	}

	virtual SynthRect AABB() const override;

	double getValue() const { return value; }
	void setValue(double newVal);

private:
	virtual void drawImpl(sf::RenderTarget& target, sf::RenderStates states) const override;
	virtual void onSfmlEvent(const sf::Event& event) override;
	void moveSlider(const SynthVec2& p);
	bool containsPoint(const SynthVec2& p) const;
	void refreshText();


	std::unique_ptr<TextDisplay> titleText, valueText;
	const double from, to;
	const std::string name;
	sf::RectangleShape mainRect, sliderRect;
	Orientation orientation;
	bool fixed = false;

	std::atomic<double> value;
	bool clicked = false;
	std::function<void()> onMove;

	SynthVec2 size;
	SynthVec2 sliderRectSize;
};

#endif //SLIDER_H_INCLUDED