/**                                        -*- mode:C++ -*-
 * NodeFuncDecl.h - Node handling the Function Declaration Order for ULAM
 *
 * Copyright (C) 2020 The Regents of the University of New Mexico.
 * Copyright (C) 2020 Ackleyshack LLC.
 * Copyright (C) 2020 The Living Computation Foundation.
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
  \file NodeFuncDecl.h - Node handling the Function Declaration Order for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2020 All rights reserved.
  \gpl
*/


#ifndef NODEFUNCDECL_H
#define NODEFUNCDECL_H

#include "Node.h"

namespace MFM{

  class SymbolFunction; //forward

  class NodeFuncDecl : public Node
  {
  public:

    NodeFuncDecl(SymbolFunction * symfunc, CompilerState & state);

    NodeFuncDecl(const NodeFuncDecl& ref);

    virtual ~NodeFuncDecl();

    virtual Node * instantiate();

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual void noteTypeAndName(UTI cuti, s32 totalsize, u32& accumsize);

    virtual void genTypeAndNameEntryAsComment(File * fp, s32 totalsize, u32& accumsize);

    virtual UTI checkAndLabelType();

    virtual EvalStatus eval();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual TBOOL packBitsInOrderOfDeclaration(u32& offset);

    virtual void calcMaxIndexOfVirtualFunctionInOrderOfDeclaration(SymbolClass* csym, s32& maxidx); //!!!

    virtual bool buildDefaultValue(u32 wlen, BV8K& dvref);

    virtual void genCodeConstantArrayInitialization(File * fp);

    virtual void generateBuiltinConstantClassOrArrayInitializationFunction(File * fp, bool declOnly);

    virtual void generateTestInstance(File * fp, bool runtest);

    virtual void generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount);

    virtual void addMemberDescriptionToInfoMap(UTI classType, ClassMemberMap& classmembers);

    virtual void genCode(File * fp, UVPass& uvpass);

    virtual void generateFunctionInDeclarationOrder(File * fp, bool declOnly, ULAMCLASSTYPE classtype);

  protected:

  private:
    SymbolFunction * m_funcSymbolPtr; //not owner
    const u32 m_fid; //to instantiate, fm symbol
    const u32 m_ordernum;  //to instantiate, must be unique among ea fid, fm symbol

    u32 getfunctionid() const; //helper to set constant dm
    u32 getfunctionordernum() const; //helper to set contant dm

  };

}

#endif //end NODEFUNCDECL_H
