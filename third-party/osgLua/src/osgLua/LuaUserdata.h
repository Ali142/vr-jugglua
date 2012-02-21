/** @file
	@brief Header

	@date 2011

	@author
	Ryan Pavlik
	<rpavlik@iastate.edu> and <abiryan@ryand.net>
	http://academic.cleardefinition.com/
	Iowa State University Virtual Reality Applications Center
	Human-Computer Interaction Graduate Program
*/

//          Copyright Iowa State University 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef INCLUDED_LuaUserdata_h_GUID_52d4ca22_cd5a_420e_b6fd_f46db9eb1f93
#define INCLUDED_LuaUserdata_h_GUID_52d4ca22_cd5a_420e_b6fd_f46db9eb1f93

// Internal Includes
#ifdef LUACPP_LUA_INCLUDE_FULL
#include LUACPP_LUA_INCLUDE_FULL
#else
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
#endif

// Library/third-party includes
// - none

// Standard includes
#include <typeinfo>
#include <new>
#include <cassert>
#include <cstring>

#ifdef ENABLE_LUA_USERDATA_VERBOSE
#	include <iostream>
#	define LUA_USERDATA_VERBOSE(X) {std::cerr << "LuaUserdata.h:" << __LINE__ << ":\t" << X << std::endl;}
#else
#	define LUA_USERDATA_VERBOSE(X) {}
#endif

#ifdef ENABLE_LUA_USERDATA_STACKCHECKER
#	include "LuaStackChecker.h"
#	define LUA_USERDATA_STACKCHECKER(NAME, STATE, EXPECTEDDIFF) StackChecker<> NAME(STATE, __FILE__, __LINE__, EXPECTEDDIFF)
#else
#	define LUA_USERDATA_STACKCHECKER(NAME, STATE, EXPECTEDDIFF)
#endif

namespace luacpputils {
	/// base class providing string constants.
	/// @internal
	class LuaUserdataBase {
		public:
			static const char * indexMetamethodName() {
				static const char ret[] = "__index";
				return ret;
			}
			static const char * gcMetamethodName() {
				static const char ret[] = "__gc";
				return ret;
			}

			template<typename T>
			inline static const char * getRegistryString() {
				return typeid(T).name();
			}
	};

	namespace UserdataStoragePolicies {
		struct StorePointerInUserdata {
				template<typename T>
				struct apply {
					public:
						inline static T * userdataToInstance(void * userdata) {
							T* instance;
							std::memcpy(&instance, userdata, sizeof(T*));
							return instance;
						}

						inline static T * allocateMemoryForInstance(lua_State * L) {
							LUA_USERDATA_STACKCHECKER(checker, L, 1);
							T * p;
							try {
								p = static_cast<T*>(operator new(sizeof(T)));
							} catch (...) {
								return NULL;
							}
							void * ud = lua_newuserdata(L, sizeof(T*));
							if (!ud) {
								throw std::bad_alloc();
							}
							std::memcpy(ud, &p, sizeof(T*));
							return p;
						}

						inline static int destroyInstance(lua_State * L) {
							void * udPtr = luaL_checkudata(L, 1, LuaUserdataBase::getRegistryString<T>());
							LUA_USERDATA_VERBOSE("Userdata " << userdataToInstance(udPtr) << ":\t" << "Deleting instance of type " << LuaUserdataBase::getRegistryString<T>());
							delete userdataToInstance(udPtr);
							return 0;
						}
				};
		};

		struct StoreInstanceInUserdata {
				template<typename T>
				struct apply {
					public:
						inline static T * userdataToInstance(void * userdata) {
							return static_cast<T*>(userdata);
						}

						inline static T * allocateMemoryForInstance(lua_State * L) {
							LUA_USERDATA_STACKCHECKER(checker, L, 1);
							void * ud = lua_newuserdata(L, sizeof(T));
							if (!ud) {
								throw std::bad_alloc();
							}
							return static_cast<T*>(ud);
						}

						inline static int destroyInstance(lua_State * L) {
							void * udPtr = luaL_checkudata(L, 1, LuaUserdataBase::getRegistryString<T>());
							LUA_USERDATA_VERBOSE("Userdata " << udPtr << ":\t" << "Deleting instance of type " << LuaUserdataBase::getRegistryString<T>());
							userdataToInstance(udPtr)->~T();
							return 0;
						}
				};

		};
	} // end of namespace UserdataStoragePolicies

	/// Template class to derive from (using the CRTP) to allow pushing
	/// with a metatable. Intended for Lua-specific classes, not wrapping
	/// other C++ classes.
	template<typename Derived, typename StoragePolicy = UserdataStoragePolicies::StorePointerInUserdata>
	class LuaUserdata : private LuaUserdataBase {
		public:
			typedef int (Derived::*NonConstInstanceMethodPtrType)(lua_State *);
			typedef int (Derived::*ConstInstanceMethodPtrType)(lua_State *) const;
			typedef Derived * PointerToDerivedType;
			typedef Derived ** DerivedPtrPtr;
			typedef Derived DerivedType;

			typedef typename StoragePolicy::template apply<Derived> Storage;

		private:

			inline static const char * _getRegistryString() {
				return getRegistryString<Derived>();
			}

			inline static void _pushMetatable(lua_State * L) {
				LUA_USERDATA_STACKCHECKER(checker, L, 1);
				if (luaL_newmetatable(L, _getRegistryString())) {
					LUA_USERDATA_VERBOSE("Userdata type " << _getRegistryString << ":\t" << "Registering garbage collection metamethod");
					lua_pushvalue(L, -1);
					lua_pushcfunction(L, &Storage::destroyInstance);
					lua_setfield(L, -2, gcMetamethodName()); /// table is one below the top of the stack
					lua_pop(L, 1); /// pop the metatable off the stack.
					{
						LUA_USERDATA_STACKCHECKER(derivedchecker, L, 0);
						Derived::registerAdditionalMetamethods(L);
					}
				}
			}

			inline static void _pushIndexTable(lua_State * L) {
				LUA_USERDATA_STACKCHECKER(checker, L, 1);
				_pushMetatable(L);

				int metatableIndex = lua_gettop(L);
				lua_getfield(L, metatableIndex, indexMetamethodName());
				if (lua_isnil(L, -1)) {
					lua_pop(L, 1);
					lua_newtable(L);
					lua_pushvalue(L, -1); /// now the top two values are the __index table
					lua_setfield(L, metatableIndex, indexMetamethodName());
				} else if (!lua_istable(L, -1)) {
					luaL_error(L, "While registering an instance method of %s: __index metamethod already set to value of type %s!", _getRegistryString(), lua_typename(L, -1));
					return;
				}
				/// OK, past this point we have the __index table at -1.

				/// Remove the metatable from the stack so that all we have pushed is the __index table
				lua_remove(L, -2);
			}

			inline static void _setMetatable(lua_State * L) {
				LUA_USERDATA_VERBOSE("Userdata " << instance << ":\t" << "Created instance of type " << _getRegistryString());
				_pushMetatable(L);
				lua_setmetatable(L, -2);
			}

		protected:
			template<typename PtrToMemberFuncType>
			class InstanceMethodHandler {
				private:
					template<PtrToMemberFuncType M>
					static void * _getMethodDescription() {
						return reinterpret_cast<void *>(&_callInstanceMethod<M>);
					}

					template<PtrToMemberFuncType M>
					static int _callInstanceMethod(lua_State * L) {
						void * ud = luaL_checkudata(L, 1, _getRegistryString());
						if (!ud) {
							return luaL_error(L, "Trying to call an instance method of %s, but first argument is not an instance!", _getRegistryString());
						}
						PointerToDerivedType instance = Storage::userdataToInstance(ud);

						LUA_USERDATA_VERBOSE("Userdata " << instance << ":\tMethod " << _getMethodDescription<M>() << "\t" << "Before instance method call with lua_gettop(L)==" << lua_gettop(L));
						/// Leaving the instance on the stack in case the method wants access to it.
						int ret = ((*instance).*(M))(L);
						LUA_USERDATA_VERBOSE("Userdata " << instance << ":\tMethod " << _getMethodDescription<M>() << "\t" << "After instance method call returning " << ret);
						return ret;
					}



				public:
					template<PtrToMemberFuncType M>
					static void pushInstanceMethod(lua_State * L, int upvalues = 0) {
						LUA_USERDATA_STACKCHECKER(checker, L, 1);
						LUA_USERDATA_VERBOSE("Userdata type " << _getRegistryString() << ":\tMethod " << _getMethodDescription<M>() << "\t" << "Pushing an instance method");
						lua_pushcclosure(L, &_callInstanceMethod<M>, upvalues);
					}

					template<PtrToMemberFuncType M>
					static void registerMetamethod(lua_State * L, const char * metamethodName) {
						LUA_USERDATA_STACKCHECKER(checker, L, 0);
						LUA_USERDATA_VERBOSE("Userdata type " << _getRegistryString << ":\tMethod " << _getMethodDescription<M>() << "\t" << "Registering metamethod " << metamethodName);
						_pushMetatable(L);
						pushInstanceMethod<M>(L);
						lua_setfield(L, -2, metamethodName); /// table is one below the top of the stack
						lua_pop(L, 1); /// pop the metatable off the stack.
					}

					template<PtrToMemberFuncType M>
					static void registerObjectMethod(lua_State * L, const char * methodName) {
						LUA_USERDATA_STACKCHECKER(checker, L, 0);
						LUA_USERDATA_VERBOSE("Userdata type " << _getRegistryString << ":\tMethod " << _getMethodDescription<M>() << "\t" << "Registering object method " << methodName);
						_pushIndexTable(L);
						pushInstanceMethod<M>(L);
						lua_setfield(L, -2, methodName); /// table is one below the top of the stack
						lua_pop(L, 1); /// pop the __index table off the stack.
					}
			};

			typedef InstanceMethodHandler<NonConstInstanceMethodPtrType> NonConstInstanceMethod;
			typedef InstanceMethodHandler<ConstInstanceMethodPtrType> ConstInstanceMethod;

			static PointerToDerivedType pushNewWithLuaParam(lua_State * L) {
				LUA_USERDATA_STACKCHECKER(checker, L, 1);
				PointerToDerivedType p = Storage::allocateMemoryForInstance(L);
				try {
					new(p) DerivedType(L);
				} catch (...) {
					return NULL;
				}
				_setMetatable(L);
				return p;
			}

			static PointerToDerivedType pushNew(lua_State * L) {
				LUA_USERDATA_STACKCHECKER(checker, L, 1);
				PointerToDerivedType p = Storage::allocateMemoryForInstance(L);
				try {
					new(p) DerivedType();
				} catch (...) {
					return NULL;
				}
				_setMetatable(L);
				return p;
			}

			template<typename T1>
			static PointerToDerivedType pushNew(lua_State * L, T1 a1) {
				LUA_USERDATA_STACKCHECKER(checker, L, 1);
				PointerToDerivedType p = Storage::allocateMemoryForInstance(L);
				try {
					new(p) DerivedType(a1);
				} catch (...) {
					return NULL;
				}
				_setMetatable(L);
				return p;
			}

			template<typename T1, typename T2>
			static PointerToDerivedType pushNew(lua_State * L, T1 a1, T2 a2) {
				LUA_USERDATA_STACKCHECKER(checker, L, 1);
				PointerToDerivedType p = Storage::allocateMemoryForInstance(L);
				try {
					new(p) DerivedType(a1, a2);
				} catch (...) {
					return NULL;
				}
				_setMetatable(L);
				return p;
			}

			template<typename T1, typename T2, typename T3>
			static PointerToDerivedType pushNew(lua_State * L, T1 a1, T2 a2, T3 a3) {
				LUA_USERDATA_STACKCHECKER(checker, L, 1);
				PointerToDerivedType p = Storage::allocateMemoryForInstance(L);
				try {
					new(p) DerivedType(a1, a2, a3);
				} catch (...) {
					return NULL;
				}
				_setMetatable(L);
				return p;
			}

			static void pushCreatorFunction(lua_State * L) {
				LUA_USERDATA_STACKCHECKER(checker, L, 1);
				lua_pushcfunction(L, pushNewWithLuaParam);
			}

			/// Implement your own method to override this if you want to
			/// register more than just __gc. You can call
			///  - NonConstInstanceMethod::registerMetamethod<>()
			///  - NonConstInstanceMethod::registerObjectMethod<>()
			///  - NonConstInstanceMethod::pushInstanceMethod<>()
			///  - ConstInstanceMethod::registerMetamethod<>()
			///  - ConstInstanceMethod::registerObjectMethod<>()
			///  - ConstInstanceMethod::pushInstanceMethod<>()
			/// from within your method, as desired.
			static void registerAdditionalMetamethods(lua_State *) {}
	};


}

#undef LUA_USERDATA_STACKCHECKER
#undef LUA_USERDATA_VERBOSE

#endif // INCLUDED_LuaUserdata_h_GUID_52d4ca22_cd5a_420e_b6fd_f46db9eb1f93
