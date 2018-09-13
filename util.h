#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <SFML/Audio.hpp>
#include <SFML/Window.hpp>

namespace utility
{



class Note
{
    public:
        Note(float freq);
        float getFreq() const {return freq;}
        float getFifth() const {return freq*3/2;}

    private:
        float freq;
};



template<typename T>
void flip(T& b)
{
    b = !b;
}

template<typename T>
void updatePressed(T& pressed, const sf::Keyboard::Key& key)
{
    using namespace sf;
    switch (key)
    {
    case sf::Keyboard::Z:
        flip(pressed[0]);
        break;
    case sf::Keyboard::S:
        flip(pressed[1]);
        break;
    case sf::Keyboard::X:
        flip(pressed[2]);
        break;
    case sf::Keyboard::D:
        flip(pressed[3]);
        break;
    case sf::Keyboard::C:
        flip(pressed[4]);
        break;
    case sf::Keyboard::V:
        flip(pressed[5]);
        break;
    case sf::Keyboard::G:
        flip(pressed[6]);
        break;
    case sf::Keyboard::B:
        flip(pressed[7]);
        break;
    case sf::Keyboard::H:
        flip(pressed[8]);
        break;
    case sf::Keyboard::N:
        flip(pressed[9]);
        break;
    case sf::Keyboard::J:
        flip(pressed[10]);
        break;
    case sf::Keyboard::M:
        flip(pressed[11]);
        break;
    default:
        break;
    }
}


extern const Note A  ;
extern const Note Ais;
extern const Note B  ;
extern const Note C  ;
extern const Note Cis;
extern const Note D  ;
extern const Note Dis;
extern const Note E  ;
extern const Note F  ;
extern const Note Fis;
extern const Note G  ;
extern const Note Gis;






}


#endif // UTIL_H_INCLUDED
