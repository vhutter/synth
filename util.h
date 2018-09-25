#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <SFML/Audio.hpp>
#include <SFML/Window.hpp>


namespace util
{


class Note
{
    public:
        Note(double freq);
        double getFreq() const {return freq;}
        double getFifth() const {return freq*3/2;}
        operator double() const {return freq;}

    private:
        double freq;
};



template<typename T>
void flip(T& b)
{
    b = !b;
}

template<typename T>
void updatePressed(T& pressed, const sf::Keyboard::Key& key, bool value)
{
    using namespace sf;
    switch (key)
    {
    case sf::Keyboard::Z:
        pressed[0] = value;
        break;
    case sf::Keyboard::S:
        pressed[1] = value;
        break;
    case sf::Keyboard::X:
        pressed[2] = value;
        break;
    case sf::Keyboard::D:
        pressed[3] = value;
        break;
    case sf::Keyboard::C:
        pressed[4] = value;
        break;
    case sf::Keyboard::V:
        pressed[5] = value;
        break;
    case sf::Keyboard::G:
        pressed[6] = value;
        break;
    case sf::Keyboard::B:
        pressed[7] = value;
        break;
    case sf::Keyboard::H:
        pressed[8] = value;
        break;
    case sf::Keyboard::N:
        pressed[9] = value;
        break;
    case sf::Keyboard::J:
        pressed[10] = value;
        break;
    case sf::Keyboard::M:
        pressed[11] = value;
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
