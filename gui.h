#ifndef GUI_H_INCLUDED
#define GUI_H_INCLUDED

#include <SFML/Graphics.hpp>
#include <vector>

class SynthKey : public sf::RectangleShape
{
public:
    enum class Type : bool {White = true, Black = false};

    SynthKey(Type t, float px=0, float py=0);
    Type getType() const {return type;};
    void setPressed(bool pressed);
    bool isPressed() const;


    static const sf::Vector2f whiteSize, blackSize;

private:
    bool pressed = false;
    Type type;

};


class SynthKeyboard : public sf::Drawable
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

#endif // GUI_H_INCLUDED
