#include <iostream>
#include <assert.h>
#include "ErrorMessageHandler.h"
#include "CompilerState.h"

namespace MFM {


  ErrorMessageHandler::ErrorMessageHandler(): m_state(NULL), m_debugMode(true), m_infoMode(true), m_noteMode(true), m_warnMode(true), m_waitMode(true), m_fOut(NULL), m_errorCount(0), m_warningCount(0)
  {
    //awaits init() call by owner
  }

  ErrorMessageHandler::~ErrorMessageHandler()
  {
    m_seenToUserMessages.clear();
  }


  void  ErrorMessageHandler::init(CompilerState * state, bool debugMode, bool infoMode, bool noteMode, bool warnMode, bool waitMode, File * fp)
  {
    assert(state);
    m_state = state;
    m_debugMode = debugMode;
    m_infoMode = infoMode;
    m_noteMode = noteMode;
    m_warnMode = warnMode;
    m_waitMode = waitMode;
    setFileOutput(fp);
    clearCounts();
  }

  void ErrorMessageHandler::changeWaitToErrMode()
  {
    m_waitMode = false;
  }

  void ErrorMessageHandler::revertToWaitMode()
  {
    m_waitMode = true;
  }

  bool ErrorMessageHandler::isWaitModeWaiting()
  {
    return m_waitMode;
  }

  void ErrorMessageHandler::buildMessage(const Token * atTok, const char * message, const char * file, const char * func, u32 atline, MSGTYPE mtype)
  {
    buildMessage(m_state->getTokenLocationAsString(atTok).c_str(), message, file, func, atline, mtype);
  }


  void ErrorMessageHandler::buildMessage(const char * loc, const char * message, const char * file, const char * func, u32 atline, MSGTYPE mtype)
  {
    std::ostringstream srcDebug;
    srcDebug << file << ":"  << func << ":" << atline;

    switch(mtype)
      {
      case MSG_ERR:
	outputMsg(loc,message,srcDebug.str().c_str(), "ERROR");
	incErrorCount();
	break;
      case MSG_WARN:
	if(m_warnMode)
	  outputMsg(loc,message,srcDebug.str().c_str(), "Warning");
	incWarningCount();
	break;
      case MSG_INFO:
	if(m_infoMode)
	  outputMsg(loc,message,srcDebug.str().c_str(), "Info");
	break;
      case MSG_NOTE:
	if(m_noteMode)
	  outputMsg(loc,message,srcDebug.str().c_str(), "NOTE");
	break;
      case MSG_DEBUG:
	if(m_debugMode)
	  outputMsg(loc,message,srcDebug.str().c_str(), "debug", false);
	break;
      case MSG_WAIT:
	if(m_waitMode)
	  outputMsg(loc,message,srcDebug.str().c_str(), "debug", false);
	else
	  {
	    if(m_state->turnWaitMessageIntoErrorMessage())
	      {
		outputMsg(loc,message,srcDebug.str().c_str(), "ERROR");
		incErrorCount();
	      }
	    else
	      outputMsg(loc,message,srcDebug.str().c_str(), "debug", false);
	  }
	break;
      default:
	break;
      };
  }


  void ErrorMessageHandler::setFileOutput(File * fp)
  {
    m_fOut = fp;  //owned by caller
  }


  void ErrorMessageHandler::incErrorCount()
  {
    m_errorCount++;
  }

  u32 ErrorMessageHandler::getErrorCount()
  {
    return m_errorCount;
  }

  void ErrorMessageHandler::incWarningCount()
  {
    m_warningCount++;
  }

  u32 ErrorMessageHandler::getWarningCount()
  {
    return m_warningCount;
  }

  void ErrorMessageHandler::clearCounts()
  {
    m_errorCount = 0;
    m_warningCount = 0;
  }


  void ErrorMessageHandler::outputMsg(const char * ulamloc, const char * message, const char * debugMethod, const char * label, bool toUser)
  {
    if(m_debugMode)
      {
	//okay to use cerr here!
	std::cerr <<ulamloc << " (" << debugMethod << ") " << label << ": " << message << "." << std::endl;
      }

    // debugMethod not given to user
    if(m_fOut && toUser)
      {
	std::ostringstream msg;
	msg << ulamloc << " " << label << ": " << message << ".\n";
	std::string msgstr(msg.str());
	std::pair<std::set<std::string>::iterator,bool> ret;
	ret = m_seenToUserMessages.insert(msgstr);
	if(ret.second) //false if already existed, i.e. not added
	  m_fOut->write(msgstr.c_str());
#if 0
	m_fOut->write(ulamloc);
	m_fOut->write(" ");
	m_fOut->write(label);
	m_fOut->write(": ");
	m_fOut->write(message);
	m_fOut->write(".\n");
#endif
      }
  }

}  // end MFM
