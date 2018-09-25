#ifndef GUI_H_INCLUDED
#define GUI_H_INCLUDED

#include <SFML/Graphics.hpp>
#include <vector>
#include <functional>
#include <atomic>

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
    bool pressed = false;
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
    SynthKeyboard(float px, float py);

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    void setPosition(const sf::Vector2f& p);
    sf::Vector2f getPosition() const {return pos;};

    SynthKey& operator[] (std::size_t i) {return keys[i];}


private:
    void repositionKeys();

    sf::Vector2f pos;
    std::vector<SynthKey> keys;
};

class TextDisplay : public GuiElement
{
public:
    TextDisplay(const std::string& initialText, float px, float py, unsigned int charSize=24, float sx=0, float sy=0);

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    void setPosition(const sf::Vector2f& p) {window.setPosition(p);}
    sf::Vector2f getPosition() const {return window.getPosition();}
    const std::string& getText() {return content;}
    void setText(const std::string& text);
    const sf::Text& getText() const {return text;}


private:
    sf::RectangleShape window;
    sf::Text text;
    std::string content;

    static sf::Font font;
};

class Slider : public GuiElement
{
public:
    enum Orientation : bool {Vertical = true, Horizontal = false};

    Slider(const std::string& name, float px, float py, Orientation ori, std::function<void()> onMove);
    Slider& setFixed(bool val) {fixed = val; return *this;}

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    virtual void forwardEvent(const sf::Event& event) override;

    void setPosition(const sf::Vector2f& p) {mainRect.setPosition(p);}
    sf::Vector2f getPosition() const {return mainRect.getPosition();}
    bool containsPoint(const sf::Vector2f& p) const;
    void moveSlider(const sf::Vector2f& p);
    double getValue() const {return value;}

private:
    TextDisplay title;
    const std::string name;
    sf::RectangleShape mainRect, sliderRect;
    std::atomic<double> value = 0;
    Orientation orientation;
    bool fixed = false;

    bool clicked;
    std::function<void()> onMove;

    static const sf::Vector2f size;
    static const sf::Vector2f sliderRectSize;
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
