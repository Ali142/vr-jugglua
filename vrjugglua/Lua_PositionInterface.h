/**	@file	Lua_PositionInterface.h
	@brief	PositionInterface wrapper class for LuaBind

	@date
	2009-2010

	@author
	Ryan Pavlik
	<rpavlik@iastate.edu> and <abiryan@ryand.net>
	http://academic.cleardefinition.com/
	Iowa State University Virtual Reality Applications Center
	Human-Computer Interaction Graduate Program
*/

#pragma once
#ifndef INCLUDED_vrjugglua_Lua_PositionInterface_h
#define INCLUDED_vrjugglua_Lua_PositionInterface_h

// Internal Includes
#include "LuaScript.h"

// Library/third-party includes
#include <gadget/Type/PositionInterface.h>

#include <gmtl/Matrix.h>

#include <osg/Matrix>
#include <osg/Vec3f>

// Standard includes
// - none

namespace vrjLua {
	class PositionInterface {
		public:
			PositionInterface();

			void init(const std::string & device);

			osg::Matrixf getMatrix();
			osg::Vec3f getPosition();
			osg::Vec3f getForwardVector();

		protected:
			gmtl::Matrix44f _getData(const float scale = gadget::PositionUnitConversion::ConvertToFeet);

			bool _ready;
			gadget::PositionInterface _iface;
	};

#ifndef LUABIND_COMBINED_COMPILE
	void bindPositionInterfaceToLua(LuaStatePtr state);
#endif
} // end of vrjLua namespace

#endif // INCLUDED_vrjugglua_Lua_PositionInterface_h
