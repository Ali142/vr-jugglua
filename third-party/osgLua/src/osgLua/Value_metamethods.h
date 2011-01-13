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

#ifndef OSGLUA_VALUE_METAMETHODS
#define OSGLUA_VALUE_METAMETHODS

#include "Value.h"
#include <vrjugglua/LuaIncludeFull.h>

#include <osgIntrospection/Value>
#include <osgIntrospection/Type>
#include <osg/Referenced>
#include <osg/ref_ptr>

namespace osgLua {


	namespace detail {
		template<class T>
		T addVectors(osgIntrospection::Value const& a, osgIntrospection::Value const& b) {
			return osgIntrospection::variant_cast<T>(a) + osgIntrospection::variant_cast<T>(b);
		}
		
		template<class T>
		T subtractVectors(osgIntrospection::Value const& a, osgIntrospection::Value const& b) {
			return osgIntrospection::variant_cast<T>(a) - osgIntrospection::variant_cast<T>(b);
		}
		
		template<class T>
		T scaleVector(osgIntrospection::Value const& vector, double scalar) {
			return osgIntrospection::variant_cast<T>(vector) * scalar;
		}
		
		template<class T>
		T multMatrices(osgIntrospection::Value const& a, osgIntrospection::Value const& b) {
			return osgIntrospection::variant_cast<T>(a) * osgIntrospection::variant_cast<T>(b);
		}
	
	} // end of namespace detail

	
	namespace metamethods {
		int tostring(lua_State *L);
		
		int eq(lua_State *L);
		int lt(lua_State *L);
		int le(lua_State *L);
	}
	
	namespace Vector {
	
		template<class T>
		int add(lua_State *L) {
			Value *a = Value::getRequired(L,1);
			Value *b = Value::getRequired(L,2);

			const osgIntrospection::Type &typeA = a->getType();
			const osgIntrospection::Type &typeB = b->getType();

			static const osgIntrospection::Type& myType = 
		  		osgIntrospection::Reflection::getType(extended_typeid<T>());
			if (typeA == myType && typeB == myType) {
		  		osgIntrospection::Value ret = detail::addVectors<T>(a->get(), b->get());
		  		Value::push(L, ret);
		  		return 1;
		  	} else {
				luaL_error(L,"[%s:%d] Could not add instance of %s, %s",__FILE__,__LINE__, typeA.getQualifiedName().c_str(), typeB.getQualifiedName().c_str());
			}
			return 0;
		}
		
		template<class T>
		int sub(lua_State *L) {
			Value *a = Value::getRequired(L,1);
			Value *b = Value::getRequired(L,2);

			const osgIntrospection::Type &typeA = a->getType();
			const osgIntrospection::Type &typeB = b->getType();

			static const osgIntrospection::Type& myType = 
		  		osgIntrospection::Reflection::getType(extended_typeid<T>());
			if (typeA == myType && typeB == myType) {
		  		osgIntrospection::Value ret = detail::subtractVectors<T>(a->get(), b->get());
		  		Value::push(L, ret);
		  		return 1;
		  	} else {
				luaL_error(L,"[%s:%d] Could not subtract instance of %s, %s",__FILE__,__LINE__, typeA.getQualifiedName().c_str(), typeB.getQualifiedName().c_str());
			}
			return 0;
		}
		
		template<class T>
		int unm(lua_State *L) {
			Value *a = Value::getRequired(L,1);
	  		osgIntrospection::Value ret = detail::scaleVector<T>(a->get(), -1);
	  		Value::push(L, ret);
	  		return 1;
		}
		
		template<class T>
		int scale(lua_State *L) {
			static const osgIntrospection::Type& tdouble =
		  		osgIntrospection::Reflection::getType(extended_typeid<double>());

			double scalar;
			Value * vector = Value::get(L,1);
			if (vector == 0) {
				if (!lua_isnumber(L, 1)) {
					luaL_error(L, "%s:%d Expected a number but get %s",
						__FILE__,__LINE__, lua_typename(L,lua_type(L, 1)) );
				}
				scalar = lua_tonumber(L, 1);
				vector = Value::getRequired(L,2);
			} else {
				if (!lua_isnumber(L, 2)) {
					luaL_error(L, "%s:%d Expected a number but get %s",
						__FILE__,__LINE__, lua_typename(L,lua_type(L, 2)) );
				}
				scalar = lua_tonumber(L, 2);
			}
			
			osgIntrospection::Value ret = detail::scaleVector<T>(vector->get(), scalar);
	  		Value::push(L, ret);
	  		return 1;
	  	}
	  	
	  	template<class T>
	  	bool bind_metamethods(lua_State *L, const osgIntrospection::Value &original) {
	  		static const osgIntrospection::Type& myType = 
		  		osgIntrospection::Reflection::getType(extended_typeid<T>()); 
		  	if (original.getType() == myType) { 
		  		lua_pushcfunction(L, &Vector::add<T>); 
		  		lua_setfield(L, -2, "__add"); 
		  		lua_pushcfunction(L, &Vector::sub<T>); 
		  		lua_setfield(L, -2, "__sub"); 
		  		lua_pushcfunction(L, &Vector::unm<T>); 
		  		lua_setfield(L, -2, "__unm"); 
		  		lua_pushcfunction(L, &Vector::scale<T>);
		  		lua_setfield(L, -2, "__mul"); 
		  		lua_pushcfunction(L, &metamethods::eq); 
		  		lua_setfield(L, -2, "__eq"); 
		  		lua_pushcfunction(L, &metamethods::lt); 
		  		lua_setfield(L, -2, "__lt"); 
		  		lua_pushcfunction(L, &metamethods::le); 
		  		lua_setfield(L, -2, "__le"); 
		  		return true;
		  	}
		  	return false;
	  	}
	} // end of Vector namespace
	
	namespace Matrix {
		template<class T>
		int mul(lua_State *L) {
			Value *a = Value::getRequired(L,1);
			Value *b = Value::getRequired(L,2);

			const osgIntrospection::Type &typeA = a->getType();
			const osgIntrospection::Type &typeB = b->getType();

			static const osgIntrospection::Type& myType = 
		  		osgIntrospection::Reflection::getType(extended_typeid<T>());
			if (typeA == myType && typeB == myType) {
		  		osgIntrospection::Value ret = detail::multMatrices<T>(a->get(), b->get());
		  		Value::push(L, ret);
		  		return 1;
		  	} else {
				luaL_error(L,"[%s:%d] Could not multiply instance of %s, %s",__FILE__,__LINE__, typeA.getQualifiedName().c_str(), typeB.getQualifiedName().c_str());
			}
			return 0;
		}
		
		template<class T>
		int eq(lua_State *L) {
			Value *a = Value::getRequired(L,1);
			Value *b = Value::getRequired(L,2);

			const osgIntrospection::Type &typeA = a->getType();
			const osgIntrospection::Type &typeB = b->getType();

			static const osgIntrospection::Type& myType = 
		  		osgIntrospection::Reflection::getType(extended_typeid<T>());
			if (typeA == myType && typeB == myType) {
				bool ret = ( osgIntrospection::variant_cast<T>(a->get()) == osgIntrospection::variant_cast<T>(b->get()) );  		
				lua_pushboolean(L, ret);
		  		return 1;
		  	} else {
				luaL_error(L,"[%s:%d] Could not compare instance of %s, %s",__FILE__,__LINE__, typeA.getQualifiedName().c_str(), typeB.getQualifiedName().c_str());
			}
			return 0;
		}
		
		template<class T>
		int lt(lua_State *L) {
			Value *a = Value::getRequired(L,1);
			Value *b = Value::getRequired(L,2);

			const osgIntrospection::Type &typeA = a->getType();
			const osgIntrospection::Type &typeB = b->getType();

			static const osgIntrospection::Type& myType = 
		  		osgIntrospection::Reflection::getType(extended_typeid<T>());
			if (typeA == myType && typeB == myType) {
				bool ret = ( osgIntrospection::variant_cast<T>(a->get()) < osgIntrospection::variant_cast<T>(b->get()) );  		
				lua_pushboolean(L, ret);
		  		return 1;
		  	} else {
				luaL_error(L,"[%s:%d] Could not compare instance of %s, %s",__FILE__,__LINE__, typeA.getQualifiedName().c_str(), typeB.getQualifiedName().c_str());
			}
			return 0;
		}
		
		template<class T>
	  	bool bind_metamethods(lua_State *L, const osgIntrospection::Value &original) {
	  		static const osgIntrospection::Type& myType = 
		  		osgIntrospection::Reflection::getType(extended_typeid<T>()); 
		  	if (original.getType() == myType) { 
		  		lua_pushcfunction(L, &Matrix::mul<T>); 
		  		lua_setfield(L, -2, "__mul"); 
		  		lua_pushcfunction(L, &Matrix::eq<T>); 
		  		lua_setfield(L, -2, "__eq"); 
		  		lua_pushcfunction(L, &Matrix::lt<T>); 
		  		lua_setfield(L, -2, "__lt"); 
		  		return true;
		  	}
		  	return false;
	  	}	
	} // end of Matrix namespace
	
	
	

} // end of osgLua namespace

#endif
