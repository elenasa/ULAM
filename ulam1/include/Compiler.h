/**                                        -*- mode:C++ -*-
 * Compiler.h - Basic top-level Compiler for ULAM
 *
 * Copyright (C) 2014-2015 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2015 Ackleyshack LLC.
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
  \date (C) 2014-2015 All rights reserved.
  \gpl
*/


#ifndef COMPILER_H
#define COMPILER_H

#include <string>
#include <vector>
#include <map>
#include "CompilerState.h"
#include "FileManager.h"
#include "File.h"
#include "Node.h"
#include "ParameterMap.h"
#include "Parser.h"
#include "SourceStream.h"
#include "TargetMap.h"

namespace MFM{

  class Compiler
  {

  public:
    Compiler();
    ~Compiler();

    u32 compileProgram(FileManager * infm, std::string startstr, FileManager * outfm, File * errput);

    u32 compileFiles(FileManager * infm, std::vector<std::string> filesToCompile, FileManager * outfm, File * errput);

    u32 parseProgram(FileManager * fm, std::string startstr, File * output); //for tests

    u32 checkAndTypeLabelProgram(File * output);

    bool resolvingLoop();

    bool hasTheTestMethod();

    bool targetIsAQuark();

    u32 testProgram(File * output);

    void printPostFix(File * output);

    void printProgramForDebug(File * output);

    void generateCodedProgram(File * output);

    std::string getMangledTarget();

    TargetMap getMangledTargetsMap();

    ParameterMap getMangledParametersMap();

    const std::string getFullPathLocationAsString(const Locator& loc);

  private:
    CompilerState m_state;  //owner

    u32 compileFile(std::string startstr, File * errput, SourceStream& ssref, Parser * p);
  };

} //MFM namespace

#endif //end COMPILER_H
