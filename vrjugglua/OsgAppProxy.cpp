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

#include "vrjLuaOutput.h"

// Library/third-party includes
#include <luabind/class.hpp>

// Standard includes
#include <cstring>

namespace vrjLua {
/// Initialize static member variable
OsgAppProxy* OsgAppProxy::_pApp = NULL;

OsgAppProxy::OsgAppProxy() :
	vrj::OsgApp(vrj::Kernel::instance()),
	_timeDelta(-1) {

}

OsgAppProxy::OsgAppProxy(vrj::Kernel* kern/*, int & argc, char** argv*/) :
	vrj::OsgApp(kern),
	_timeDelta(-1)
{
	/// update static pointer to app object
	_pApp = this;
}

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

OsgAppProxy::~OsgAppProxy() {

}

void OsgAppProxy::setActiveApplication() {
	vrj::Kernel::instance()->setApplication(this);
}


void OsgAppProxy::setAppDelegate(luabind::object const & delegate) {
	/// @todo test here to see if the passed delegate is suitable
#ifdef VERBOSE
	VRJLUA_MSG_START(dbgVRJLUA_PROXY, MSG_STATUS)
		<< "Trying to set the app delegate"
		<< VRJLUA_MSG_END(dbgVRJLUA_PROXY, MSG_STATUS);
#endif
	if (luabind::type(delegate) == LUA_TTABLE) {
		_delegate = delegate;
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
	std::cout << "---------- OsgAppProxy::initScene() ---------------" << std::endl;

	// Create the top level node of the tree
	_rootNode = new osg::Group();

	_forwardCallToDelegate("initScene");
}

void OsgAppProxy::configSceneView(osgUtil::SceneView* newSceneViewer) {
	vrj::OsgApp::configSceneView(newSceneViewer);
/*
	newSceneViewer->getLight()->setAmbient(osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
	newSceneViewer->getLight()->setDiffuse(osg::Vec4(0.9f, 0.9f, 0.9f, 1.0f));
	newSceneViewer->getLight()->setSpecular(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	*/
	osg::ref_ptr<osg::Light> light0;
	osg::ref_ptr<osg::LightSource> lightSource0;
	light0 = new osg::Light();
	light0->setLightNum(0);
	light0->setAmbient(osg::Vec4f(0.36862f, 0.36842f, 0.36842f, 1.0f));
	light0->setDiffuse(osg::Vec4f(0.88627f, 0.88500f, 0.88500f, 1.0f));
	light0->setSpecular(osg::Vec4f(0.49019f, 0.48872f, 0.48872f, 1.0f));
	//light0->setPosition(osg::Vec4f(0.0f, 10000.0f, -20.0f, 0.0f));
	light0->setPosition(osg::Vec4f(10000.0f, 10000.0f, 10000.0f, 0.0f));
	light0->setDirection(osg::Vec3f(-1.0f, -1.0f, -1.0f));

	lightSource0 = new osg::LightSource();
	lightSource0->setLight(light0.get());
	lightSource0->setLocalStateSetModes(osg::StateAttribute::ON);

	// Now that we know we have a root node add the default light to the
	// scene.
	getScene()->addChild( lightSource0.get() );
}

void OsgAppProxy::preFrame() {
	vpr::Interval cur_time = vrj::Kernel::instance()->getUsers()[0]->getHeadUpdateTime();
	vpr::Interval diff_time(cur_time - _lastPreFrameTime);
	if (_lastPreFrameTime.getBaseVal() >= cur_time.getBaseVal()) {
		diff_time.secf(0.0f);
	}

	_timeDelta = diff_time.secf();

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

bool OsgAppProxy::_forwardCallToDelegate(const char * call) {
	if (_delegate && luabind::type(_delegate[call]) == LUA_TFUNCTION) {
		_delegate[call](_delegate);
		return true;
	}

#ifdef VERY_VERBOSE
	VRJLUA_MSG_START(dbgVRJLUA_PROXY, MSG_ERROR)
		<< "Delegate not valid or no '" << call << "' element defined: " << _delegate
		<< VRJLUA_MSG_END(dbgVRJLUA_PROXY, MSG_ERROR);
#endif
	return false;
}

} // end of vrjLua namespace
