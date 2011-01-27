/**	@file	binding_detail/BindvrjLuaToLua.inl
	@brief	implementation of convenience binding

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
#include "BindvrjLuaToLua.h"
#include <vrjugglua/VRJLuaOutput.h>
#include <vrjugglua/LuaPath.h>

// Library/third-party includes
#include <luabind/luabind.hpp>

#include <osgDB/Registry>

// Standard includes
#ifdef VERBOSE
#include <iostream>
#endif

namespace vrjLua {
using namespace luabind;

static void appendToModelSearchPath(std::string const& path) {
	osgDB::Registry::instance()->getDataFilePathList().push_back(path);
}

static void appendToLuaRequirePath(LuaStateRawPtr s, std::string const& path) {
	LuaPath::instance().addLuaRequirePath(borrowStatePtr(s), path);
}

void BindvrjLuaToLua(LuaStatePtr state) {
#ifdef VERBOSE
	std::cerr << "Registering vrjLua module functions with Lua..." << std::flush << std::endl;
#endif
	module(state.get(), "vrjLua") [
		def("appendToModelSearchPath", &appendToModelSearchPath),
		def("appendToLuaRequirePath", &appendToLuaRequirePath),
		def("safePrint", &LuaScript::doPrint)
	];


}

}// end of vrjLua namespace
