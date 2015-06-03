/**                                        -*- mode:C++ -*-
 * NodeBlockClass.h - Basic Node for handling Classes for ULAM
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
  \file NodeBlockClass.h - Basic Node for handling Classes for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2015 All rights reserved.
  \gpl
*/


#ifndef NODEBLOCKCLASS_H
#define NODEBLOCKCLASS_H

#include "NodeBlock.h"
#include "NodeBlockFunctionDefinition.h"
#include "NodeList.h"
#include "Symbol.h"

namespace MFM{

  class NodeBlockClass : public NodeBlock
  {
  public:

    NodeBlockClass(NodeBlock * prevBlockNode, CompilerState & state, NodeStatements * s = NULL);
    NodeBlockClass(const NodeBlockClass& ref);
    virtual ~NodeBlockClass();

    virtual Node * instantiate();

    bool isEmpty();

    void setEmpty();

    virtual void updateLineage(NNO pno);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual UTI checkAndLabelType();

    bool checkParameterNodeTypes();

    void addParameterNode(Node * nodeArg);

    virtual void countNavNodes(u32& cnt);

    void checkCustomArrayTypeFunctions();

    UTI getCustomArrayTypeFromGetFunction();

    void checkDuplicateFunctions();

    void calcMaxDepthOfFunctions();

    virtual EvalStatus eval();

    //checks both function and variable symbol names
    virtual bool isIdInScope(u32 id, Symbol * & symptrref);

    bool isFuncIdInScope(u32 id, Symbol * & symptrref);

    void addFuncIdToScope(u32 id, Symbol * symptr);

    u32 getNumberOfFuncSymbolsInTable();

    u32 getSizeOfFuncSymbolsInTable();

    void updatePrevBlockPtrOfFuncSymbolsInTable();

    void packBitsForVariableDataMembers();

    virtual u32 countNativeFuncDecls();

    void generateCodeForFunctions(File * fp, bool declOnly, ULAMCLASSTYPE classtype);

    virtual void genCode(File * fp, UlamValue& uvpass);

    void genCodeBody(File * fp, UlamValue& uvpass);  //specific for this class

    NodeBlockFunctionDefinition * findTestFunctionNode();

  protected:
    SymbolTable m_functionST;

  private:

    bool m_isEmpty; //replaces separate node
    UTI m_templateClassParentUTI;
    NodeList * m_nodeParameterList; //constants

    void genCodeHeaderQuark(File * fp);
    void genCodeHeaderElement(File * fp);

    void genImmediateMangledTypesForHeaderFile(File * fp);
    void genShortNameParameterTypesExtractedForHeaderFile(File * fp);

    virtual void generateCodeForBuiltInClassFunctions(File * fp, bool declOnly, ULAMCLASSTYPE classtype);
    void generateInternalIsMethodForElement(File * fp, bool declOnly);
    virtual void generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount);
    void generateUlamClassInfoCount(File * fp, bool declOnly, u32 dmcount);
    void generateUlamClassGetMangledName(File * fp, bool declOnly);

    std::string removePunct(std::string str);
  };

}

#endif //end NODEBLOCKCLASS_H
