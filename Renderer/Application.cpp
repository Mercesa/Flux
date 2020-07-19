#include "Application.h"

#include "Renderer.h"
#include "Input.h"
#include "Camera.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

using namespace Flux;


std::shared_ptr<Input> mInput;
std::shared_ptr<Camera> mCamera;

void cursorpos_callback(GLFWwindow* window, double xpos, double ypos)
{
	static bool firstFrame = true;
	static double lastPosX = 0;
	static double lastPosY = 0;

	if (firstFrame)
	{
		lastPosX = xpos;
		lastPosY = ypos;
		firstFrame = false;
	}
	


	mCamera->ProcessMouseMovement((xpos - lastPosX), -(ypos - lastPosY), true);
	mInput->MouseMoveInput(xpos, ypos);

	lastPosX = xpos;
	lastPosY = ypos;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	mInput->KeyboardInput(key, scancode, action, mods);
}

Flux::Application::Application() : mResized(false), mRenderer(nullptr), mWindow(nullptr)
{
}

Flux::Application::~Application()
{
	glfwDestroyWindow(mWindow);

}

void character_callback(GLFWwindow* window, unsigned int codepoint)
{
	std::cout << codepoint << std::endl;
}


void Flux::Application::Run()
{
	mInput = std::make_shared<Input>();
	mCamera = std::make_shared<Camera>(glm::vec3(0.0f, 5.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f), YAW, -45.0f);


	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	mWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	glfwSetWindowUserPointer(mWindow, this);
	glfwSetFramebufferSizeCallback(mWindow, framebufferResizeCallback);
	glfwSetCharCallback(mWindow, character_callback);
	glfwSetKeyCallback(mWindow, key_callback);
	glfwSetCursorPosCallback(mWindow, cursorpos_callback);
	mRenderer = std::make_unique<Renderer>(mCamera);
	mRenderer->SetWindow(mWindow);
	mRenderer->Init();
	mCamera->MouseSensitivity = 1.0f;

	while (!glfwWindowShouldClose(mWindow)) {
		glfwPollEvents();
		
		if (mInput->GetKeyHeld(GLFW_KEY_W))
		{
			mCamera->ProcessKeyboard(Flux::Camera_Movement::FORWARD, 0.1f);
		}
		else if (mInput->GetKeyHeld(GLFW_KEY_A))
		{
			mCamera->ProcessKeyboard(Flux::Camera_Movement::LEFT, 0.1f);
		}
		else if (mInput->GetKeyHeld(GLFW_KEY_S))
		{
			mCamera->ProcessKeyboard(Flux::Camera_Movement::BACKWARD, 0.1f);
		}
		else if (mInput->GetKeyHeld(GLFW_KEY_D))
		{
			mCamera->ProcessKeyboard(Flux::Camera_Movement::RIGHT, 0.1f);
		}

		mRenderer->Draw();

	}

	mRenderer->WaitIdle();

}
