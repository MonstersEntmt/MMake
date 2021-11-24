-- This file is only temporary until the MMake can generate a build system for itself.
-- Then using MMake it will pregenerate Visual Studio solution and gnu make files as a fallback.
newoption({
	trigger = "no-tests",
	description = "Disables Tests"
})

local buildTests = true
if _OPTIONS["no-tests"] then
	buildTests = false
end

workspace("MMake")
	configurations({ "Debug", "Release", "Dist" })
if _TARGET_OS == "macosx" then
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

	targetdir("%{wks.location}/Bin/Int-%{cfg.system}-%{cfg.platform}-%{cfg.buildcfg}/%{prj.name}/")
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
		removefiles({ "*.DS_Store" })

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
		removefiles({ "*.DS_Store" })

	group("Libs")
	project("CMakeInterpreter")
		location("%{wks.location}/CMakeInterpreter/")
		kind("StaticLib")

		includedirs({ "%{prj.location}/Inc/" })

		files({
			"%{prj.location}/Inc/**",
			"%{prj.location}/Src/**"
		})
		removefiles({ "*.DS_Store" })

	project("MMakeLib")
		location("%{wks.location}/MMakeLib/")
		kind("StaticLib")

		includedirs({ "%{prj.location}/Inc/" })

		links({
			"CommonCLI",
			"Piccolo",
			"CMakeInterpreter"
		})
		sysincludedirs({ 
			"%{wks.location}/ThirdParty/CommonCLI/Inc/",
			"%{wks.location}/ThirdParty/",
			"%{wks.location}/CMakeInterpreter/Inc/"
		})

		files({
			"%{prj.location}/Inc/**",
			"%{prj.location}/Src/**"
		})
		removefiles({ "*.DS_Store" })

	group("MMake")
	project("MMake")
		location("%{wks.location}/MMake/")
		kind("ConsoleApp")
		targetdir("%{wks.location}/Bin/%{cfg.system}-%{cfg.platform}-%{cfg.buildcfg}/")

		includedirs({ "%{prj.location}/Src/" })

		links({
			"MMakeLib",
			"CommonCLI"
		})
		sysincludedirs({
			"%{wks.location}/MMakeLib/Inc/",
			"%{wks.location}/ThirdParty/CommonCLI/Inc/",
			"%{wks.location}/CMakeInterpreter/Inc/"
		})

		files({ "%{prj.location}/Src/**" })
		removefiles({ "*.DS_Store" })

	if buildTests then
		group("Tests")
		project("Tests")
			location("%{wks.location}/Tests/")
			kind("ConsoleApp")
			targetdir("%{wks.location}/Bin/%{cfg.system}-%{cfg.platform}-%{cfg.buildcfg}/")

			links({ "MMakeLib" })
			sysincludedirs({
				"%{wks.location}/MMakeLib/Inc/",
				"%{wks.location}/ThirdParty/CommonCLI/Inc/",
				"%{wks.location}/CMakeInterpreter/Inc/"
			})

			includedirs({ "%{prj.location}/Src/" })

			files({
				"%{prj.location}/Run/**",
				"%{prj.location}/Src/**"
			})
			removefiles({ "*.DS_Store" })
	end
