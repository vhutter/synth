#ifndef GUI_ELEMENTS_H_INCLUDED
#define GUI_ELEMENTS_H_INCLUDED

#include <SFML/Graphics.hpp>
#include <vector>
#include <functional>
#include <atomic>
#include <variant>
#include <memory>

#include "events.h"

class SynthKey : public sf::RectangleShape
{
public:
    enum Type : bool { White = true, Black = false };
	enum State : bool { Pressed = true, Released = false };

    SynthKey(Type t, float px=0, float py=0);
    Type getType() const {return type;};
    void setPressed(bool pressed);
    bool isPressed() const;

    static const sf::Vector2f whiteSize, blackSize;

private:
    bool pressed = std::atomic<bool>(false);
    Type type;

};

class GuiElement: public sf::Drawable, public sf::Transformable
{
public:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const final override;
	void forwardEvent(const SynthEvent& event);
	void addChildren(std::vector<std::shared_ptr<GuiElement>> child);
	void removeChild(std::shared_ptr<GuiElement> child);

protected:
	virtual void drawImpl(sf::RenderTarget& target, sf::RenderStates states) const = 0;
	virtual void onEvent(const SynthEvent& event) {}

	std::vector<std::shared_ptr<GuiElement>> children;
};

class EmptyGuiElement : public GuiElement
{
	using callback_t = std::function<void(const SynthEvent&)>;

public:
	EmptyGuiElement(const callback_t& onEvt = {}) :callback(onEvt) {}

protected:
	virtual void drawImpl(sf::RenderTarget& target, sf::RenderStates states) const override {}
	virtual void onEvent(const SynthEvent& event) {
		if (callback) callback(event);
	}

	callback_t callback;
};

class SynthKeyboard : public GuiElement
{
public:
	using callback_t = std::function<void(unsigned, SynthKey::State)>;

    SynthKeyboard(float px, float py, callback_t eventCallback);

    virtual void onEvent(const SynthEvent& event) override;

    SynthKey& operator[] (std::size_t i) {return keys[i];}

private:
	void onMidiEvent(const MidiEvent& event);
	void onSfmlEvent(const sf::Event& event);
    virtual void drawImpl(sf::RenderTarget& target, sf::RenderStates states) const override;
    void repositionKeys();

    std::vector<SynthKey> keys;
	callback_t onKey;
};

class TextDisplay : public GuiElement
{
public:
    TextDisplay(const std::string& initialText, float px, float py, float sx=0, float sy=0, unsigned int charSize=24);
    static TextDisplay AroundPoint(const std::string text, float px, float py, float sx=0, float sy=0, unsigned int charSize=24);

    void setPosition(const sf::Vector2f& p) {window.setPosition(p);}
    sf::Vector2f getPosition() const {return window.getPosition();}
    const std::string& getText() {return content;}
    void setText(const std::string& text);
    const sf::Text& getText() const {return text;}
    void setBgColor(const sf::Color& color);
    const sf::Color& getBgColor() {return window.getFillColor();}

protected:
	virtual void drawImpl(sf::RenderTarget& target, sf::RenderStates states) const override;

    sf::RectangleShape window;
    sf::Text text;
    std::string content;

    static sf::Font font;
};

class Button : public TextDisplay
{
public:
    Button(const std::string& initialText, float px, float py, float sx, float sy, unsigned int charSize, std::function<void()> onClick);

    static std::unique_ptr<Button> DefaultButton(const std::string& s, float px, float py, std::function<void()> onClick) {
        return std::make_unique<Button>(s, px, py, 100, 30, 16, onClick);
    }
    static std::unique_ptr<Button> DefaultButton(const std::string& s, float px, float py, std::atomic<bool>& val) {
        return std::make_unique<Button>(s, px, py, 100, 30, 16, [&](){val = !val;});
    }

    virtual void onEvent(const SynthEvent& event) override;

private:
    std::function<void()> clickCallback;
};



class Slider : public GuiElement
{
public:
    enum Orientation : bool {Vertical = true, Horizontal = false};

	Slider(const std::string& name, double from, double to, float px, float py, float sx, float sy, unsigned titleSize, Orientation ori, std::function<void()> onMove = {});
    Slider(const std::string& name, double from, double to, float px, float py, float sx, float sy, unsigned titleSize, Orientation ori, std::atomic<double>& val);
    Slider& setFixed(bool val) {fixed = val; return *this;}

	template<class T = std::function<void()>>
	static std::unique_ptr<Slider> DefaultSlider(const std::string& name, double from, double to, float px, float py, T&& onMoveVal = {})
	{
		constexpr float width = 30;
		constexpr float height = 100;
		constexpr unsigned titleSize = 16;

		if constexpr (std::is_constructible_v<std::function<void(Slider&)>,T>)
		{
			std::unique_ptr<Slider> ptr = std::make_unique<Slider>(name, from, to, px, py, width, height, titleSize, Slider::Vertical, []() {});
			ptr->onMove = [onMoveVal, ptr=ptr.get()]() {
				onMoveVal(*ptr);
			};
			return ptr;
		}
		else {
			return std::make_unique<Slider>(name, from, to, px, py, width, height, titleSize, Slider::Vertical, onMoveVal);
		}
	}

    virtual void onEvent(const SynthEvent& event) override;
    sf::Vector2f getPosition() const {return mainRect.getPosition();}
    void setPosition(const sf::Vector2f& p) {mainRect.setPosition(p);}

    double getValue() const {return value;}

private:
	virtual void drawImpl(sf::RenderTarget& target, sf::RenderStates states) const override;
    void moveSlider(const sf::Vector2f& p);
    bool containsPoint(const sf::Vector2f& p) const;


    TextDisplay title;
    const double from, to;
    const std::string name;
    sf::RectangleShape mainRect, sliderRect;
    Orientation orientation;
    bool fixed = false;

    std::atomic<double> value;
    bool clicked = false;
    std::function<void()> onMove;

    sf::Vector2f size;
    sf::Vector2f sliderRectSize;
};

class Oscilloscope : public GuiElement
{
public:
    Oscilloscope(float px, float py, float sx, float sy, unsigned resolution, double speed);

    void setPosition(const sf::Vector2f& p) {window.setPosition(p);}
    sf::Vector2f getPosition() const {return window.getPosition();}
	unsigned getResolution() const { return resolution; }
	void newSamples(const std::vector<double>& samples) const;

private:
	virtual void drawImpl(sf::RenderTarget& target, sf::RenderStates states) const override;

    sf::RectangleShape window;
    mutable std::vector<sf::Vertex> vArray;
    const unsigned resolution;
    double speed;
    double currTime = 0;
};

#endif // GUI_ELEMENTS_H_INCLUDED
