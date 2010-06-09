/**	@file	binding_detail/BindKernelToLua.cpp
	@brief	implementation of kernel and print overload binding

	@date
	2009-2010

	@author
	Ryan Pavlik
	<rpavlik@iastate.edu> and <abiryan@ryand.net>
	http://academic.cleardefinition.com/
	Iowa State University Virtual Reality Applications Center
	Human-Computer Interaction Graduate Program
*/

// Local includes
#include "BindKernelToLua.h"
#include <vrjugglua/VRJLuaOutput.h>

// Library/third-party includes
#include <luabind/luabind.hpp>

#include <vrj/Draw/OSG/OsgApp.h>
#include <vrj/Kernel/Kernel.h>

// Standard includes
#ifdef VERBOSE
#include <iostream>
#endif

namespace vrjLua {
using namespace luabind;

namespace Kernel {
	void start() {
		vrj::Kernel::instance()->start();
	}

	void stop() {
		vrj::Kernel::instance()->stop();
	}

	void waitForKernelStop() {
		vrj::Kernel::instance()->waitForKernelStop();
	}

	void setApplication(luabind::object app) {
		vrj::Kernel::instance()->setApplication(luabind::object_cast<vrj::OsgApp*>(app));
	}

	void loadConfigFile(const std::string & fn) {
		vrj::Kernel::instance()->loadConfigFile(fn);
	}

	bool isRunning() {
		return vrj::Kernel::instance()->isRunning();
	}

	void safePrint(const std::string & s) {
		VRJLUA_MSG_START(dbgVRJLUA_APP, MSG_STATUS)
				<< s
				<< VRJLUA_MSG_END(dbgVRJLUA_APP, MSG_STATUS);
	}

} // end of Internal namespace

void bindKernelToLua(LuaStatePtr state) {
#ifdef VERBOSE
	std::cerr << "Registering vrjKernel module functions with Lua..." << std::flush << std::endl;
#endif
	module(state.get(), "vrjKernel") [
		def("start", &Kernel::start),
		def("stop", &Kernel::stop),
		def("setApplication", &Kernel::setApplication),
		def("loadConfigFile", &Kernel::loadConfigFile),
		def("waitForKernelStop", &Kernel::waitForKernelStop),
		def("isRunning", &Kernel::isRunning),
		def("safePrint", &Kernel::safePrint)
	];

	// Hide the print function, using the threadsafe one.
	luaL_dostring(state.get(),
	"do \n\
		local oldprint = print \n\
		print = function(s) \n\
			vrjKernel.safePrint(tostring(s)) \n\
		end \n\
	end");

}

}// end of vrjLua namespace
