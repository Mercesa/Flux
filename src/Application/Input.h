#pragma once

#include <array>
#include <cstdint>

#include "Application/Application.h"

namespace Flux
{
	static const int32_t numOfKeys = 349;

class Input
{
public:
	struct XYPair
	{
		double x = 0;
		double y = 0;
	};

	Input();
	~Input();

	void Update(bool pause);
	void KeyboardInput(int key, int scancode, int action, int mods);
	void MouseMoveInput(double xpos, double ypos);

	bool GetKeyUp(int32_t key);
	bool GetKeyDown(int32_t key);
	bool GetKeyHeld(int32_t key);

	XYPair GetRelMousePos();
	XYPair GetCurrentMousePos();

	XYPair lastMousePos;

private:
	bool firstFrame = true;

	XYPair relMousePos;

	std::array<bool, numOfKeys> keyPress;
	std::array<bool, numOfKeys> keyHeld;
	std::array<bool, numOfKeys> keyUp;
};
}
