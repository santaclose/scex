workspace "ste"
	architecture "x64"
	startproject "ste"

	configurations
	{
		"Debug",
		"Release"
	}
	
	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Projects
group "Dependencies"
	include "vendor/glfw"
	include "vendor/Glad"
	include "vendor/imgui"

group ""

project "ste"
	kind "WindowedApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.hpp",
		"src/**.cpp",
		"vendor/ImGuiColorTextEdit/*.cpp",
		"vendor/ImGuiColorTextEdit/vendor/regex/src/**.cpp",
		"vendor/load.cpp"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE"
	}

	includedirs
	{
		"src",
		"vendor/glfw/include",
		"vendor/Glad/include",
		"vendor/imgui",
		"vendor/ImGuiColorTextEdit",
		"vendor/ImGuiColorTextEdit/vendor/regex/include",
		"vendor/json",
		"vendor/portable-file-dialogs",
		"vendor/stb"
	}

	links 
	{ 
		"glfw",
		"Glad",
		"ImGui"
	}

	filter "system:windows"
		systemversion "latest"
		defines { "STE_PLATFORM_WINDOWS" }
		buildoptions { "/openmp" }
		links { "opengl32.lib" }

	filter "system:Unix"
		systemversion "latest"
		defines { "STE_PLATFORM_LINUX" }
		links { "GL" }

	filter "configurations:Debug"
		defines "STE_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "STE_RELEASE"
		runtime "Release"
		optimize "on"
