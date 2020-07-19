#include "Application.h"

#include "Renderer.h"
#include "Input.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

using namespace Flux;


std::shared_ptr<Input> mInput;

void cursorpos_callback(GLFWwindow* window, double xpos, double ypos)
{
	mInput->MouseMoveInput(xpos, ypos);
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
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	mWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	glfwSetWindowUserPointer(mWindow, this);
	glfwSetFramebufferSizeCallback(mWindow, framebufferResizeCallback);
	glfwSetCharCallback(mWindow, character_callback);
	glfwSetKeyCallback(mWindow, key_callback);
	glfwSetCursorPosCallback(mWindow, cursorpos_callback);

	mRenderer = std::make_unique<Renderer>();
	mRenderer->SetWindow(mWindow);
	mRenderer->Init();

	while (!glfwWindowShouldClose(mWindow)) {
		glfwPollEvents();
		mRenderer->Draw();

	}

	mRenderer->WaitIdle();

}
