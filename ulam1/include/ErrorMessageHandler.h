/* -*- c++ -*- */

#ifndef ERRORMESSAGEHANDLER_H
#define ERRORMESSAGEHANDLER_H

#include <stdio.h>
#include <string>
#include <string.h>
#include "itype.h"
#include "Token.h"

namespace MFM
{

#define MSG(l,m,t) m_state.m_err.buildMessage(l,m, __FILE__ , __func__ , __LINE__ ,MSG_##t)
#define MSG2(l,m,t) m_err.buildMessage(l,m, __FILE__ , __func__ , __LINE__ ,MSG_##t)

  class CompilerState; //forward

  enum MSGTYPE { MSG_ERR=0, MSG_WARN, MSG_INFO, MSG_DEBUG};


  class ErrorMessageHandler
  {
  public:

    ErrorMessageHandler();

    ~ErrorMessageHandler();

    void init(CompilerState * state, bool debugMode, File * fp);

    void setFileOutput(File * fp);

    u32 getErrorCount();
    u32 getWarningCount();

    //ability to reset counts between parsing, labeling, eval
    void clearCounts();

    void buildMessage(Token * atTok, const char * message, const char * file, const char * func, u32 atline, MSGTYPE mtype);
    void buildMessage(const char *, const char * message, const char * file, const char * func, u32 atline, MSGTYPE mtype);


  private: 
    CompilerState * m_state;
    bool m_debugMode;
    File * m_fOut;

    u32 m_errorCount;
    u32 m_warningCount;

    void incErrorCount();
    void incWarningCount();

    void outputMsg(const char * ulamloc, const char * message, const char * debugMethod, const char * label, bool toUser = true);
  };
}

#endif  /* ERRORMESSAGEHANDLER_H */
