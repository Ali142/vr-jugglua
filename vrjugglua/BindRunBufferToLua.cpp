/**	@file	BindRunBufferToLua.cpp
	@brief	implementation

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
#include "BindRunBufferToLua.h"
#include "SynchronizedRunBuffer.h"

// Library/third-party includes
#include <luabind/luabind.hpp>
#include <boost/shared_ptr.hpp>

// Standard includes
#ifdef VERBOSE
#include <iostream>
#endif

namespace vrjLua {
using namespace luabind;

void bindRunBufferToLua(LuaStatePtr state) {
#ifdef VERBOSE
	std::cerr << "Registering vrjSync.RunBuffer object with Lua..." << std::flush << std::endl;
#endif
	module(state.get(), "vrjSync") [
		class_<SynchronizedRunBuffer, boost::shared_ptr<SynchronizedRunBuffer> >("RunBuffer")
			.def(constructor<luabind::object>())
			.def("addFile", &SynchronizedRunBuffer::addFile)
			.def("addString", &SynchronizedRunBuffer::addString)
			.def("runBuffer", &SynchronizedRunBuffer::runBuffer)
	];

}

}// end of vrjLua namespace
