/*
	osgLua: use Lua to access dynamically to osg using osgIntrospection
	Copyright(C) 2006 Jose L. Hidalgo Valiño (PpluX) (pplux at pplux.com)

    This library is open source and may be redistributed and/or modified under
    the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
    (at your option) any later version.  The full license is in LICENSE file
    included with this distribution, and on the openscenegraph.org website.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    OpenSceneGraph Public License for more details.
*/

#ifndef OSGLUA_MAIN
#define OSGLUA_MAIN

#if defined(__cplusplus) && !defined(BUILD_LUA_AS_CPP)
extern "C" {
#endif
	#include "lua.h"
	int luaopen_osgLua(lua_State *L);
	int luaopen_libosgLua(lua_State *L);
#if defined(__cplusplus) && !defined(BUILD_LUA_AS_CPP)
}
#endif
namespace osgLua {

	bool loadWrapper(lua_State *L, const char *name);
	int lua_loadWrapper(lua_State *L);
	int lua_loadObjectFile(lua_State *L);
}

#endif
