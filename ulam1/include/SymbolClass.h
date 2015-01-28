/**                                        -*- mode:C++ -*-
 * SymbolClass.h -  Basic handling of Class Symbols for ULAM
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
  \file SymbolClass.h -  Basic handling of Class Symbols for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2015 All rights reserved.
  \gpl
*/

#ifndef SYMBOLCLASS_H
#define SYMBOLCLASS_H

#include "Symbol.h"
#include "NodeBlockClass.h"
#include "UlamTypeClass.h"

namespace MFM{


  class CompilerState;  //forward

  class SymbolClass : public Symbol
  {
  public:
    SymbolClass(u32 id, UTI utype, NodeBlockClass * classblock, CompilerState& state);
    ~SymbolClass();

    virtual bool isClass();

    virtual bool isClassTemplate();

    void setClassBlockNode(NodeBlockClass * node);

    NodeBlockClass * getClassBlockNode();

    virtual const std::string getMangledPrefix();

    ULAMCLASSTYPE getUlamClass();

    void setUlamClass(ULAMCLASSTYPE type);

    void setQuarkUnion();

    bool isQuarkUnion();

    virtual void generateCode(FileManager * fm);

  protected:

  private:
    NodeBlockClass * m_classBlock;
    bool m_quarkunion;

    void generateHeaderPreamble(File * fp);
    void genAllCapsIfndefForHeaderFile(File * fp);
    void genAllCapsEndifForHeaderFile(File * fp);
    void generateHeaderIncludes(File * fp);

    void genMangledTypesHeaderFile(FileManager * fm);  //obsolete
    void generateMain(FileManager * fm);

  };

}

#endif //end SYMBOLCLASS_H
