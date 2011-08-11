/** @file
	@brief Header defining type trait indicating whether a given type is
	an OSG reference object type.

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
#ifndef INCLUDED_osgLuaBind_isRefType_h_GUID_146a2ea2_6bb9_40c7_a2fc_093b0088c484
#define INCLUDED_osgLuaBind_isRefType_h_GUID_146a2ea2_6bb9_40c7_a2fc_093b0088c484

// Internal Includes
#include "osgLuaBind_traitBase.h"
#include "osgLuaBind_RefBase.h"

// Library/third-party includes
#include <osg/Object>

#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_base_of.hpp>

#include <luabind/lua_state_fwd.hpp>

// Standard includes
// - none

// lua_State is a commonly-passed incomplete type: let Boost know it's never derived from anything.
#include <boost/type_traits/detail/bool_trait_def.hpp>
namespace boost {
	BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC2_1(typename T, is_base_and_derived, T, lua_State, false)
}
#include <boost/type_traits/detail/bool_trait_undef.hpp>

namespace osgLuaBind {
	using boost::enable_if;
	using boost::is_base_of;

	template <typename T, typename Enable = void>
	struct IsRefType : TraitFalse {};

	template <typename T>
	struct IsRefType <
			T,
			typename enable_if <
			typename is_base_of< detail::RefBase, T>::type
			>::type
			> : TraitTrue {};

} // end of namespace osgLuaBind

#endif // INCLUDED_osgLuaBind_isRefType_h_GUID_146a2ea2_6bb9_40c7_a2fc_093b0088c484
