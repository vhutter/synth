#ifndef SYNTH_UTILITY_H_DEFINED
#define SYNTH_UTILITY_H_DEFINED

#include <SFML/Graphics.hpp>


template<typename T>
struct Property
{
public:
	Property() = default;
	Property(const T& val) : obj(val) {}

	const T& get() { return obj; }
	operator const T& () const { return obj; }
	void set(const T& val) { obj = val; }
	T& operator= (const T& val) { obj = val; return obj; }


private:
	T obj;
};

using SynthFloat = double;
using SynthVec2 = sf::Vector2<SynthFloat>;
using SynthRect = sf::Rect<SynthFloat>;

sf::Font loadCourierNew();
sf::View getCroppedView(const sf::View& oldView, SynthFloat x, SynthFloat y, SynthFloat w, SynthFloat h);
sf::View getCroppedView(const sf::View& oldView, const SynthVec2& p, const SynthVec2& s);
sf::View getCroppedView(const sf::View& oldView, const SynthRect& box);

#endif