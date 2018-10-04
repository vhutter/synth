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
