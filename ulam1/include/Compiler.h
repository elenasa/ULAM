/**                                        -*- mode:C++ -*-
 * Compiler.h - Basic top-level Compiler for ULAM
 *
 * Copyright (C) 2014 The Regents of the University of New Mexico.
 * Copyright (C) 2014 Ackleyshack LLC.
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
  \file Compiler.h - Basic top-level Compiler for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \gpl
*/


#ifndef COMPILER_H
#define COMPILER_H

#include <string>
#include "CompilerState.h"
#include "Node.h"
#include "FileManager.h"
#include "File.h"

namespace MFM{

  class Compiler
  {

  public:
    Compiler();
    ~Compiler();

    u32 compileProgram(FileManager * infm, std::string startstr, FileManager * outfm, File * errput);
    u32 parseProgram(FileManager * fm, std::string startstr, File * output, Node *& rtnNode);
    u32 checkAndTypeLabelProgram(Node * root, File * output);
    bool hasTheTestMethod();
    u32 testProgram(Node * root, File * output, s32& rtnValue);
    void printPostFix(Node * root, File * output);
    void printProgramForDebug(Node * root, File * output);
    void generateCodedProgram(Node * root, File * output);
    std::string getMangledTarget();

  private:

    CompilerState m_state;  //owner

  };

} //MFM namespace

#endif //end COMPILER_H
