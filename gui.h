#ifndef GUI_H_INCLUDED
#define GUI_H_INCLUDED

#include <SFML/Graphics.hpp>
#include <vector>
#include <functional>
#include <atomic>
#include <memory>

class SynthKey : public sf::RectangleShape
{
public:
    enum Type : bool {White = true, Black = false};

    SynthKey(Type t, float px=0, float py=0);
    Type getType() const {return type;};
    void setPressed(bool pressed);
    bool isPressed() const;

    static const sf::Vector2f whiteSize, blackSize;

private:
    bool pressed = std::atomic<bool>(false);
    Type type;

};

class GuiElement: public sf::Drawable
{
public:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const = 0;
    virtual void forwardEvent(const sf::Event& event) {}
};

class SynthKeyboard : public GuiElement
{
public:
    SynthKeyboard(float px, float py, std::function<void(unsigned)> eventCallback);

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    virtual void forwardEvent(const sf::Event& event) override;
    void forwardEvent(const std::vector<unsigned char>& midiMessage);

    void setPosition(const sf::Vector2f& p);
    sf::Vector2f getPosition() const {return pos;};
    bool isLastEventKeypress() const {return lastPressed;}

    SynthKey& operator[] (std::size_t i) {return keys[i];}

private:
    void repositionKeys();

    sf::Vector2f pos;
    std::vector<SynthKey> keys;
    std::function<void(unsigned)> onKey;
    bool lastPressed = false;
};

class TextDisplay : public GuiElement
{
public:
    TextDisplay(const std::string& initialText, float px, float py, float sx=0, float sy=0, unsigned int charSize=24);
    static TextDisplay AroundPoint(const std::string text, float px, float py, float sx=0, float sy=0, unsigned int charSize=24);

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    void setPosition(const sf::Vector2f& p) {window.setPosition(p);}
    sf::Vector2f getPosition() const {return window.getPosition();}
    const std::string& getText() {return content;}
    void setText(const std::string& text);
    const sf::Text& getText() const {return text;}
    void setBgColor(const sf::Color& color);
    const sf::Color& getBgColor() {return window.getFillColor();}

protected:
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

    virtual void forwardEvent(const sf::Event& event) override;

private:
    std::function<void()> clickCallback;
};



class Slider : public GuiElement
{
public:
    enum Orientation : bool {Vertical = true, Horizontal = false};

    Slider(const std::string& name, double from, double to, float px, float py, float sx, float sy, unsigned titleSize, Orientation ori, std::function<void()> onMove);
    Slider(const std::string& name, double from, double to, float px, float py, float sx, float sy, unsigned titleSize, Orientation ori, std::atomic<double>& val);
    Slider& setFixed(bool val) {fixed = val; return *this;}

	template<class T>
	static std::shared_ptr<Slider> DefaultSlider(const std::string& name, double from, double to, float px, float py, T&& onMoveVal)
	{
		constexpr float width = 30;
		constexpr float height = 100;
		constexpr unsigned titleSize = 16;

		std::shared_ptr<Slider> ptr;
		if constexpr (std::is_constructible_v<std::function<void(Slider&)>,T>)
		{
			ptr = std::make_shared<Slider>(name, from, to, px, py, width, height, titleSize, Slider::Vertical, [&]() {});
			ptr->onMove = [&onMoveVal, ptr]() {
				onMoveVal(*ptr);
			};
		}
		else {
			ptr = std::make_shared<Slider>(name, from, to, px, py, width, height, titleSize, Slider::Vertical, onMoveVal);
		}
		return ptr;
	}

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    virtual void forwardEvent(const sf::Event& event) override;
    sf::Vector2f getPosition() const {return mainRect.getPosition();}
    void setPosition(const sf::Vector2f& p) {mainRect.setPosition(p);}

    double getValue() const {return value;}

private:
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

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    void setPosition(const sf::Vector2f& p) {window.setPosition(p);}
    sf::Vector2f getPosition() const {return window.getPosition();}
    void newSamples(const std::vector<double>& samples);

private:
    sf::RectangleShape window;
    mutable std::vector<sf::Vertex> vArray;
    unsigned resolution;
    double speed;
    double currTime = 0;
};

#endif // GUI_H_INCLUDED
