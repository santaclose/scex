#include <GLFW/glfw3.h>
#include <filesystem>
#include <iostream>
#include <vector>
#include <glad/glad.h>

#include <Utils.h>
#include <PathUtils.h>
#include <ImGuiController.h>

#ifdef STE_PLATFORM_WINDOWS
#include <windows.h>
#include <atlstr.h>
#endif

#define REDRAW_COUNT 5
#define BACKGROUND_COLOR 0.1

#ifdef STE_DEBUG
void APIENTRY glDebugOutput(GLenum source,
	GLenum type,
	uint32_t id,
	GLenum severity,
	GLsizei length,
	const char* message,
	const void* userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}
#endif

#ifdef STE_PLATFORM_WINDOWS
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	LPWSTR* szArglist;
	int argc;
	szArglist = CommandLineToArgvW(GetCommandLine(), &argc);
	std::string firstArgument = CW2A(szArglist[0]);
	std::string secondArgument;
	if (argc > 1) secondArgument = CW2A(szArglist[1]);
#else
int main(int argc, char** argv)
{
	std::string firstArgument(argv[0]);
	std::string secondArgument;
	if (argc > 1) secondArgument = std::string(argv[1]);
#endif

	PathUtils::SetProgramDirectory(firstArgument);

	int redrawCounter = REDRAW_COUNT;

	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

#ifdef STE_DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

	window = glfwCreateWindow(argc > 1 ? 720 : 1280, 720, "ste", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	Utils::SetWindowIcon(PathUtils::GetAssetsDirectory() + "icon32.png", window);

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	if (argc > 1)
		ste::ImGuiController::Setup(window, secondArgument);
	else
		ste::ImGuiController::Setup(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "[Renderer] Failed to initialize OpenGL context (GLAD)" << std::endl;
		return false;
	}
	int openglMajor, openglMinor;
	glGetIntegerv(GL_MAJOR_VERSION, &openglMajor);
	glGetIntegerv(GL_MINOR_VERSION, &openglMinor);
	std::cout << "OpenGL version: " << openglMajor << '.' << openglMinor << std::endl;
	float openglVersionFloat = openglMajor + (openglMinor / 10.0f);

	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

#ifdef STE_DEBUG
	int flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if ((flags & GL_CONTEXT_FLAG_DEBUG_BIT) && openglVersionFloat > 4.2)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
	else
		std::cout << "Skipping OpenGL debug hook, version older than 4.3\n";
#endif

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Draw scene */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ste::ImGuiController::Tick();

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		if (redrawCounter == 0)
		{
			glfwWaitEvents();
			redrawCounter = REDRAW_COUNT;
		}
		else
			redrawCounter--;
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}