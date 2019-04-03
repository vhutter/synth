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

    SynthKey(Type t, float px=0, float py=0, const sf::Vector2f& size = sf::Vector2f(0,0));
    Type getType() const {return type;};
    void setPressed(bool pressed);
    bool isPressed() const;

    static const sf::Vector2f whiteSizeDefault, blackSizeDefault;

private:
    bool pressed = std::atomic<bool>(false);
    Type type;

};

class GuiElement: public sf::Drawable, public sf::Transformable
{
public:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const final override;
	virtual sf::FloatRect AABB();
	virtual void moveAroundPoint(const sf::Vector2f& center);
	void forwardEvent(const SynthEvent& event);
	void addChildren(const std::vector<std::shared_ptr<GuiElement>>& child);
	void removeChild(const std::shared_ptr<GuiElement>& child);

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
	virtual sf::FloatRect AABB() override;

    SynthKey& operator[] (std::size_t i) {return keys[i];}

private:
	void onMidiEvent(const MidiEvent& event);
	void onSfmlEvent(const sf::Event& event);
    virtual void drawImpl(sf::RenderTarget& target, sf::RenderStates states) const override;
    void repositionKeys();

    std::vector<SynthKey> keys;
	callback_t onKey;
	const sf::Vector2f blackSize{ SynthKey::blackSizeDefault }, whiteSize{ SynthKey::whiteSizeDefault };
};

class Window : public GuiElement
{
public:
	Window(float px, float py, float sx, float sy, const sf::Color& fillColor);
	void addChildren(const std::vector<std::shared_ptr<GuiElement>>& child);
	void setSize(const sf::Vector2f& size);

	virtual sf::FloatRect AABB() override;

private:
	virtual void drawImpl(sf::RenderTarget& target, sf::RenderStates states) const override;

	sf::RectangleShape mainRect;
	float childAlignment{ 10 }, cursorX{ 0 }, cursorY{ 0 };
};

class TextDisplay : public GuiElement
{
public:
    TextDisplay(const std::string& initialText, float px, float py, float sx=0, float sy=0, unsigned int charSize=30);
	static std::unique_ptr<TextDisplay> DefaultText(
		const std::string& initialText, 
		float px, float py,
		unsigned int charSize = 30
	);
    static std::unique_ptr<TextDisplay> Multiline(
		const std::string text, 
		float px, float py, 
		float width,
		unsigned int charSize=24
	);

    const std::string& getText() const {return content;}
	const sf::Color& getBgColor() const;

    void setText(const std::string& text);
    void setBgColor(const sf::Color& color);

	virtual sf::FloatRect AABB() override;
	void centralize();
	void setFixedSize(bool fixed);
	void setFrameSize(const sf::Vector2f& size);
	void fitFrame(const sf::Vector2f& size = {0,0});

protected:
	virtual void drawImpl(sf::RenderTarget& target, sf::RenderStates states) const override;
	const sf::Vector2f topLeftAlignment() const;

    sf::RectangleShape frame;
    sf::Text text;
    std::string content;
	bool fixedFrame{ false };

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
	virtual sf::FloatRect AABB() override;
    sf::Vector2f getPosition() const {return mainRect.getPosition();}
    void setPosition(const sf::Vector2f& p) {mainRect.setPosition(p);}

    double getValue() const {return value;}

private:
	virtual void drawImpl(sf::RenderTarget& target, sf::RenderStates states) const override;
    void moveSlider(const sf::Vector2f& p);
    bool containsPoint(const sf::Vector2f& p) const;
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

    sf::Vector2f size;
    sf::Vector2f sliderRectSize;
};

class Oscilloscope : public GuiElement
{
public:
    Oscilloscope(float px, float py, float sx, float sy, unsigned resolution, double speed);

	virtual sf::FloatRect AABB() override;

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
