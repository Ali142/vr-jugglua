/**	@file	FLTKConsole.cpp
	@brief	implementation

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
#include "FLTKConsole.h"

// Library/third-party includes
#include <boost/scoped_ptr.hpp>
#include <boost/bind.hpp>


// Standard includes
#include <iostream>
#include <stdexcept>


#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <vrjlua-fltk-console.h>

namespace vrjLua {



class FLTKConsoleView : public FLTKConsoleUI {
	public:
		FLTKConsoleView(FLTKConsole* console) :
				_console(console),
				FLTKConsoleUI(700,560),
				_inputBuf(new Fl_Text_Buffer()),
				_codeBuf(new Fl_Text_Buffer()) {
			_input->buffer(_inputBuf.get());
			_existingCode->buffer(_codeBuf.get());
			show();
		}

		virtual void runInput() {
			std::string input;
			{
				char * input_chars = _inputBuf->text();
				input = std::string(input_chars);
				free(input_chars);
			}
			bool ret = _console->runString(input);
			if (ret) {
				// Successful - append to the text display
				_codeBuf->append("\n");
				_codeBuf->append(input.c_str());
				_inputBuf->text("");
			} else {
				_codeBuf->append("\nExecution of input failed - check for errors and try again");
			}

			// Scroll to bottom
			_existingCode->scroll(_existingCode->count_lines(0, _codeBuf->length(), 1), 0);
		}


		virtual void chooseAndRunFile() {
			boost::shared_ptr<Fl_Native_File_Chooser> fc(new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_FILE));
			fc->title("Open VRJLua File...");
			fc->filter("VRJLua files\t*.{vrjlua,lua}");

			int ret = fc->show();
			if (ret != 0) {
				// No file picked, for one reason or another.
				return;
			}

			bool bret = _console->runFile(fc->filename());
			if (bret) {
				// Successful - append to the text display
				std::string code("require('");
				code += fc->filename();
				code += "')";
				_codeBuf->append("\n-- Run file chosen in GUI\n");
				_codeBuf->append(code.c_str());
			} else {
				_codeBuf->append("\n-- Execution of chosen file failed - check for errors and try again");
			}
		}

		virtual void saveFile() {
			Fl_Native_File_Chooser fc(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
			fc.title("Save VRJLua File...");
			fc.filter("VRJLua files\t*.{vrjlua,lua}");

			int ret = fc.show();
			if (ret != 0) {
				// No file picked, for one reason or another.
				return;
			}

			int saveret = _codeBuf->savefile(fc.filename());
			if (saveret == 0) {
				// Successful - append to the text display
				std::string code("-- code up to here saved to file '");
				code += fc.filename();
				code += "'\n";
				_codeBuf->append(code.c_str());
			} else {
				_codeBuf->append("\n-- Saving to chosen file failed");
			}
		}

		virtual void appendToDisplay(std::string const& message) {
			_codeBuf->append(message.c_str());
			_codeBuf->append("\n");
		}

	protected:
		FLTKConsole * _console;
		boost::scoped_ptr<Fl_Text_Buffer> _inputBuf;
		boost::scoped_ptr<Fl_Text_Buffer> _codeBuf;
};

FLTKConsole::FLTKConsole() :
		LuaConsole(),
		_running(false),
		_view(new FLTKConsoleView(this)) {
	// No constructor body
}

FLTKConsole::FLTKConsole(LuaScript const& script) :
		LuaConsole(script),
		_running(false),
		_view(new FLTKConsoleView(this))  {
	// No constructor body
}

FLTKConsole::~FLTKConsole() {
}

bool FLTKConsole::startThread() {
	if (_running) {
		/// @todo notify that the thread is already running?
		return true;
	}
	_running = true;
	_thread.setFunctor(boost::bind(&FLTKConsole::_threadLoop, this));
	_thread.start();
}

void FLTKConsole::stopThread() {
	_running = false;
}

void FLTKConsole::waitForThreadStop() {
	_thread.join();
}

void FLTKConsole::appendToDisplay(std::string const& message) {
	_view->appendToDisplay(message);
}

void FLTKConsole::_threadLoop() {
	bool ret = true;
	while (_running) {
		// Do the FLTK loop
		ret = _doThreadWork();
		if (!ret) {
			// Exit originating from FLTK
			_signalThreadExit();
			_running = false;
			break;
		}
	}
}

bool FLTKConsole::_doThreadWork() {
	return ((Fl::wait(0) >= 0) && (Fl::first_window() != NULL));
}

} // end of vrjLua namespace
