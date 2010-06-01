/**	@file	OsgAppProxy.cpp
	@brief Application proxy object implementation

	@date
	2009-2010

	@author
	Ryan Pavlik
	<rpavlik@iastate.edu> and <abiryan@ryand.net>
	http://academic.cleardefinition.com/
	Iowa State University Virtual Reality Applications Center
	Human-Computer Interaction Graduate Program
*/

// Internal Includes
#include "OsgAppProxy.h"

#include "ConvertOsgPtrToOsgLuaPtr.h"

#include "vrjLuaOutput.h"

// Library/third-party includes
#include <luabind/class.hpp>

#include <osg/LightModel>

// Standard includes
#include <cstring>
#include <cstdlib>

namespace vrjLua {
/// Initialize static member variable
OsgAppProxy* OsgAppProxy::_pApp = NULL;

void OsgAppProxy::bindToLua(LuaStatePtr & state) {
	// Bind this class
	if (state) {
#ifdef VERBOSE
		VRJLUA_MSG_START(dbgVRJLUA_PROXY, MSG_STATUS)
			<< "Registering vrj.OsgAppProxy with Lua"
			<< VRJLUA_MSG_END(dbgVRJLUA_PROXY, MSG_STATUS);
#endif
		using namespace luabind;
		module(state.get(), "vrjApp") [
			class_<OsgAppProxy>("OsgAppProxy")
				.def(constructor<>())
				.def("setAppDelegate", & OsgAppProxy::setAppDelegate)
				.def("getAppDelegate", & OsgAppProxy::getAppDelegate)
				.def("setActiveApplication", & OsgAppProxy::setActiveApplication)
				.def("getScene", & OsgAppProxy::getScene)
				.def("getTimeDelta", & OsgAppProxy::getTimeDelta)
	   ];
	}
}

OsgAppProxy::OsgAppProxy() :
	vrj::OsgApp(vrj::Kernel::instance()),
	_timeDelta(-1),
	_delegationSuccessFlag(true) {

}

OsgAppProxy::OsgAppProxy(vrj::Kernel* kern/*, int & argc, char** argv*/) :
	vrj::OsgApp(kern),
	_timeDelta(-1),
	_delegationSuccessFlag(true) {
	/// update static pointer to app object
	_pApp = this;

}

OsgAppProxy::~OsgAppProxy() {

}

void OsgAppProxy::setActiveApplication() {
	vrj::Kernel::instance()->setApplication(this);
}


void OsgAppProxy::setAppDelegate(luabind::object const & delegate) {
	/// @todo test here to see if the passed delegate is suitable

	if (luabind::type(delegate) == LUA_TTABLE) {
#ifdef VERBOSE
	VRJLUA_MSG_START(dbgVRJLUA_PROXY, MSG_STATUS)
		<< "App delegate set."
		<< VRJLUA_MSG_END(dbgVRJLUA_PROXY, MSG_STATUS);
#endif
		_delegate = delegate;

		// New delegate gets a fresh attempt
		_delegationSuccessFlag = true;

	} else {
		VRJLUA_MSG_START(dbgVRJLUA_PROXY, MSG_ERROR)
			<< "Lua app tried to set an invalid app delegate: " << delegate
			<< VRJLUA_MSG_END(dbgVRJLUA_PROXY, MSG_ERROR);
	}
}

luabind::object const & OsgAppProxy::getAppDelegate() {
	return _delegate;
}

void OsgAppProxy::initScene() {
#ifdef VERBOSE
	VRJLUA_MSG_START(dbgVRJLUA_PROXY, MSG_STATUS)
		<< "------- OsgAppProxy::initScene() -------"
		<< VRJLUA_MSG_END(dbgVRJLUA_PROXY, MSG_STATUS);
#endif

	_lastPreFrameTime.setNow();

	// Exit now if there's no delegate set
	if (!_delegate) {
		VRJLUA_MSG_START(dbgVRJLUA_PROXY, MSG_WARNING)
			<< "No delegate has been set yet - exiting to avoid busy-waiting forever."
			<< VRJLUA_MSG_END(dbgVRJLUA_PROXY, MSG_WARNING);
		//vrj::Kernel::instance()->stop();
		std::exit(1);
	}

	// Create the top level node of the tree
	_rootNode = new osg::Group();
	std::cerr << "Number of children: " << _rootNode->getNumChildren() << std::endl;
	_forwardCallToDelegate("initScene");
	std::cerr << "Number of children: " << _rootNode->getNumChildren() << std::endl;
}

void OsgAppProxy::configSceneView(osgUtil::SceneView* newSceneViewer) {
	vrj::OsgApp::configSceneView(newSceneViewer);
		newSceneViewer->getLight()->setAmbient(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
		newSceneViewer->getLight()->setDiffuse(osg::Vec4(0.9f,0.9f,0.9f,1.0f));
		newSceneViewer->getLight()->setSpecular(osg::Vec4(1.0f,1.0f,1.0f,1.0f));

		// setup the ambient light the way I want it
		osg::LightModel* lightmodel = new osg::LightModel;
		lightmodel->setAmbientIntensity(osg::Vec4(1.0f,1.0f,1.0f,1.0f));

		newSceneViewer->getGlobalStateSet()->setAttributeAndModes(lightmodel, osg::StateAttribute::ON);
}

void OsgAppProxy::preFrame() {
	if (!_delegationSuccessFlag) {
		// We made it through a whole frame without a successful call
		// to the delegate - bail out.
		VRJLUA_MSG_START(dbgVRJLUA_PROXY, MSG_ERROR)
				<< "A full frame loop completed without any successful calls to the delegate."
				<< VRJLUA_MSG_END(dbgVRJLUA_PROXY, MSG_ERROR);
		VRJLUA_MSG_START(dbgVRJLUA_PROXY, MSG_ERROR)
				<< "Assuming application failure, bailing out."
				<< VRJLUA_MSG_END(dbgVRJLUA_PROXY, MSG_ERROR);
		std::exit(1);
	}
	_delegationSuccessFlag = false;
	vpr::Interval cur_time = vrj::Kernel::instance()->getUsers()[0]->getHeadUpdateTime();
	vpr::Interval diff_time(cur_time - _lastPreFrameTime);
	if (_lastPreFrameTime.getBaseVal() >= cur_time.getBaseVal()) {
		diff_time.secf(0.0f);
	}

	_timeDelta = diff_time.secd();

	_forwardCallToDelegate("preFrame");

	_lastPreFrameTime = cur_time;
}

void OsgAppProxy::latePreFrame() {
	_forwardCallToDelegate("latePreFrame");

	// Finish updating the scene graph.
	vrj::OsgApp::latePreFrame();
}

void OsgAppProxy::intraFrame() {
	_forwardCallToDelegate("latePreFrame");
}

void OsgAppProxy::postFrame() {
	_forwardCallToDelegate("postFrame");
}

osg::Group* OsgAppProxy::getScene() {
	return _rootNode.get();
}

double OsgAppProxy::getTimeDelta() {
	return _timeDelta;
}

bool OsgAppProxy::_forwardCallToDelegate(const char * call) {
	if (_delegate && luabind::type(_delegate[call]) == LUA_TFUNCTION) {
		try {
			_delegate[call](_delegate);

			// We had at least one success this frame
			_delegationSuccessFlag = true;
			return true;
		} catch (luabind::error & e) {
			VRJLUA_MSG_START(dbgVRJLUA_PROXY, MSG_ERROR)
				<< "Calling '" << call << "' in the delegate failed - check your lua code!"
				<< VRJLUA_MSG_END(dbgVRJLUA_PROXY, MSG_ERROR);
			luabind::object o(luabind::from_stack(e.state(), -1));
			VRJLUA_MSG_START(dbgVRJLUA_PROXY, MSG_ERROR)
				<< "Top of the Lua stack (error message) is: '" << o << "'"
				<< VRJLUA_MSG_END(dbgVRJLUA_PROXY, MSG_ERROR);
		}
		return false;
	}
	VRJLUA_MSG_START(dbgVRJLUA_PROXY, MSG_DRIVEL)
		<< "Delegate not valid or no '" << call << "' element defined: " << _delegate
		<< VRJLUA_MSG_END(dbgVRJLUA_PROXY, MSG_DRIVEL);
	return false;
}

} // end of vrjLua namespace
