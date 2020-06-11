/**                                        -*- mode:C++ -*-
 * ErrorMessageHandler.h - Basic Error Message handling for ULAM
 *
 * Copyright (C) 2014-2017 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2017 Ackleyshack LLC.
 *
 * This file is part of the ULAM programming language compilation system.
 *
 * The ULAM programming language compilation system is free software:
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * The ULAM programming language compilation system is distributed in
 * the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the ULAM programming language compilation system
 * software.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 */

/**
  \file ErrorMessageHandler.h - Basic Error Message handling for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2017 All rights reserved.
  \gpl
*/


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
#define MSG3(l,m,t) state.m_err.buildMessage(l,m, __FILE__ , __func__ , __LINE__ ,MSG_##t)

  class CompilerState; //forward

  enum MSGTYPE { MSG_ERR=0, MSG_WARN, MSG_INFO, MSG_NOTE, MSG_DEBUG, MSG_WAIT};


  class ErrorMessageHandler
  {
  public:

    ErrorMessageHandler();

    ~ErrorMessageHandler();

    void init(CompilerState * state, bool debugMode, bool infoMode, bool noteMode, bool warnMode, bool waitMode, File * fp);

    void changeWaitToErrMode();

    void revertToWaitMode();

    void setFileOutput(File * fp);

    u32 getErrorCount();

    u32 getWarningCount();

    //ability to reset counts between parsing, labeling, eval
    void clearCounts();

    void buildMessage(const Token * atTok, const char * message, const char * file, const char * func, u32 atline, MSGTYPE mtype);

    void buildMessage(const char *, const char * message, const char * file, const char * func, u32 atline, MSGTYPE mtype);


  private:
    CompilerState * m_state;
    bool m_debugMode;
    bool m_infoMode;
    bool m_noteMode;
    bool m_warnMode;
    bool m_waitMode;
    File * m_fOut;

    u32 m_errorCount;
    u32 m_warningCount;

    void incErrorCount();
    void incWarningCount();

    void outputMsg(const char * ulamloc, const char * message, const char * debugMethod, const char * label, bool toUser = true);
  };
}

#endif  /* ERRORMESSAGEHANDLER_H */
