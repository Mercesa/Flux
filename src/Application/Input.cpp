#include "Input.h"


#include "GLFW/glfw3.h"
#include <iostream>

using namespace Flux;

Input::Input()
{
	for (int i = 0; i < numOfKeys; ++i)
	{
		keyPress[i] = false;
		keyHeld[i] = false;
		keyUp[i] = false;
	}
}


Input::~Input()
{
}

void MousebuttonInput(int key, int scancode, int action, int mods)
{

}


void Input::Update(bool pause)
{
	for (int i = 0; i < numOfKeys; ++i)
	{
		keyUp[i] = false;
		keyPress[i] = false;
		if (pause)
		{
			keyHeld[i] = false;
		}
	}

	relMousePos.x = 0.0f;
	relMousePos.y = 0.0f;


	firstFrame = false;
}


void Input::KeyboardInput(int key, int scancode, int action, int mods)
{
	switch (action)
	{
	case GLFW_PRESS:
		keyPress[key] = true;
		keyHeld[key] = true;
		break;

	case GLFW_RELEASE:
		keyUp[key] = true;
		keyHeld[key] = false;
		break;

	case GLFW_REPEAT:
		keyHeld[key] = true;
		break;

	default:
		//LOG(ERROR) << "Input::MouseButtonInput invalid operation detected";
		break;
	}
}

bool Input::GetKeyUp(int32_t key)
{
	return keyUp[key];
}

bool Input::GetKeyDown(int32_t key)
{
	return keyPress[key];
}

bool Input::GetKeyHeld(int32_t key)
{
	return keyHeld[key];
}

Input::XYPair Input::GetCurrentMousePos()
{
	return lastMousePos;
}

Input::XYPair Input::GetRelMousePos()
{
	return relMousePos;
}

void Input::MouseMoveInput(double xpos, double ypos)
{
	if (firstFrame)
	{
		lastMousePos.x = xpos;
		lastMousePos.y = ypos;

	}

	relMousePos.x = xpos - lastMousePos.x;
	relMousePos.y = lastMousePos.y - ypos;

	lastMousePos.x = xpos;
	lastMousePos.y = ypos;
}

