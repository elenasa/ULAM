/**                                        -*- mode:C++ -*-
 * NodeInitDM.h - Node handling Class Data Member Init for ULAM
 *
 * Copyright (C) 2018 The Regents of the University of New Mexico.
 * Copyright (C) 2018 Ackleyshack LLC.
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
  \file NodeInitDM.h - Node handling Class Data Member Init for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2018 All rights reserved.
  \gpl
*/


#ifndef NODEINITDM_H
#define NODEINITDM_H

#include "NodeConstantDef.h"

namespace MFM{

  class NodeInitDM : public NodeConstantDef
  {
  public:

    NodeInitDM(u32 dmid, Node * assignNode, UTI ofclass, CompilerState & state);
    NodeInitDM(const NodeInitDM& ref);

    virtual ~NodeInitDM();

    virtual Node * instantiate();

    virtual void printPostfix(File * f);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual void setSymbolPtr(SymbolWithValue * cvsymptr);

    void resetOfClassType(UTI cuti);

    virtual UTI checkAndLabelType();

    virtual bool buildDefaultValue(u32 wlen, BV8K& dvref);

    virtual void genCodeDefaultValueStringRegistrationNumber(File * fp, u32 startpos);

    virtual void genCodeElementTypeIntoDataMemberDefaultValue(File * fp, u32 startpos);

    virtual void fixPendingArgumentNode();

    virtual bool assignClassArgValueInStubCopy();

    virtual void packBitsInOrderOfDeclaration(u32& offset);

    virtual void printUnresolvedVariableDataMembers();

    virtual void printUnresolvedLocalVariables(u32 fid);

    virtual bool isAConstant();

    virtual void genCode(File * fp, UVPass& uvpass);

    virtual void genCodeConstantArrayInitialization(File * fp);

    virtual void generateBuiltinConstantArrayInitializationFunction(File * fp, bool declOnly);

    virtual void cloneAndAppendNode(std::vector<Node *> & cloneVec);

  protected:

    virtual void checkForSymbol();

  private:
    UTI m_ofClassUTI;

  };

} //MFM

#endif //NODEINITDM_H
