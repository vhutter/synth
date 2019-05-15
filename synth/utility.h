#ifndef SYNTH_UTILITY_H_DEFINED
#define SYNTH_UTILITY_H_DEFINED

#include <SFML/Graphics.hpp>

using namespace std::string_literals;

using SynthFloat = double;
using SynthVec2 = sf::Vector2<SynthFloat>;
using SynthRect = sf::Rect<SynthFloat>;

const sf::Font& loadCourierNew();

sf::View getCroppedView(const sf::View& oldView, SynthFloat x, SynthFloat y, SynthFloat w, SynthFloat h);
sf::View getCroppedView(const sf::View& oldView, const SynthVec2& p, const SynthVec2& s);
sf::View getCroppedView(const sf::View& oldView, const SynthRect& box);

struct Note;
std::vector<Note> generateNotes(int fromOctave, int toOctave);

static double eps = 0.001;

void log(const std::string& str);
unsigned getConfig(const std::string& str);

#endif