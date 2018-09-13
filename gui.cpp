#include "gui.h"

#include <deque>

const sf::Vector2f SynthKey::whiteSize(100, 200);
const sf::Vector2f SynthKey::blackSize(80, 150);

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
    const auto& w = SynthKey::Type::White;
    const auto& b = SynthKey::Type::Black;

    for (auto type: {w, b, w, b, w, w, b, w, b, w, b, w})
        keys.emplace_back(SynthKey(type));

    repositionKeys();
}

void SynthKeyboard::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    for (const auto& key: keys)
        if (key.getType() == SynthKey::Type::White)
            target.draw(key);

    for (const auto& key: keys)
        if (key.getType() == SynthKey::Type::Black)
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
        if (key.getType() == SynthKey::Type::White) {
            key.setPosition(sf::Vector2f(pos.x+ whitesCount*SynthKey::whiteSize.x, pos.y));
            ++whitesCount;
        }
        else {
            key.setPosition(sf::Vector2f(keys[i-1].getPosition().x + dif, pos.y));
        }
    }
}
