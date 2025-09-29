workspace "scex"
	architecture "x64"
	startproject "scex"

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

project "scex"
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
		"_CRT_SECURE_NO_WARNINGS"
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
		"vendor/stb",
		"vendor/cpp-subprocess"
	}

	links 
	{ 
		"GLFW",
		"Glad",
		"ImGui",
		-- "opengl32.lib"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"SCEX_PLATFORM_WINDOWS",
			"GLFW_INCLUDE_NONE"
		}

	filter "configurations:Debug"
		defines "SCEX_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "SCEX_RELEASE"
		runtime "Release"
		optimize "on"

	filter { "action:gmake" }
		buildoptions { "-fopenmp" }
		linkoptions { "-fopenmp" }

	filter { "action:vs2022" }
		buildoptions { "/openmp" }
