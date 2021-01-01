#include "Application.h"

#include "Application/Rendering/CustomRenderer.h"

#include "Input.h"
#include "Camera.h"

#include "Application/Scene/FirstScene.h"

#include "Common/Time/Timer.h"

const uint32_t WIDTH = 1366;
const uint32_t HEIGHT = 768;

using namespace Flux;


std::shared_ptr<Input> mInput;

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
{}

void Flux::Application::Run()
{
	mInput = std::make_shared<Input>();

	std::shared_ptr<FirstScene> mScene = std::make_shared<FirstScene>(mInput);
	mRenderer = std::make_unique<CustomRenderer>();
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	mWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	glfwSetWindowUserPointer(mWindow, this);
	glfwSetFramebufferSizeCallback(mWindow, framebufferResizeCallback);
	glfwSetCharCallback(mWindow, character_callback);
	glfwSetKeyCallback(mWindow, key_callback);
	glfwSetCursorPosCallback(mWindow, cursorpos_callback);
	mRenderer->SetWindow(mWindow);
	mRenderer->Init();

	std::shared_ptr<Timer> tTimer = std::make_shared<Timer>();
	tTimer->Reset();
	float tDeltaTime = 0.0f;

	mScene->Init();
	while (!glfwWindowShouldClose(mWindow)) {

		tDeltaTime = static_cast<float>(tTimer->GetDelta() * 0.001);

		glfwPollEvents();

		if (mInput->GetKeyHeld(GLFW_KEY_ESCAPE))
		{
			break;
		}
		mScene->Update(tDeltaTime);
		mRenderer->Draw(mScene);
		mInput->Update();
	}
	mScene->Cleanup();

	mRenderer->WaitIdle();
	mRenderer->Cleanup();
}
