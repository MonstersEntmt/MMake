if not libs then libs = {} end
if not libs.CMakeInterpreter then
	libs.CMakeInterpreter = {
		name       = "",
		location   = ""
	}
end

local CMakeInterpreter = libs.CMakeInterpreter

function CMakeInterpreter:setup()
	self.name     = common:projectName()
	self.location = common:projectLocation()

	kind("StaticLib")
	common:outDirs(true)

	includedirs({ self.location .. "/Inc/" })

	files({
		self.location .. "/Inc/**",
		self.location .. "/Src/**"
	})
	removefiles({ "*.DS_Store" })
end

function CMakeInterpreter:setupDep()
	links({ self.name })
	sysincludedirs({ self.location .. "/Inc/" })
end
