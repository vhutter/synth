#ifndef SYNTHKEYBOARD_H_INCLUDED
#define SYNTHKEYBOARD_H_INCLUDED

#include "GuiElement.h"
#include <atomic>

class SynthKey : public sf::RectangleShape
{
public:
	enum Type : bool { White = true, Black = false };
	enum State : bool { Pressed = true, Released = false };

	SynthKey(Type t, const SynthVec2& size = SynthVec2(0, 0));
	SynthKey(const SynthKey& other) : sf::RectangleShape(other), type(other.type), pressed(other.pressed.load()) {}
	void setPressed(bool p);

	static SynthVec2 whiteSizeDefault();
	static SynthVec2 blackSizeDefault();

	const Type type;

private:
	std::atomic<bool> pressed{ false };

};

class KeyboardOutput;

class SynthKeyboard : public GuiElement
{
	friend class KeyboardOutput;
public:
	using callback_t = std::function<void(unsigned, SynthKey::State)>;

	SynthKeyboard(unsigned keyCount, callback_t eventCallback);
	void setSize(SynthKey::Type type, const SynthVec2& size);
	virtual SynthRect AABB() const override;
	virtual bool needsEvent(const SynthEvent& event) const override;
	SynthKey& operator[] (std::size_t i);
	void setOctaveShift(unsigned n);
	unsigned getOctaveShift();

private:
	virtual void onMidiEvent(const MidiEvent& event) override;
	virtual void onSfmlEvent(const sf::Event& event) override;
	virtual void drawImpl(sf::RenderTarget& target, sf::RenderStates states) const override;
	void repositionKeys(unsigned keyCount);

	std::vector<SynthKey> keys;
	callback_t onKey;
	SynthVec2 blackSize{ SynthKey::blackSizeDefault() }, whiteSize{ SynthKey::whiteSizeDefault() };
	unsigned octaveShift{ 0 };
};

class KeyboardOutput
{
public:
	KeyboardOutput(unsigned keyCount);
	std::shared_ptr<SynthKeyboard> getSynthKeyboard() const;
	void stopAll();

	template<typename ...T>
	void outputTo(T&... other)
	{
		using namespace std::placeholders;
		(callbacks.push_back(std::bind(&T::onKeyEvent, &other, _1, _2)), ...);
	}

private:
	std::vector<SynthKeyboard::callback_t> callbacks;
	std::shared_ptr<SynthKeyboard> kb;
};

#endif //SYNTHKEYBOARD_H_INCLUDED