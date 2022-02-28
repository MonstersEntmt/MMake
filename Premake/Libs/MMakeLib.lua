if not libs then libs = {} end
if not libs.MMakeLib then
	libs.MMakeLib = {
		name       = "",
		location   = ""
	}
end

local MMakeLib = libs.MMakeLib

require("CMakeInterpreter")

require("../ThirdParty/Piccolo")
require("../../ThirdParty/CommonCLI/Premake/Libs/CommonCLI")

function MMakeLib:setup()
	self.name     = common:projectName()
	self.location = common:projectLocation()

	kind("StaticLib")
	common:outDirs(true)

	includedirs({ self.location .. "/Inc/" })

	libs.CMakeInterpreter:setupDep()
	libs.CommonCLI:setupDep()
	libs.Piccolo:setupDep()

	files({
		self.location .. "/Inc/**",
		self.location .. "/Src/**"
	})
	removefiles({ "*.DS_Store" })
end

function MMakeLib:setupDep()
	links({ self.name })
	sysincludedirs({ self.location .. "/Inc/" })
	libs.CMakeInterpreter:setupDep()
	libs.CommonCLI:setupDep()
	libs.Piccolo:setupDep()
end
