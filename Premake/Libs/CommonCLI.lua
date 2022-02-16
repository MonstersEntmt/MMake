if not libs then libs = {} end
if not libs.CommonCLI then
	libs.CommonCLI = {
		name       = "",
		location   = ""
	}
end

local CommonCLI = libs.CommonCLI

function CommonCLI:setup()
	self.name     = common:projectName()
	self.location = common:projectLocation()

	kind("StaticLib")
	common:outDirs(true)

	includedirs({
		self.location .. "/Inc/",
		self.location .. "/Src/"
	})

	files({
		self.location .. "/Inc/**",
		self.location .. "/Src/**"
	})
	removefiles({ "*.DS_Store" })
end

function CommonCLI:setupDep()
	links({ self.name })
	sysincludedirs({ self.location .. "/Inc/" })
end
