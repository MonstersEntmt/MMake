if not libs then libs = {} end
if not libs.Piccolo then
	libs.Piccolo = {
		name       = "",
		location   = ""
	}
end

local Piccolo = libs.Piccolo

function Piccolo:setup()
	self.name     = common:projectName()
	self.location = common:projectLocation()

	kind("StaticLib")
	common:outDirs(true)

	includedirs({ self.location })

	files({ self.location .. "**" })
	removefiles({ "*.DS_Store" })
end

function Piccolo:setupDep()
	links({ self.name })
	sysincludedirs({ self.location })
end
