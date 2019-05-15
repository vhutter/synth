#ifndef GUI_H_INCLUDED
#define GUI_H_INCLUDED

#include <SFML/Graphics.hpp>

class Window;

void setupGui(std::shared_ptr<Window> gui, sf::RenderWindow& window);

#endif // GUI_H_INCLUDED