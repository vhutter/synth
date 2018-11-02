#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <initializer_list>
#include <vector>
#include <iostream>
#include "generators.h"


namespace util
{


class Note
{
    public:
        Note(double freq);
        double getFifth() const {return freq*3/2;}
        operator double() const {return freq;}

    private:
        double freq;
};


struct Tone {
    Note note;
    double intensity;
    waves::wave_t waveform;
    double phase;

    Tone(const Note& note, double intensity, waves::wave_t waveform, double phase=0)
        :note(note), intensity(intensity), waveform(waveform), phase(phase) {}
};

class CompoundTone
{
    public:

        CompoundTone();
        CompoundTone(std::initializer_list<Tone>, const ADSREnvelope& env = ADSREnvelope());

        void addComponent(const Tone& desc);
        void normalize();
        void modifyMainPitch(double t, double dest);
        void shiftOctave(double time, double n);
        double getSample(double t);
        const Note& getMainNote() const {return mainNote;}

        ADSREnvelope envelope;
    private:
        const std::vector<Tone> initialComponents;
        std::vector<Tone> components;
        double octave = 1.;
        Note mainNote = 0;
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
