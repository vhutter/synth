#include "gui.h"

#include <deque>
#include <algorithm>

const sf::Vector2f SynthKey::whiteSize(100, 200);
const sf::Vector2f SynthKey::blackSize(40, 150);
const sf::Vector2f Slider::size(80, 300);
const sf::Vector2f Slider::sliderRectSize(40, 40);

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

SynthKeyboard::SynthKeyboard(float px, float py)
    : pos(px, py)
{
    const auto& w = SynthKey::White;
    const auto& b = SynthKey::Black;

    for (auto type: {w, b, w, b, w, w, b, w, b, w, b, w})
        keys.emplace_back(SynthKey(type));

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


Slider::Slider(float px, float py, Orientation ori)
    :orientation(ori)
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


void Slider::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
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
