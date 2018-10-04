#include "gui.h"

#include <deque>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace
{
    sf::Font loadCourierNew()
    {
        sf::Font tmpFont;
        if (!tmpFont.loadFromFile("fonts/cour.ttf")) {
            throw std::runtime_error("fonts/cour.ttf not found");
        }
        return tmpFont;
    }
}

const sf::Vector2f SynthKey::whiteSize(100, 200);
const sf::Vector2f SynthKey::blackSize(40, 150);
const sf::Vector2f Slider::size(80, 300);
const sf::Vector2f Slider::sliderRectSize(40, 40);
sf::Font TextDisplay::font = loadCourierNew();

SynthKey::SynthKey(Type t, float px, float py)
    :type(t)
{
    setPosition(sf::Vector2f(px, py));

    if (t == Type::Black) {
        setSize(SynthKey::blackSize);
        setFillColor(sf::Color::Black);
    }
    else {
        setSize(SynthKey::whiteSize);
        setFillColor(sf::Color::White);
    }

    setOutlineColor(sf::Color(0x4C0099FF));
    setOutlineThickness(4.);
}

void SynthKey::setPressed(bool p)
{
    pressed = p;
    if (type == Type::White) {
        if (pressed) setFillColor(sf::Color(0xC0C0C0FF));
        else setFillColor(sf::Color::White);
    }
    else {
        if (pressed) setFillColor(sf::Color(0x404040FF));
        else setFillColor(sf::Color::Black);
    }
}

bool SynthKey::isPressed() const
{
    return pressed;
}

SynthKeyboard::SynthKeyboard(float px, float py, std::function<void(unsigned)> eventCallback)
    : pos(px, py), onKey(eventCallback)
{
    const auto& w = SynthKey::White;
    const auto& b = SynthKey::Black;

    keys.reserve(12);
    for (auto type: {w, b, w, b, w, w, b, w, b, w, b, w})
        keys.push_back(SynthKey(type));

    repositionKeys();
}

void SynthKeyboard::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    for (const auto& key: keys)
        if (key.getType() == SynthKey::White)
            target.draw(key);

    for (const auto& key: keys)
        if (key.getType() == SynthKey::Black)
            target.draw(key);
}

void SynthKeyboard::forwardEvent(const sf::Event& event)
{
    const auto& key = event.key.code;
    int value = -1;
    switch (key)
    {
    case sf::Keyboard::Z:
        value = 0;
        break;
    case sf::Keyboard::S:
        value = 1;
        break;
    case sf::Keyboard::X:
        value = 2;
        break;
    case sf::Keyboard::D:
        value = 3;
        break;
    case sf::Keyboard::C:
        value = 4;
        break;
    case sf::Keyboard::V:
        value = 5;
        break;
    case sf::Keyboard::G:
        value = 6;
        break;
    case sf::Keyboard::B:
        value = 7;
        break;
    case sf::Keyboard::H:
        value = 8;
        break;
    case sf::Keyboard::N:
        value = 9;
        break;
    case sf::Keyboard::J:
        value = 10;
        break;
    case sf::Keyboard::M:
        value = 11;
        break;
    default:
        break;
    }


    lastPressed = false;

    if (value == -1)
        return;

    switch(event.type)
    {
        case sf::Event::KeyPressed: {
            lastPressed = true;
            onKey(value);
            break;
        }
        case sf::Event::KeyReleased: {
            onKey(value);
            break;
        }
        default:
            break;
    }
}

void SynthKeyboard::setPosition(const sf::Vector2f& newPos)
{
    pos = newPos;
    repositionKeys();
}

void SynthKeyboard::repositionKeys()
{
    unsigned whitesCount = 0;
    float dif = SynthKey::whiteSize.x - SynthKey::blackSize.x / 2;
    for (unsigned i=0; i<keys.size(); ++i) {
        auto& key = keys[i];
        if (key.getType() == SynthKey::White) {
            key.setPosition(sf::Vector2f(pos.x+ whitesCount*SynthKey::whiteSize.x, pos.y));
            ++whitesCount;
        }
        else {
            key.setPosition(sf::Vector2f(keys[i-1].getPosition().x + dif, pos.y));
        }
    }
}


Slider::Slider(const std::string& str, float px, float py, Orientation ori, std::function<void()> callback)
    :title(str, px, py-80), name(str), orientation(ori), onMove(callback)
{
    if (orientation == Vertical) {
        mainRect.setSize(size);
    }
    else {
        mainRect.setSize(sf::Vector2f(size.y, size.x));
    }

    mainRect.setPosition(sf::Vector2f(px, py));
    mainRect.setOutlineColor(sf::Color(0x757575FF));
    mainRect.setFillColor(sf::Color(0x660000FF));
    mainRect.setOutlineThickness(4.);

    sliderRect.setSize(sliderRectSize);
    sliderRect.setPosition(sf::Vector2f(px + mainRect.getSize().x/2 - sliderRectSize.x/2, py + mainRect.getSize().y/2 - sliderRectSize.y/2));
    sliderRect.setOutlineColor(sf::Color(0x757575FF));
    sliderRect.setFillColor(sf::Color(0x003366FF));
    sliderRect.setOutlineThickness(4.);
}

void Slider::forwardEvent(const sf::Event& event)
{
    switch(event.type)
    {
        case sf::Event::MouseButtonPressed: {
            const auto& mousePos = sf::Vector2f(event.mouseButton.x, event.mouseButton.y);
            if(containsPoint(mousePos))
                clicked = true;
            break;
        }
        case sf::Event::MouseButtonReleased: {
            clicked = false;
            if (fixed) {
                auto [px, py] = mainRect.getPosition();
                sliderRect.setPosition(sf::Vector2f(px + mainRect.getSize().x/2 - sliderRectSize.x/2, py + mainRect.getSize().y/2 - sliderRectSize.y/2));
                moveSlider(sf::Vector2f(px+mainRect.getSize().x/2, py+mainRect.getSize().y/2));
                onMove();
            }
            break;
        }
        case sf::Event::MouseMoved: {
            if (clicked) {
                const auto& mousePos = sf::Vector2f(event.mouseMove.x, event.mouseMove.y);
                moveSlider(mousePos);
                onMove();
            }
            break;
        }
        default: {
            break;
        }
    }
}

void Slider::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    std::ostringstream oss;
    oss << std::setprecision(2) << std::fixed << value;
    const_cast<TextDisplay*>(&title)->setText(name+"\n["+oss.str()+"]");
    target.draw(title, states);
    target.draw(mainRect, states);
    target.draw(sliderRect, states);
}

bool Slider::containsPoint(const sf::Vector2f& p) const
{
    const auto& rect = sliderRect;
    const auto& u = rect.getPosition();
    const auto& v = u + rect.getSize();

    return u.x <= p.x && p.x <= v.x &&
        u.y <= p.y && p.y <= v.y;
}

void Slider::moveSlider(const sf::Vector2f& p)
{
    const auto& minPos = mainRect.getPosition();
    const auto& maxPos = minPos + mainRect.getSize() - sliderRect.getSize();
    const auto& rectSize = maxPos - minPos;
    const auto& currentPos = sliderRect.getPosition();
    const auto& sliderSize = sliderRect.getSize();
    const auto& midPos = minPos + rectSize / 2.f;

    double newValue;

    if (orientation == Vertical) {
        sliderRect.setPosition(currentPos.x, std::clamp(p.y-sliderSize.y/2, minPos.y, maxPos.y));
        newValue = (midPos.y - sliderRect.getPosition().y) / rectSize.y*2;
    }
    else {
        sliderRect.setPosition(std::clamp(p.x-sliderSize.x/2, minPos.x, maxPos.x), currentPos.y);
        newValue = (sliderRect.getPosition().x - midPos.x) / rectSize.x*2;
    }

    value = newValue;
}

Oscilloscope::Oscilloscope(float px, float py, float sx, float sy, unsigned res, double speed)
    : resolution(res), speed(speed)
{
    window.setPosition(sf::Vector2f(px, py));
    window.setSize(sf::Vector2f(sx, sy));
    window.setOutlineColor(sf::Color::White);
    window.setFillColor(sf::Color::Black);
    window.setOutlineThickness(2.);

    vArray.reserve(resolution*2); // bigger buffer size for faster incoming samples
    vArray.resize(resolution);
    const auto& pos = window.getPosition();
    const auto& size = window.getSize();
    for (unsigned i=0; i<resolution; ++i) {
        vArray[i].position.x = pos.x+i*size.x/resolution;
        vArray[i].position.y = pos.y+size.y/2;
        vArray[i].color = sf::Color::Green;
    }
}

void Oscilloscope::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(window, states);
    target.draw(vArray.data(), resolution, sf::LineStrip);
}

void Oscilloscope::newSamples(const std::vector<double>& samples)
{
    unsigned dif = samples.size();
    if (dif < vArray.size()) {
        for (unsigned i=0; i<vArray.size()-dif; ++i)
            vArray[i].position.y = vArray[i+dif].position.y;
        for (unsigned i=vArray.size()-dif; i<vArray.size(); ++i)
            vArray[i].position.y = window.getPosition().y + window.getSize().y/2 + samples[i-vArray.size()+dif];
    }
    else {
        for (unsigned i=0; i<vArray.size(); ++i)
            vArray[i].position.y = window.getPosition().y + window.getSize().y/2 + samples[i];
    }
}

TextDisplay::TextDisplay(const std::string& initialText, float px, float py, unsigned int size, float sx, float sy)
    : text()
{
    window.setPosition(sf::Vector2f(px, py));
    window.setSize(sf::Vector2f(sx, sy));
    window.setOutlineColor(sf::Color::White);
    window.setFillColor(sf::Color::Black);
    window.setOutlineThickness(1.);

    text.setFont(font);
    text.setCharacterSize(size);
    text.setString(initialText);
    text.setPosition(sf::Vector2f(px, py));
    text.setFillColor(sf::Color::Green);
}

void TextDisplay::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(window, states);
    target.draw(text, states);
}

void TextDisplay::setText(const std::string& content)
{
    text.setString(content);
}
