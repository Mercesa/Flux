#include "Application.h"

#include "Application/Rendering/CustomRenderer.h"

#include "Input.h"
#include "Camera.h"

#include "Application/Scene/FirstScene.h"

#include "Common/Time/Timer.h"
#include "Application/Rendering/ImguiRenderingHelper.h"

const uint32_t WIDTH = 1920;
const uint32_t HEIGHT = 1080;

using namespace Flux;


std::shared_ptr<Input> mInput;
static bool pauseInput = false;
static int32_t pauseKey = GLFW_KEY_TAB;

void cursorpos_callback(GLFWwindow* window, double xpos, double ypos)
{
	static bool firstFrame = true;
	static double lastPosX = 0;
	static double lastPosY = 0;

	if (!pauseInput)
	{
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
	else
	{
		lastPosX = xpos;
		lastPosY = ypos;
		mInput->lastMousePos.x = lastPosX;
		mInput->lastMousePos.y = lastPosY;
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (!pauseInput)
	{
		mInput->KeyboardInput(key, scancode, action, mods);
	}
	else
	{
		if (key == pauseKey)
		{
			mInput->KeyboardInput(key, scancode, action, mods);
		}
	}
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
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	mWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	glfwSetWindowUserPointer(mWindow, this);
	glfwSetFramebufferSizeCallback(mWindow, framebufferResizeCallback);
	glfwSetCharCallback(mWindow, character_callback);
	glfwSetKeyCallback(mWindow, key_callback);
	glfwSetCursorPosCallback(mWindow, cursorpos_callback);
	mInput = std::make_shared<Input>();

	std::shared_ptr<FirstScene> mScene = std::make_shared<FirstScene>(mInput);
	mRenderer = std::make_unique<CustomRenderer>(mWindow);

	mRenderer->SetWindow(mWindow);
	mRenderer->Init();



	std::shared_ptr<Timer> tTimer = std::make_shared<Timer>();
	tTimer->Reset();
	float tDeltaTime = 0.0f;

	//ImGuiIO& io = ImGui::GetIO();


	mScene->Init();
	while (!glfwWindowShouldClose(mWindow)) {

		tDeltaTime = static_cast<float>(tTimer->GetDelta() * 0.001);

		glfwPollEvents();

		if (mInput->GetKeyHeld(GLFW_KEY_ESCAPE))
		{
			break;
		}

		if (mInput->GetKeyUp(pauseKey))
		{
			pauseInput = !pauseInput;
		}

		mScene->Update(tDeltaTime);
		mRenderer->Draw(mScene);


		mInput->Update(pauseInput);
	}
	mScene->Cleanup();

	mRenderer->WaitIdle();
	mRenderer->Cleanup();
}
