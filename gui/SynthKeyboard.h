#ifndef SYNTHKEYBOARD_H_INCLUDED
#define SYNTHKEYBOARD_H_INCLUDED

#include "GuiElement.h"
#include <atomic>

class SynthKey : public sf::RectangleShape
{
public:
	enum Type : bool { White = true, Black = false };
	enum State : bool { Pressed = true, Released = false };

	SynthKey(Type t, SynthFloat px = 0, SynthFloat py = 0, const SynthVec2& size = SynthVec2(0, 0));
	SynthKey(const SynthKey& other) : sf::RectangleShape(other), type(other.type), pressed(other.pressed.load()) {}

	static const SynthVec2 whiteSizeDefault, blackSizeDefault;

	const Type type;
	std::atomic<bool> pressed{ false };

};

class SynthKeyboard : public GuiElement
{
public:
	using callback_t = std::function<void(unsigned, SynthKey::State)>;

	SynthKeyboard(SynthFloat px, SynthFloat py, callback_t eventCallback);
	virtual SynthRect AABB() const override;
	virtual bool needsEvent(const SynthEvent& event) const override;
	SynthKey& operator[] (std::size_t i) { return keys[i]; }

private:
	virtual void onMidiEvent(const MidiEvent& event) override;
	virtual void onSfmlEvent(const sf::Event& event) override;
	virtual void drawImpl(sf::RenderTarget& target, sf::RenderStates states) const override;
	void repositionKeys();

	std::vector<SynthKey> keys;
	callback_t onKey;
	const SynthVec2 blackSize{ SynthKey::blackSizeDefault }, whiteSize{ SynthKey::whiteSizeDefault };
};

#endif //SYNTHKEYBOARD_H_INCLUDED