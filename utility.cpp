#include "utility.h"

sf::Font loadCourierNew()
{
	sf::Font tmpFont;
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
	oldPos.x = std::floor(oldPos.x);
	oldPos.y = std::floor(oldPos.y);
	SynthFloat u = x + w;
	SynthFloat v = y + h;
	x = std::max(x, SynthFloat(oldPos.x));
	y = std::max(y, SynthFloat(oldPos.y));
	w = std::min(u, SynthFloat(oldPos.x + oldDim.x)) - x;
	h = std::min(v, SynthFloat(oldPos.y + oldDim.y)) - y;
	if (w < 0 || h < 0) {
		return sf::View({ 0, 0 }, { 0,0 });
	}

	const auto center = SynthVec2{ std::round(x + w / 2), std::round(y + h / 2) };
	const auto size = SynthVec2{ w, h };
	sf::View ret{ sf::Vector2f(center), sf::Vector2f(size) };

	const auto& oldViewport = SynthRect{ oldView.getViewport() };
	auto oldSize = SynthVec2{ oldView.getSize() };
	oldSize.x = std::round(oldSize.x * 1.f / oldViewport.width);
	oldSize.y = std::round(oldSize.y * 1.f / oldViewport.height);
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