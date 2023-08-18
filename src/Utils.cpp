#include "Utils.h"

#include <codecvt>

#include <stb_image.h>
#include <subprocess.hpp>

std::wstring Utils::Utf8ToWstring(const std::string& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.from_bytes(str);
}

void Utils::SetWindowIcon(const std::string& path, GLFWwindow* window)
{
	GLFWimage images[1];
	images[0].pixels = stbi_load(path.c_str(), &images[0].width, &images[0].height, 0, 4); //rgba channels 
	glfwSetWindowIcon(window, 1, images);
	stbi_image_free(images[0].pixels);
}


int Utils::SubprocessCall(const std::string& cmd)
{
	return subprocess::call(cmd);
}

int Utils::SubprocessCall(const std::vector<std::string>& cmd)
{
	return subprocess::call(cmd);
}

std::string Utils::SubprocessCheckOutput(const std::vector<std::string>& cmd)
{
	subprocess::Buffer out = subprocess::check_output(cmd);
	std::string result;
	result.resize(out.length + 1);
	memcpy(result.data(), out.buf.data(), result.size());
	return result;
}