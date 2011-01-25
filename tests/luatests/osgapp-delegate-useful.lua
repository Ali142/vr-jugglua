print("Loading lua file")
print("CTEST_FULL_OUTPUT")

testapp = {counter = 0,
	max = 10}


function testapp:initScene()
	print("In initScene")
end


function testapp:preFrame(dt)
	print("In preFrame")
	self.counter = self.counter + 1
	print("Counter at " .. tostring(self.counter))
	if self.counter >= self.max then
		print("Counter at maximum")
		vrjKernel.stop()
	end
end

print("App delegate defined, now creating OsgAppProxy")
testapp.appProxy = vrjApp.OsgAppProxy()

print ("Now setting OsgAppProxy's delegate")
testapp.appProxy:setAppDelegate(testapp)

print("Loading config files into kernel")
vrjKernel.loadConfigFile("standalone.jconf")

print("Setting kernel application")
testapp.appProxy:setActiveApplication()

print("Starting kernel")
vrjKernel.enter()
