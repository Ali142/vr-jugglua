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

//          Copyright Iowa State University 2010-2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)


#pragma once
#ifndef INCLUDED_osgLuaBind_getQualifiedName_h_GUID_767d0293_c779_4289_b145_a88a276e28ef
#define INCLUDED_osgLuaBind_getQualifiedName_h_GUID_767d0293_c779_4289_b145_a88a276e28ef

// Internal Includes
// - none

// Library/third-party includes
#include <osgLua/introspection/Type>
#include <osgLua/introspection/Reflection>

// Standard includes
#include <string>

namespace osgLuaBind {

	/// Get the human-readable fully-qualified type name of the static type
	/// passed as the template parameter, retrieved from osgIntrospection wrappers.
	template <typename T>
	inline std::string getQualifiedName() {
		static const ::osgLua::introspection::Type& destType =
		    ::osgLua::introspection::Reflection::getType(extended_typeid<T>());
		return destType.getQualifiedName();
	}

	/// Get the human-readable fully-qualified type name of an osgIntrospection value
	/// at runtime, retrieved from osgIntrospection wrappers.
	inline std::string getQualifiedName(::osgLua::introspection::Value const& v) {
		return v.getType().getQualifiedName();
	}

} // end of namespace osgLuaBind

#endif // INCLUDED_osgLuaBind_getQualifiedName_h_GUID_767d0293_c779_4289_b145_a88a276e28ef
