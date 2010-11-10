/**	@file	osgLuaBind_refType.h
	@brief	header for repeated include by osgLuaBind.h only

	@date
	2009-2010

	@author
	Ryan Pavlik
	<rpavlik@iastate.edu> and <abiryan@ryand.net>
	http://academic.cleardefinition.com/
	Iowa State University Virtual Reality Applications Center
	Human-Computer Interaction Graduate Program
*/

#ifdef OSG_QUALIFIED_TYPENAME

// Local includes
// - none

// Library/third-party includes
// - none

// Standard includes
#include <iostream>

namespace luabind
{
    template <>
    struct default_converter<OSG_QUALIFIED_TYPENAME*>
      : native_converter_base<OSG_QUALIFIED_TYPENAME*>
    {
        static int compute_score(lua_State* L, int index)
        {
			/// @todo actually figure out what's good and what's not
			osgLua::Value * v = osgLua::Value::get(L, index);
			if (!v) {
				return -1;
			}
			static const osgIntrospection::Type& destType =
				osgIntrospection::Reflection::getType(extended_typeid<OSG_QUALIFIED_TYPENAME*>());
			if (v->get().getType() == destType) {
				std::cout << "Exact match for type!" << std::endl;
				return 1;
			} else if (v->get().getType().isSubclassOf(destType)) {
				std::cout << "Subclass match for type." << std::endl;
				return 0;
			}
        	return -1;
            return lua_type(L, index) == LUA_TUSERDATA ? 0 : -1;
        }
        
        int match(lua_State* L, detail::by_pointer<OSG_QUALIFIED_TYPENAME>, int index)
		{
		    return compute_score(L, index);
		}

        OSG_QUALIFIED_TYPENAME* from(lua_State* L, int index)
        {
        	osgLua::Value * v = osgLua::Value::get(L, index);
			if (!v) {
				return NULL;
			}
        	return osgIntrospection::variant_cast<OSG_QUALIFIED_TYPENAME*>(v->get());
        }
        
        OSG_QUALIFIED_TYPENAME* apply(lua_State* L, detail::by_pointer<OSG_QUALIFIED_TYPENAME>, int index)
		{
			return from(L, index);
		}

        void to(lua_State* L, OSG_QUALIFIED_TYPENAME* const& x)
        {
        	osgLua::Value::push(L, x);
        }
    };

    template <>
    struct default_converter<OSG_QUALIFIED_TYPENAME* const&>
      : default_converter<OSG_QUALIFIED_TYPENAME*>
    {};

}

#undef OSG_QUALIFIED_TYPENAME

#endif // ifdef OSG_QUALIFIED_TYPENAME
