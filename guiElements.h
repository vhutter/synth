#ifndef GUI_ELEMENTS_H_INCLUDED
#define GUI_ELEMENTS_H_INCLUDED

#include <SFML/Graphics.hpp>
#include <vector>
#include <functional>
#include <atomic>
#include <variant>
#include <memory>

#include "utility.h"
#include "events.h"

class SynthKey : public sf::RectangleShape
{
public:
    enum Type : bool { White = true, Black = false };
	enum State : bool { Pressed = true, Released = false };

    SynthKey(Type t, SynthFloat px=0, SynthFloat py=0, const SynthVec2& size = SynthVec2(0,0));
    Type getType() const {return type;};
    void setPressed(bool pressed);
    bool isPressed() const;

    static const SynthVec2 whiteSizeDefault, blackSizeDefault;

private:
    bool pressed = std::atomic<bool>(false);
    Type type;

};

class GuiElement: public sf::Drawable, public sf::Transformable
{
public:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	virtual SynthRect AABB() const;
	void moveAroundPoint(const SynthVec2& center);
	void forwardEvent(const SynthEvent& event, const sf::Transform& transform = {});
	void addChildren(const std::vector<std::shared_ptr<GuiElement>>& child);
	void removeChild(const std::shared_ptr<GuiElement>& child);

protected:
	virtual void drawImpl(sf::RenderTarget& target, sf::RenderStates states) const = 0;
	virtual void onEvent(const SynthEvent& event) {}

	std::vector<std::shared_ptr<GuiElement>> children;

	// Used for event handling
	sf::Transform globalTransform;
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

    SynthKeyboard(SynthFloat px, SynthFloat py, callback_t eventCallback);

    virtual void onEvent(const SynthEvent& event) override;
	virtual SynthRect AABB() const override;

    SynthKey& operator[] (std::size_t i) {return keys[i];}

private:
	void onMidiEvent(const MidiEvent& event);
	void onSfmlEvent(const sf::Event& event);
    virtual void drawImpl(sf::RenderTarget& target, sf::RenderStates states) const override;
    void repositionKeys();

    std::vector<SynthKey> keys;
	callback_t onKey;
	const SynthVec2 blackSize{ SynthKey::blackSizeDefault }, whiteSize{ SynthKey::whiteSizeDefault };
};

class Window : public GuiElement
{
public:
	Window(SynthFloat px, SynthFloat py, SynthFloat sx, SynthFloat sy, const sf::Color& fillColor);
	void setSize(const SynthVec2& size);

	virtual SynthRect AABB() const override;
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	virtual void drawImpl(sf::RenderTarget& target, sf::RenderStates states) const override;

	sf::RectangleShape mainRect;
	SynthFloat childAlignment{ 10 }, cursorX{ 0 }, cursorY{ 0 };
};

class TextDisplay : public GuiElement
{
public:
    TextDisplay(const std::string& initialText, SynthFloat px, SynthFloat py, SynthFloat sx=0, SynthFloat sy=0, unsigned int charSize=30);
	static std::unique_ptr<TextDisplay> DefaultText(
		const std::string& initialText, 
		SynthFloat px, SynthFloat py,
		unsigned int charSize = 30
	);
    static std::unique_ptr<TextDisplay> Multiline(
		const std::string text, 
		SynthFloat px, SynthFloat py, 
		SynthFloat width,
		unsigned int charSize=24
	);

    const std::string& getText() const {return content;}
	const sf::Color& getBgColor() const;

    void setText(const std::string& text);
    void setBgColor(const sf::Color& color);

	virtual SynthRect AABB() const override;
	void centralize();
	void setFixedSize(bool fixed);
	void setFrameSize(const SynthVec2& size);
	void fitFrame(const SynthVec2& size = {0,0});

protected:
	virtual void drawImpl(sf::RenderTarget& target, sf::RenderStates states) const override;
	const SynthVec2 topLeftAlignment() const;

    sf::RectangleShape frame;
    sf::Text text;
    std::string content;
	bool fixedFrame{ false };

    static sf::Font font;
};

class Button : public TextDisplay
{
public:
    Button(const std::string& initialText, SynthFloat px, SynthFloat py, SynthFloat sx, SynthFloat sy, unsigned int charSize, std::function<void()> onClick);

    static std::unique_ptr<Button> DefaultButton(const std::string& s, SynthFloat px, SynthFloat py, std::function<void()> onClick) {
        return std::make_unique<Button>(s, px, py, 100, 30, 16, onClick);
    }
    static std::unique_ptr<Button> DefaultButton(const std::string& s, SynthFloat px, SynthFloat py, std::atomic<bool>& val) {
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

	Slider(const std::string& name, double from, double to, SynthFloat px, SynthFloat py, SynthFloat sx, SynthFloat sy, unsigned titleSize, Orientation ori, std::function<void()> onMove = {});
    Slider(const std::string& name, double from, double to, SynthFloat px, SynthFloat py, SynthFloat sx, SynthFloat sy, unsigned titleSize, Orientation ori, std::atomic<double>& val);
    Slider& setFixed(bool val) {fixed = val; return *this;}

	template<class T = std::function<void()>>
	static std::unique_ptr<Slider> DefaultSlider(const std::string& name, double from, double to, SynthFloat px, SynthFloat py, T&& onMoveVal = {})
	{
		constexpr SynthFloat width = 30;
		constexpr SynthFloat height = 100;
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
	virtual SynthRect AABB() const override;

    double getValue() const {return value;}

private:
	virtual void drawImpl(sf::RenderTarget& target, sf::RenderStates states) const override;
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

class Oscilloscope : public GuiElement
{
public:
    Oscilloscope(SynthFloat px, SynthFloat py, SynthFloat sx, SynthFloat sy, unsigned resolution, double speed);

	virtual SynthRect AABB() const override;

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
