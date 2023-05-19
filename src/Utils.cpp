#include "Utils.h"

#include <codecvt>

#include <stb_image.h>

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
