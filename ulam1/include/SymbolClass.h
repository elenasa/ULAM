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
#include "NodeTypeBitsize.h"
#include "NodeSquareBracket.h"
#include "NodeConstantDef.h"
#include "TargetMap.h"

namespace MFM{


  class CompilerState;  //forward
  class SymbolClassNameTemplate;  //forward
  class Resolver; //forward

  class SymbolClass : public Symbol
  {
  public:
    SymbolClass(Token id, UTI utype, NodeBlockClass * classblock, SymbolClassNameTemplate * parent, CompilerState& state);
    SymbolClass(const SymbolClass& sref);
    virtual ~SymbolClass();

    virtual Symbol * clone();

    virtual bool isClass();

    virtual bool isClassTemplate(UTI cuti);

    void setClassBlockNode(NodeBlockClass * node);

    NodeBlockClass * getClassBlockNode();

    SymbolClassNameTemplate * getParentClassTemplate();

    virtual const std::string getMangledPrefix();

    ULAMCLASSTYPE getUlamClass();

    void setUlamClass(ULAMCLASSTYPE type);

    void setQuarkUnion();

    bool isQuarkUnion();

    bool isStub();

    void unsetStub();

    bool isCustomArray(); //by ulamtypeclass

    UTI getCustomArrayType(); //by function return type

    bool trySetBitsizeWithUTIValues(s32& totalbits);

    void printBitSizeOfClass();

    void testThisClass(File * fp); //eval-land

    bool statusNonreadyClassArguments();

    bool constantFoldNonreadyClassArguments();

    void linkConstantExpressionForPendingArg(NodeConstantDef * constNode);
    bool pendingClassArgumentsForClassInstance();
    void cloneResolverForStubClassInstance(const SymbolClass* csym, UTI context);
    void cloneResolverUTImap(SymbolClass * csym);

    UTI getContextForPendingArgs();

    bool mapUTItoUTI(UTI auti, UTI mappedUTI);
    bool hasMappedUTI(UTI auti, UTI& mappedUTI);
    bool findNodeNoInResolver(NNO n, Node *& foundNode);

    virtual void generateCode(FileManager * fm);

    void generateAsOtherInclude(File * fp);

    void generateAsOtherForwardDef(File * fp);

    void generateTestInstance(File * fp, bool runtest);

    void addTargetDescriptionMapEntry(TargetMap& classtargets);

  protected:
    Resolver * m_resolver;

    void setParentClassTemplate(SymbolClassNameTemplate *);

  private:
    NodeBlockClass * m_classBlock;
    SymbolClassNameTemplate * m_parentTemplate;
    bool m_quarkunion;
    bool m_stub;

    void generateHeaderPreamble(File * fp);
    void genAllCapsIfndefForHeaderFile(File * fp);
    void genAllCapsEndifForHeaderFile(File * fp);
    void generateHeaderIncludes(File * fp);

    void genMangledTypesHeaderFile(FileManager * fm);  //obsolete
    void generateMain(FileManager * fm);

    static std::string firstletterTolowercase(const std::string s);
  };

}

#endif //end SYMBOLCLASS_H
