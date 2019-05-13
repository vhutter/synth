#include "utility.h"
#include "core/generators.h"

#include <unordered_set>
#include <fstream>
#include <sstream>
#include <ctime>
#include <utility>

const sf::Font& loadCourierNew()
{
	static sf::Font tmpFont;
	if (!tmpFont.loadFromFile("fonts/cour.ttf")) {
		throw std::runtime_error("fonts/cour.ttf not found");
	}
	return tmpFont;
}

sf::View getCroppedView(const sf::View& oldView, SynthFloat x, SynthFloat y, SynthFloat w, SynthFloat h)
{
	// Intersect the 2 rectangles (old and current origin window)
	const auto& oldDim = oldView.getSize();
	auto oldPos = oldView.getCenter() - oldDim / 2.f;
	oldPos.x = oldPos.x;
	oldPos.y = oldPos.y;
	x = std::max(x, SynthFloat(oldPos.x));
	y = std::max(y, SynthFloat(oldPos.y));
	w = std::min(x + w, SynthFloat(oldPos.x) + oldDim.x) - x;
	h = std::min(y + h, SynthFloat(oldPos.y) + oldDim.y) - y;
	if (w < 0 || h < 0) {
		return sf::View({ 0, 0 }, { 0,0 });
	}

	const auto center = SynthVec2{ x + w / 2, y + h / 2 };
	const auto size = SynthVec2{ w, h };
	sf::View ret{ sf::Vector2f(center), sf::Vector2f(size) };

	const auto& oldViewport = SynthRect{ oldView.getViewport() };
	auto oldSize = SynthVec2{ oldView.getSize() };
	oldSize.x = oldSize.x / oldViewport.width;
	oldSize.y = oldSize.y / oldViewport.height;
	const auto& ratio = SynthVec2{
		size.x / oldSize.x,
		size.y / oldSize.y
	};
	const auto& pos = SynthVec2(
		x / oldSize.x,
		y / oldSize.y
	);

	ret.setViewport(sf::FloatRect(pos.x, pos.y, ratio.x, ratio.y));
	return ret;
}

sf::View getCroppedView(const sf::View& oldView, const SynthVec2& p, const SynthVec2& s)
{
	return getCroppedView(oldView, p.x, p.y, s.x, s.y);
}

sf::View getCroppedView(const sf::View& oldView, const SynthRect& box)
{
	return getCroppedView(oldView, box.left, box.top, box.width, box.height);
}

std::vector<Note> generateNotes(int from, int to)
{
	std::vector<Note> notes;
	for (int i = from; i <= to; ++i)
		for (auto note : Note::baseNotes()) notes.push_back(note * pow(2, i));
	notes.push_back(Note::baseNotes()[0] * pow(2, to + 1));
	return notes;
}

bool isAlpha(const sf::Keyboard::Key& key)
{
	using K = sf::Keyboard::Key;
	static std::unordered_set<sf::Keyboard::Key> set{
		K::A,
		K::B,
		K::C,
		K::D,
		K::E,
		K::F,
		K::G,
		K::H,
		K::I,
		K::J,
		K::K,
		K::L,
		K::M,
		K::N,
		K::O,
		K::P,
		K::Q,
		K::R,
		K::S,
		K::T,
		K::U,
		K::V,
		K::W,
		K::X,
		K::Y,
		K::Z,
	};
	return set.find(key) != set.end();
}

bool isNumeric(const sf::Keyboard::Key& key)
{
	using K = sf::Keyboard::Key;
	static std::unordered_set<sf::Keyboard::Key> set{
		K::Num0,
		K::Num1,
		K::Num2,
		K::Num3,
		K::Num4,
		K::Num5,
		K::Num6,
		K::Num7,
		K::Num8,
		K::Num9,
	};
	return set.find(key) != set.end();
}

bool isAlnum(const sf::Keyboard::Key& key)
{
	return isNumeric(key) || isAlpha(key);
}

void log(const std::string& str)
{
	static std::ofstream log("Logs.txt", std::ios_base::app);
	static char buf[500];
	std::time_t now = std::time(nullptr);
	if (std::strftime(buf, sizeof(buf), "%F/%T", std::localtime(&now))) {
		log << buf << "   " << str << "\n";
	}
}

//https://stackoverflow.com/questions/28054528/overloading-ifstream-iterator-for-pairs
struct P : std::pair<std::string, unsigned>
{
	using std::pair<std::string, unsigned>::pair;
};

std::istream& operator>> (std::istream& in, P& p)
{
	std::string tmp;
	in >> p.first >> tmp;
	std::istringstream iss(tmp);
	if (tmp.rfind("0x", 0) == 0) {
		iss >> std::hex >> p.second;
	}
	else {
		iss >> std::dec >> p.second;
	}
	return in;
}

unsigned getConfig(const std::string& str)
{
	static std::ifstream cfgFile("Config.txt");
	static std::unordered_map<std::string, unsigned> cfg{
		std::istream_iterator<P>(cfgFile),
		std::istream_iterator<P>()
	};
	try {
		return cfg.at(str);
	}
	catch (...) {
		throw std::out_of_range(str + " does not exist in Config.txt");
	}
}
