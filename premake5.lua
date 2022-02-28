-- This file is only temporary until the MMake can generate a build system for itself.
-- Then using MMake it will pregenerate Visual Studio solution and gnu make files as a fallback.
require("Premake/Common")

require("Premake/Libs/CMakeInterpreter")
require("Premake/Libs/MMakeLib")

require("ThirdParty/CommonCLI/Premake/Libs/CommonCLI")
require("Premake/ThirdParty/Piccolo")

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
		warnings("Extra")
		libs.CMakeInterpreter:setup()

	project("MMakeLib")
		location("MMakeLib/")
		warnings("Extra")
		libs.MMakeLib:setup()

	group("MMake")
	project("MMake")
		location("MMake/")
		kind("ConsoleApp")
		warnings("Extra")

		common:outDirs()
		common:debugDir()

		includedirs({ "%{prj.location}/Src/" })

		libs.MMakeLib:setupDep()

		files({ "%{prj.location}/Src/**" })
		removefiles({ "*.DS_Store" })

	group("Tests")
	project("Tests")
		location("%{wks.location}/Tests/")
		kind("ConsoleApp")
		warnings("Extra")

		common:outDirs()
		common:debugDir()

		libs.MMakeLib:setupDep()

		includedirs({ "%{prj.location}/Src/" })

		files({
			"%{prj.location}/Run/**",
			"%{prj.location}/Src/**"
		})
		removefiles({ "*.DS_Store" })