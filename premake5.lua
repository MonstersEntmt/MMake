-- This file is only temporary until the MMake can generate a build system for itself.
-- Then using MMake it will pregenerate Visual Studio solution and gnu make files as a fallback.
newoption({
	trigger = "no-cmake-compiler",
	description = "Disables CMake Compiler, making MMake unable to run CMake scripts"
})
newoption({
	trigger = "no-tests",
	description = "Disables Tests"
})

local integrateCMakeCompiler = true
local buildTests = true
if _OPTIONS["no-cmake-compiler"] then
	integrateCMakeCompiler = false
end
if _OPTIONS["no-tests"] then
	buildTests = false
end

workspace("MMake")
	configurations({ "Debug", "Release", "Dist" })
if _OS == "macosx" then
	platforms({ "x64" })
else
	platforms({ "x86", "x64" })
end

	cdialect("C99")
	cppdialect("C++20")
	rtti("Off")
	exceptionhandling("Off")
	flags("MultiProcessorCompile")

	filter("configurations:Debug")
		defines({ "PREMAKE_CONFIG=PREMAKE_CONFIG_DEBUG" })
		optimize("Off")
		symbols("On")

	filter("configurations:Release")
		defines({ "PREMAKE_CONFIG=PREMAKE_CONFIG_RELEASE" })
		optimize("Full")
		symbols("On")

	filter("configurations:Dist")
		defines({ "PREMAKE_CONFIG=PREMAKE_CONFIG_DIST" })
		optimize("Full")
		symbols("Off")

	filter("system:windows")
		toolset("msc")
		defines({
			"PREMAKE_SYSTEM=PREMAKE_SYSTEM_WINDOWS",
			"NOMINMAX", -- Windows.h disabled
			"WIN32_LEAN_AND_MEAN",
			"_CRT_SECURE_NO_WARNINGS"
		})

	filter("system:macosx")
		defines({ "PREMAKE_SYSTEM=PREMAKE_SYSTEM_MACOSX" })

	filter("system:linux")
		defines({ "PREMAKE_SYSTEM=PREMAKE_SYSTEM_LINUX" })

	filter("toolset:msc")
		defines({ "PREMAKE_TOOLSET=PREMAKE_TOOLSET_MSVC" })

	filter("toolset:clang")
		defines({ "PREMAKE_TOOLSET=PREMAKE_TOOLSET_CLANG" })

	filter("toolset:gcc")
		defines({ "PREMAKE_TOOLSET=PREMAKE_TOOLSET_GCC" })

	filter("platforms:x86")
		defines({ "PREMAKE_PLATFORM=PREMAKE_PLATFORM_X86" })

	filter("platforms:x64")
		defines({ "PREMAKE_PLATFORM=PREMAKE_PLATFORM_AMD64" })

	filter("platforms:arm")
		defines({ "PREMAKE_PLATFORM=PREMAKE_PLATFORM_ARM32" })

	filter("platforms:arm64")
		defines({ "PREMAKE_PLATFORM=PREMAKE_PLATFORM_ARM64" })

	filter({})

	targetdir("%{wks.location}/Bin/%{cfg.system}-%{cfg.platform}-%{cfg.buildcfg}/")
	objdir("%{wks.location}/Bin/Int-%{cfg.system}-%{cfg.platform}-%{cfg.buildcfg}/%{prj.name}/")

	if buildTests then
		startproject("Tests")
	else
		startproject("MMake")
	end

	group("ThirdParty")
	project("CommonCLI")
		location("%{wks.location}/ThirdParty/")
		kind("StaticLib")
		warnings("Off")

		includedirs({
			"%{prj.location}/CommonCLI/Inc/",
			"%{prj.location}/CommonCLI/Src/"
		})

		files({
			"%{prj.location}/CommonCLI/Inc/**",
			"%{prj.location}/CommonCLI/Src/**"
		})

	project("Piccolo")
		location("%{wks.location}/ThirdParty/")
		kind("StaticLib")
		warnings("Off")

		includedirs({
			"%{prj.location}/Piccolo/"
		})

		files({
			"%{prj.location}/Piccolo/**"
		})

	group("Libs")
	if integrateCMakeCompiler then
		project("CMakeCompiler")
			location("%{wks.location}/CMakeCompiler/")
			kind("StaticLib")

			includedirs({ "%{prj.location}/Inc/" })

			files({
				"%{prj.location}/Inc/**",
				"%{prj.location}/Src/**"
			})
	end

	project("MMakeLib")
		location("%{wks.location}/MMakeLib/")
		kind("StaticLib")

		includedirs({ "%{prj.location}/Inc/" })

		if integrateCMakeCompiler then
			defines({ "MMAKE_CMAKE_COMPILER" })
			links({ "CMakeCompiler" })
			sysincludedirs({ "%{wks.location}/CMakeCompiler/Inc/" })
		end

		links({
			"CommonCLI",
			"Piccolo"
		})
		sysincludedirs({ 
			"%{wks.location}/ThirdParty/CommonCLI/Inc/",
			"%{wks.location}/ThirdParty/"
		})

		files({
			"%{prj.location}/Inc/**",
			"%{prj.location}/Src/**"
		})

	group("MMake")
	project("MMake")
		location("%{wks.location}/MMake/")
		kind("ConsoleApp")

		includedirs({ "%{prj.location}/Src/" })

		if integrateCMakeCompiler then
			defines({ "MMAKE_CMAKE_COMPILER" })
			sysincludedirs({ "%{wks.location}/CMakeCompiler/Inc/" })
		end
		links({
			"MMakeLib",
			"CommonCLI"
		})
		sysincludedirs({
			"%{wks.location}/MMakeLib/Inc/",
			"%{wks.location}/ThirdParty/CommonCLI/Inc/"
		})

		files({ "%{prj.location}/Src/**" })

	if buildTests then
		group("Tests")
		project("Tests")
			location("%{wks.location}/Tests/")
			kind("ConsoleApp")

			includedirs({ "%{prj.location}/Src/" })

			files({ "%{prj.location}/Src/**" })
	end
