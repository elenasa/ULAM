/**                                        -*- mode:C++ -*-
 * SymbolTableOfFunctions.h - Handling of Table of Function Symbols for ULAM
 *
 * Copyright (C) 2014-2016 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2016 Ackleyshack LLC.
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
  \file SymbolTableOfFunctions.h - Handling of Table of Function Symbols for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2016 All rights reserved.
  \gpl
*/


#ifndef SYMBOLTABLEOFFUNCTIONS_H
#define SYMBOLTABLEOFFUNCTIONS_H

#include "SymbolTable.h"
#include "MapClassMemberDesc.h"

namespace MFM{

  class SymbolTableOfFunctions : public SymbolTable
  {
  public:

    SymbolTableOfFunctions(CompilerState& state);
    SymbolTableOfFunctions(const SymbolTableOfFunctions& ref);
    virtual ~SymbolTableOfFunctions();

    virtual u32 getTotalSymbolSize();

    //Table Of Functions:
    void addClassMemberFunctionDescriptionsToMap(UTI classType, ClassMemberMap& classmembers);

    void checkTableOfFunctions(std::map<std::string, UTI>& mangledFunctionMap, u32& probcount);
    void checkTableOfFunctionsInAncestor(std::map<std::string, UTI>& mangledFunctionMap, u32& probcount);

    void linkToParentNodesAcrossTableOfFunctions(NodeBlockClass * p);

    void updatePrevBlockPtrAcrossTableOfFunctions(NodeBlockClass * p);

    bool findNodeNoAcrossTableOfFunctions(NNO n, Node*& foundNode);

    void labelTableOfFunctions();

    void countNavNodesAcrossTableOfFunctions(u32& ncnt, u32& hcnt, u32& nocnt);

    bool checkCustomArrayTypeFuncs();

    UTI getCustomArrayReturnTypeGetFunction();

    u32 getCustomArrayIndexTypeGetFunction(Node * rnode, UTI& idxuti, bool& hasHazyArgs);

    u32 countNativeFuncDeclsForTableOfFunctions();

    void printUnresolvedLocalVariablesForTableOfFunctions();

    void calcMaxDepthForTableOfFunctions();

    void calcMaxIndexForVirtualTableOfFunctions(s32& maxidx);

    void checkAbstractInstanceErrorsAcrossTableOfFunctions();

    void genCodeForTableOfFunctions(File * fp, bool declOnly, ULAMCLASSTYPE classtype);

  protected:

  private:

  };

}

#endif //end SYMBOLTABLEOFFUNCTIONS_H
