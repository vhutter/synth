#ifndef SLIDER_H_INCLUDED
#define SLIDER_H_INCLUDED

#include <atomic>

#include "GuiElement.h"
#include "TextDisplay.h"

class Slider : public GuiElement
{
public:
	enum Orientation : bool { Vertical = true, Horizontal = false };

	Slider(const std::string& name, double from, double to, SynthFloat px, SynthFloat py, SynthFloat sx, SynthFloat sy, unsigned titleSize, Orientation ori, std::function<void()> onMove = {});
	Slider(const std::string& name, double from, double to, SynthFloat px, SynthFloat py, SynthFloat sx, SynthFloat sy, unsigned titleSize, Orientation ori, std::atomic<double>& val);
	Slider& setFixed(bool val) { fixed = val; return *this; }
	virtual bool needsEvent(const SynthEvent& event) const override;

	template<class T = std::function<void()>>
	static std::unique_ptr<Slider> DefaultSlider(const std::string& name, double from, double to, SynthFloat px, SynthFloat py, T&& onMoveVal = {})
	{
		constexpr SynthFloat width = 30;
		constexpr SynthFloat height = 100;
		constexpr unsigned titleSize = 16;

		if constexpr (std::is_constructible_v<std::function<void(Slider&)>, T>)
		{
			std::unique_ptr<Slider> ptr = std::make_unique<Slider>(name, from, to, px, py, width, height, titleSize, Slider::Vertical, []() {});
			ptr->onMove = [onMoveVal, ptr = ptr.get()]() {
				onMoveVal(*ptr);
			};
			return ptr;
		}
		else {
			return std::make_unique<Slider>(name, from, to, px, py, width, height, titleSize, Slider::Vertical, onMoveVal);
		}
	}

	virtual SynthRect AABB() const override;

	double getValue() const { return value; }

private:
	virtual void drawImpl(sf::RenderTarget& target, sf::RenderStates states) const override;
	virtual void onSfmlEvent(const sf::Event& event) override;
	void moveSlider(const SynthVec2& p);
	bool containsPoint(const SynthVec2& p) const;
	void refreshText();


	TextDisplay title;
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