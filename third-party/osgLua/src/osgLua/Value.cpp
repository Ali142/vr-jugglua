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

#include "Value.h"
#include "Value_metamethods.h"
#include "Type.h"
#include "lua_functions.h"

#include <osgIntrospection/Reflection>
#include <osgIntrospection/MethodInfo>
#include <osgIntrospection/ConstructorInfo>
#include <osgIntrospection/PropertyInfo>
#include <osgIntrospection/Utility>
#include <osgIntrospection/ExtendedTypeInfo>
#include <osgIntrospection/variant_cast>

#include <osg/NodeVisitor>

#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Matrix>

namespace osgLua {
	static const char OSGLUAVALUEMETATABLE[] = "__isOsgLuaValue";

	static int osgLuaTypeTag(lua_State *L) {
		lua_pushboolean(L, true);
		return 1;
	}

	Value::Value( const osgIntrospection::Value &v ) : _value(v)
	{
		
		static const osgIntrospection::Type &t_referenced = 
		  osgIntrospection::Reflection::getType(extended_typeid<osg::Referenced>());

		const osgIntrospection::Type &type = v.getType();
		if (type.isDefined())
		if (type.isNonConstPointer())
		{
			const osgIntrospection::Type &p_type = type.getPointedType();
			if (p_type.isDefined())
			if (p_type.isSubclassOf(t_referenced))
			{
				_referenced = 
					osgIntrospection::variant_cast<osg::Referenced*>(_value);
			}
		}
	}

	Value::~Value()
	{
	}

	osgIntrospection::Value getValue(lua_State *L, int index)
	{
		int top = lua_gettop(L);
		index = (index>0)? index : top + index + 1;

		if (lua_isuserdata(L, index))
		{
			Value *v = Value::get(L, index);
			if (v) return v->get();
			else
			{
				luaL_error(L,
					"userdata can not be used as osgIntrospection::Value");
			}
		}

		if (lua_isnumber(L, index))
		{
			lua_Integer vi = lua_tointeger(L, index);
			lua_Number  vf = lua_tonumber(L, index);

			if ( ( (lua_Number)vi) == vf )
			{
				return osgIntrospection::Value( vi );
			}
			return osgIntrospection::Value( vf );
		}

		if (lua_isstring(L, index))
		{
			return osgIntrospection::Value(
				std::string(lua_tostring(L, index))
			);
		}

		if (lua_isboolean(L, index))
		{
			return osgIntrospection::Value( (lua_toboolean(L, index) == 0) ? false : true);
		}
		
		luaL_error(L,
			"Value at %d(%s) can not be used as osgIntrospection::Value",
			index, lua_typename(L, lua_type(L, index)));

		return osgIntrospection::Value();
	}

	void push_stdString(lua_State *L, const std::string &s)
	{
		lua_pushstring(L, s.c_str());
	}

	void Value::push(lua_State *L, const osgIntrospection::Value &original)
	{
		#define basic_type(TYPE, NAME, FUNCTION) \
		static const osgIntrospection::Type& NAME = \
		  osgIntrospection::Reflection::getType(extended_typeid<TYPE>()); \
			if ( original.getType() == NAME ) \
			{ \
				FUNCTION(L, (TYPE) \
					osgIntrospection::getInstance<TYPE>(original)); \
				return;	\
			} 

		basic_type(bool, tbool,  lua_pushboolean)
		basic_type(int, tint, lua_pushinteger)
		basic_type(unsigned int, tuint, lua_pushinteger)
		basic_type(lua_Integer, tlua_int, lua_pushinteger)
		basic_type(float, tfloat, lua_pushnumber)
		basic_type(double, tdouble, lua_pushnumber)
		basic_type(lua_Number, tlnumber, lua_pushnumber)
		basic_type(const char*, tcchar, lua_pushstring)
		basic_type(const std::string, tstd_string, push_stdString)

		#undef basic_type

		// if not is a basic type...
		// create a userdata
		Value** data = (Value**) lua_newuserdata(L, sizeof(Value*));
		
		// create/get the metatable
		if(luaL_newmetatable(L,original.getType().getQualifiedName().c_str()))
		{
			std::cout << "First time pushing " << original.getType().getQualifiedName() << " to Lua - creating metatable!" << std::endl;
			// tag this as an osgLua value
			lua_pushcfunction(L, osgLuaTypeTag);
			lua_setfield(L, -2, OSGLUAVALUEMETATABLE);
			
			lua_pushcfunction(L, Value::gc);
			lua_setfield(L, -2, "__gc");	
			
			lua_pushcfunction(L, Value::index);
			lua_setfield(L, -2, "__index");
			
			lua_pushcfunction(L, Value::newindex);
			lua_setfield(L, -2, "__newindex");
			
			if (original.getType().getReaderWriter()) {
				/// If we know how to turn it into a string
				lua_pushcfunction(L, metamethods::tostring);
			} else {
				lua_pushcfunction(L, metamethods::minimal_tostring);
			}
			
			lua_setfield(L, -2, "__tostring");
			
			/// Bind mathematically-inclined values specially
		  	bool success = false;
			#define BIND_VECTOR(TYPE) \
			if (!success) { \
		  		success = Vector::bind_metamethods<TYPE>(L, original); \
		  	}
		  	
		  	BIND_VECTOR(osg::Vec4d)
		  	BIND_VECTOR(osg::Vec4f)
		  	BIND_VECTOR(osg::Vec4ub)
		  	BIND_VECTOR(osg::Vec4s)
		  	BIND_VECTOR(osg::Vec4)
		  	BIND_VECTOR(osg::Vec3d)
		  	BIND_VECTOR(osg::Vec3f)
		  	BIND_VECTOR(osg::Vec3s)
		  	BIND_VECTOR(osg::Vec3b)
		  	BIND_VECTOR(osg::Vec3)
		  	BIND_VECTOR(osg::Vec2d)
		  	BIND_VECTOR(osg::Vec2f)
		  	BIND_VECTOR(osg::Vec2b)
		  	BIND_VECTOR(osg::Vec2s)
		  	BIND_VECTOR(osg::Vec2)
		  	
		  	#undef BIND_VECTOR


			#define BIND_MATRIX(TYPE) \
			if (!success) { \
		  		success = Matrix::bind_metamethods<TYPE>(L, original); \
		  	}
		  	
		  	BIND_MATRIX(osg::Matrixd)
		  	BIND_MATRIX(osg::Matrixf)
		  	BIND_MATRIX(osg::Matrix)
		  	
		  	#undef BIND_MATRIX
		  		
		}
		lua_setmetatable(L, -2);

		Value *v = new Value(original); //lua's copy
		*data = v;
	}

	inline Value* Value::rawGet(lua_State *L, int index )
	{
		Value **value = reinterpret_cast<Value**>(lua_touserdata(L,index));
		if (value) { return *value; }
		return 0;
	}

	Value* Value::get(lua_State *L, int index )
	{
		int top = lua_gettop(L);
		index = (index>0)? index : top + index + 1;

		if (lua_isuserdata(L, index))
		{
			lua_getmetatable(L, index);
			lua_pushstring(L, OSGLUAVALUEMETATABLE);
			lua_gettable(L, -2);
			if (lua_tocfunction(L,-1) == &osgLuaTypeTag)
			{
				Value *v = rawGet(L,index);
				lua_settop(L,top);
				return v;
			}
		}
		lua_settop(L,top);
		return 0;
	}
	
	Value* Value::getRequired(lua_State *L, int index )
	{
		Value *a = Value::get(L,index);
		if (a == 0) {
			luaL_error(L, "%s:%d Expected a osgLua userdata but get %s",
				__FILE__,__LINE__, lua_typename(L,lua_type(L, index)) ) ;
		}
		return a;
	}
	
	int Value::gc(lua_State *L)
	{
		Value *v = rawGet(L,1);
		delete v;
		return 0;
	}
	
	int Value::index(lua_State *L)
	{
		Value *v = rawGet(L,1);
		if (lua_isstring(L,2))
		{
			const osgIntrospection::Type &type = v->getType();
			if (!type.isDefined())
			{
				luaL_error(L, "Type not defined %s", 
					type.getStdTypeInfo().name());
			}
			//std::string cname = type.getQualifiedName();
			
			const char * memberName = lua_tostring(L, 2);
			std::string memName(memberName);
			osgIntrospection::PropertyInfoList props;
			type.getAllProperties(props);
			if (props.size() > 0) {
				for (unsigned int i = 0; i < props.size(); ++i) {
					if (props[i]->getName() == memName) {
						if (props[i]->isIndexed()) {
							/// @todo implement indexed properties
							luaL_error(L, "Indexed properties are not yet implemented in osgLua");
						} else if (!props[i]->canGet()) {
							luaL_error(L, "Property %s defined as not gettable", props[i]->getName().c_str());
						} else {
							//std::cout << "Getting a property named " << props[i]->getName() << std::endl;
							osgIntrospection::Value propVal = props[i]->getValue(v->get());
							Value::push(L, propVal);
							return 1;
						}
					}
				}
			}
			
			lua_pushvalue(L,2); // copy the name
			lua_pushcclosure(L, Value::methodCall,1);
			return 1;
		}
		// maybe ... if is an integer... access indexed data
		return 0;
	}
	
	int Value::newindex(lua_State *L) {
		
		Value *v = rawGet(L,1);
		
		Value *newVal = get(L,3);
		if (lua_isstring(L,2))
		{
			const osgIntrospection::Type &type = v->getType();
			if (!type.isDefined())
			{
				luaL_error(L, "Type not defined %s", 
					type.getStdTypeInfo().name());
			}
			//std::string cname = type.getQualifiedName();
			
			const char * memberName = lua_tostring(L, 2);
			std::string memName(memberName);
			osgIntrospection::PropertyInfoList props;
			type.getAllProperties(props);
			if (props.size() > 0) {
				for (unsigned int i = 0; i < props.size(); ++i) {
					if (props[i]->getName() == memName) {
						if (props[i]->isIndexed()) {
							/// @todo implement indexed properties
							luaL_error(L, "Indexed properties are not yet implemented in osgLua");
						} else if (!props[i]->canSet()) {
							luaL_error(L, "Property %s defined as not settable", props[i]->getName().c_str());
						} else {
							std::cout << "Setting a property named " << props[i]->getName() << std::endl;
							props[i]->setValue(v->get(), newVal->get());
							return 0;
						}
					}
				}
			}
			
			luaL_error(L, "No property %s defined in %s", memberName, type.getQualifiedName().c_str());
		
		}
		// maybe ... if is an integer... access indexed data
		return 0;	
	}
	
	int Value::methodCall(lua_State *L)
	{
		int top = lua_gettop(L);

		Value *value = Value::get(L,1);
		if (value == 0)
		{
			luaL_error(L, "%s:%d Expected a osgLua userdata but get %s",
				__FILE__,__LINE__, lua_typename(L,lua_type(L, 1)) ) ;
		}

		osgIntrospection::ValueList vl;
		for(int i = 2; i <= top; ++i)
		{
			vl.push_back( getValue(L, i) );
		}

		try
		{
			std::string name( lua_tostring(L, lua_upvalueindex(1)) );
			const osgIntrospection::MethodInfo *method = 0;
			const osgIntrospection::Type &type = value->getType();
			method = type.getCompatibleMethod (name,vl, true);
			/* This code is no longer needed if getCompatibleMethod
			 * finds methods in base types correctly.
			if (!method)
			{
				//manual method finding... d'oh!
				for(int i = 0; i < type.getNumBaseTypes() && !method; ++i)
				{
					const osgIntrospection::Type &base = 
						type.getBaseType(i);
					if (!base.isDefined()) continue;
					method = base.getCompatibleMethod(name,vl,false);
				}
			}
			*/

			if (!method)
			{
				if (vl.back().getType().isNonConstPointer() &&
						vl.back().getInstanceType().isSubclassOf(osgIntrospection::Reflection::getType("osg::NodeVisitor"))) {
					// OK, we have a pointer to a visitor, let's search again after dereferencing
					osgIntrospection::Value vPointer = vl.back();
					vl.pop_back();
					// Dereference the pointer
					vl.push_back(*osgIntrospection::variant_cast<osg::NodeVisitor*>(vPointer));

					// Search again for the method
					method = type.getCompatibleMethod (name,vl, true);
				}
			}
			if (!method)
			{
				// Couldn't find a method
				int top = lua_gettop(L);
				lua_pushfstring(L, "Error method %s::%s(",
					type.getName().c_str(),
					lua_tostring(L,lua_upvalueindex(1))
					);
				for( osgIntrospection::ValueList::iterator
					i = vl.begin(); i != vl.end(); ++i)
				{
					lua_pushstring(L, i->getType().getName().c_str() );
					lua_pushstring(L,",");
				}
				if (!vl.empty()) lua_pop(L,1);
				lua_pushstring(L,") not found");
				lua_concat(L,lua_gettop(L) - top);
				lua_error(L);

			}

			// OK, we got a method!
			osgIntrospection::Value returnedval = method->invoke(value->get(), vl);
			Value::push(L, returnedval);
			return 1;
		}
		catch(osgIntrospection::Exception &e)
		{
			luaL_error(L,"[%s:%d] %s",__FILE__,__LINE__,e.what().c_str());
		}
		return 0;
			
	}

	int Value::getTypeInfo(lua_State *L)
	{
		const osgIntrospection::Type *type = 0;
		Value *v = Value::get(L,1);
		if (v) type = &(v->getType());
		else if (lua_isstring(L,1))
		{
			try
			{
				const osgIntrospection::Type &t = 
					osgIntrospection::Reflection::getType(lua_tostring(L,1));
				type = &t;
			}
			catch (osgIntrospection::TypeNotFoundException&)
			{
				return 0;
			}
		}

		if (type) 
		{
			pushTypeInfo(L, type);
			return 1;
		}

		return 0;
	}

	int Value::getTypes(lua_State *L)
	{
		const osgIntrospection::TypeMap &map = 
			osgIntrospection::Reflection::getTypes();
		int counter = 1;

		lua_newtable(L);

		for (osgIntrospection::TypeMap::const_iterator i = map.begin(); 
				i != map.end(); ++i)
		{
			osgIntrospection::Type *type = i->second;
			if (!type->isDefined() || type->isPointer() ) continue;
			lua_pushstring(L, type->getQualifiedName().c_str());
			lua_rawseti(L,-2,counter);
			counter++;
			
		}
		return 1;	
	}

	int Value::createByName(lua_State *L)
	{
		int top = lua_gettop(L);
		std::string name( luaL_checkstring(L, 1) );
		try
		{
			const osgIntrospection::Type &type = 
				osgIntrospection::Reflection::getType(name);
		
			osgIntrospection::ValueList vl;
			for(int i = 2; i <= top; ++i)
			{
				vl.push_back( getValue(L, i) );
			}

			osgIntrospection::Value returnedval = type.createInstance(vl);

			Value::push(L, returnedval);
			return 1;
		}
		catch (osgIntrospection::Exception &e)
		{
			luaL_error(L,"[%s:%d] %s",__FILE__,__LINE__,e.what().c_str());
		}
		return 0;
	}

} // end of osgLua namespace
