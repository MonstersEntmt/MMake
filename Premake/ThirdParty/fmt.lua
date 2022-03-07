if not libs then libs = {} end
if not libs.fmt then
	libs.fmt = {
		name       = "",
		location   = ""
	}
end

local fmt = libs.fmt

function fmt:setup()
	self.name     = common:projectName()
	self.location = common:projectLocation()

	kind("Utility")
	common:outDirs(true)

	includedirs({ self.location .. "/include/" })

	files({ })
	removefiles({ "*.DS_Store" })
end

function fmt:setupDep()
	sysincludedirs({ self.location .. "/include/" })
end