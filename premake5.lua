-- This file is only temporary until the MMake can generate a build system for itself.
-- Then using MMake it will pregenerate Visual Studio solution and gnu make files as a fallback.
require("Premake/Common")

require("Premake/Libs/CommonCLI")
require("Premake/Libs/Piccolo")

newoption({
	trigger = "no-tests",
	description = "Disables Tests"
})

local buildTests = true
if _OPTIONS["no-tests"] then
	buildTests = false
end

workspace("MMake")
	common:setConfigsAndPlatforms()

	common:addCoreDefines()

	cdialect("C99")
	cppdialect("C++20")
	rtti("Off")
	exceptionhandling("Off")
	flags("MultiProcessorCompile")

	if buildTests then
		startproject("Tests")
	else
		startproject("MMake")
	end

	group("Dependencies")
	project("CommonCLI")
		location("ThirdParty/CommonCLI/")
		warnings("Off")
		libs.CommonCLI:setup()
		location("ThirdParty/")

	project("Piccolo")
		location("ThirdParty/Piccolo/")
		warnings("Off")
		libs.Piccolo:setup()
		location("ThirdParty/")

	group("Libs")
	project("CMakeInterpreter")
		location("CMakeInterpreter/")
		kind("StaticLib")
		warnings("Extra")

		common:outDirs(true)

		includedirs({ "%{prj.location}/Inc/" })

		files({
			"%{prj.location}/Inc/**",
			"%{prj.location}/Src/**"
		})
		removefiles({ "*.DS_Store" })

	project("MMakeLib")
		location("MMakeLib/")
		kind("StaticLib")
		warnings("Extra")

		common:outDirs(true)

		includedirs({ "%{prj.location}/Inc/" })

		libs.CommonCLI:setupDep()
		libs.Piccolo:setupDep()

		links({ "CMakeInterpreter" })
		sysincludedirs({ "CMakeInterpreter/Inc/" })

		files({
			"%{prj.location}/Inc/**",
			"%{prj.location}/Src/**"
		})
		removefiles({ "*.DS_Store" })

	group("MMake")
	project("MMake")
		location("MMake/")
		kind("ConsoleApp")
		warnings("Extra")

		common:outDirs()
		common:debugDir()

		includedirs({ "%{prj.location}/Src/" })

		links({ "MMakeLib" })
		sysincludedirs({
			"MMakeLib/Inc/",
			"ThirdParty/CommonCLI/Inc/",
			"%{wks.location}/CMakeInterpreter/Inc/"
		})

		files({ "%{prj.location}/Src/**" })
		removefiles({ "*.DS_Store" })

	if buildTests then
		group("Tests")
		project("Tests")
			location("%{wks.location}/Tests/")
			kind("ConsoleApp")
			warnings("Extra")

			common:outDirs()
			common:debugDir()

			links({ "MMakeLib" })
			sysincludedirs({
				"MMakeLib/Inc/",
				"ThirdParty/CommonCLI/Inc/",
				"CMakeInterpreter/Inc/"
			})

			includedirs({ "%{prj.location}/Src/" })

			files({
				"%{prj.location}/Run/**",
				"%{prj.location}/Src/**"
			})
			removefiles({ "*.DS_Store" })
	end
