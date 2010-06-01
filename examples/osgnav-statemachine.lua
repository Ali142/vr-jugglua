#!/home/rpavlik/src/vrjugglua-build/src/vrjlua

require("Navigator")
require("osgTools")
require("StateMachine")

-- Define application-wide data --
navtransform = osg.PositionAttitudeTransform()

-- Define the osgnav "state" --

osgnav = {}
osgnav.position = osg.Vec3d(0, 0, 0)

function osgnav:enter()
	-- Set up the state
	print("Setting up position interface")
	local wand = gadget.PositionInterface("VJWand")

	print("Setting up digital interface")
	local button = gadget.DigitalInterface("VJButton0")
	
	local button2 = gadget.DigitalInterface("VJButton1")

	print("Creating navigator")
	self.nav = Navigator.create(maxspeed)
	Navigator.useWandTranslation(self.nav, wand, button)

	print("Setting up scenegraph")
	navtransform:addChild(osgLua.loadObjectFile('/home/rpavlik/src/wiimote-modelrot/models/cessna.osg'))

	print("Attaching to scene")
	StateMachine.getScene():addChild(navtransform)
	
	-- Set up the events
	self.events = {}
	self.events[function() button2:wasJustPressed() end] = function() StateMachine.moveToState(simplerotation) end
	print(tostring(self.events))
	for event, action in pairs(self.events) do
		print(tostring(event) .. " --> " .. tostring(action))
	end
end

function osgnav:update(dt)
	self.position = osgTools.subVec(self.position, self.nav:getTranslation(dt, self.position))
	navtransform:setPosition(self.position)
end

-- Define the simplerotation "state"
simplerotation = {}
simplerotation.degrees = 0
function simplerotation:enter()
	local button2 = gadget.DigitalInterface("VJButton1")
	self.events = {}
	self.events[function() button2:wasJustPressed() end] = function() StateMachine.moveToState(osgnav) end
end

function simplerotation:update(dt)
	degreesPerSec = 15
	simplerotation.degrees = simplerotation.degrees + degreesPerSec * dt
	navtransform.setRotate(osg.Quat(simplerotation.degrees,  osg.Vec3d(0, 1, 0))	)
end

-- Define the events --





-- Start up the app --

print("Loading config files into kernel")
StateMachine.loadConfigFile("standalone.jconf")

StateMachine.defineStartingState(osgnav)
StateMachine.runApp()
StateMachine.waitForStop()