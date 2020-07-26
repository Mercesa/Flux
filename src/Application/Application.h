#pragma once

#include <memory>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


namespace Flux
{
	class Renderer;
class Application
{
public:
	Application();
	~Application();
	
	void Run();
private:
	GLFWwindow* mWindow;


	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto app = reinterpret_cast<Flux::Application*>(glfwGetWindowUserPointer(window));
		app->mResized = true;
	}

	bool mResized;

	std::unique_ptr<Renderer> mRenderer;
	
protected:
};
}
