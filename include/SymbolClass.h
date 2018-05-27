/**                                        -*- mode:C++ -*-
 * SymbolClass.h -  Basic handling of Class Symbols for ULAM
 *
 * Copyright (C) 2014-2018 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2018 Ackleyshack LLC.
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
  \date (C) 2014-2018 All rights reserved.
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
#include "StringPoolUser.h"
#include "TargetMap.h"
#include "MapClassMemberDesc.h"
#include "VirtualTable.h"
#include "BitVector.h"

namespace MFM{

  class CompilerState;  //forward
  class SymbolClassNameTemplate;  //forward
  class Resolver; //forward

  class SymbolClass : public Symbol
  {
  public:
    SymbolClass(const Token& id, UTI utype, NodeBlockClass * classblock, SymbolClassNameTemplate * parent, CompilerState& state);
    SymbolClass(const SymbolClass& sref);
    virtual ~SymbolClass();

    virtual Symbol * clone();

    virtual bool isClass();

    virtual bool isClassTemplate(UTI cuti);

    void setSuperClass(UTI superclass);
    UTI getSuperClass();

    void setClassBlockNode(NodeBlockClass * node);

    NodeBlockClass * getClassBlockNode();

    SymbolClassNameTemplate * getParentClassTemplate();

    virtual const std::string getMangledPrefix();

    ULAMCLASSTYPE getUlamClass(); //helper

    void setQuarkUnion();

    bool isQuarkUnion();

    bool isStub();

    void unsetStub();

    bool isCustomArray(); //by ulamtypeclass

    UTI getCustomArrayType(); //by function return type

    u32 getCustomArrayIndexTypeFor(Node * rnode, UTI& idxuti, bool& hasHazyArgs);

    bool hasCustomArrayLengthof();

    bool trySetBitsizeWithUTIValues(s32& totalbits);

    void printBitSizeOfClass();

    bool getDefaultQuark(u32& dqref);
    bool getPackedDefaultValue(u64& dpkref);
    bool getDefaultValue(BV8K& dvref); //return true if ready

    bool buildClassConstantDefaultValues();

    TBOOL packBitsForClassVariableDataMembers();

    void testThisClass(File * fp); //eval-land

    void addUnknownTypeTokenToClass(const Token& tok, UTI huti);
    Token removeKnownTypeTokenFromClass(UTI huti);
    bool hasUnknownTypeInClass(UTI huti);
    bool getUnknownTypeTokenInClass(UTI huti, Token& tok);
    bool statusUnknownTypeInClass(UTI huti);
    bool statusUnknownTypeNamesInClass();
    u32 reportUnknownTypeNamesInClass();
    bool reportLongClassName();

    bool statusNonreadyClassArguments();

    u32 countNonreadyClassArguments();

    void linkConstantExpressionForPendingArg(NodeConstantDef * constNode);
    bool pendingClassArgumentsForClassInstance();
    void cloneArgumentNodesForClassInstance(SymbolClass * fmcsym, UTI argvaluecontext, UTI argtypecontext, bool toStub);
    void cloneResolverUTImap(SymbolClass * csym);
    void cloneUnknownTypesMapInClass(SymbolClass * to);

    void setContextForPendingArgValues(UTI context);
    UTI getContextForPendingArgValues();
    void setContextForPendingArgTypes(UTI context);
    UTI getContextForPendingArgTypes();

    bool mapUTItoUTI(UTI auti, UTI mappedUTI);
    bool hasMappedUTI(UTI auti, UTI& mappedUTI);

    virtual void generateCode(FileManager * fm);

    void generateAsOtherInclude(File * fp);
    void generateAllIncludesForTestMain(File * fp);

    void generateAsOtherForwardDef(File * fp);

    void generateTestInstance(File * fp, bool runtest);

    void addTargetDescriptionMapEntry(TargetMap& classtargets, u32 scid);

    void addClassMemberDescriptionsMapEntry(ClassMemberMap& classmembers);

    void initVTable(s32 initialmax);
    void updateVTable(u32 idx, SymbolFunction * fsym, UTI kinuti, bool isPure);
    VT& getVTableRef();
    bool isPureVTableEntry(u32 idx);
    UTI getClassForVTableEntry(u32 idx);
    void notePureFunctionSignatures();
    std::string getMangledFunctionNameForVTableEntry(u32 idx);
    std::string getMangledFunctionNameWithTypesForVTableEntry(u32 idx);
    struct VTEntry getVTableEntry(u32 idx);

    bool isAbstract();

    StringPoolUser& getUserStringPoolRef();
    void setUserStringPoolRef(const StringPoolUser& spref);

  protected:
    Resolver * m_resolver;

    void setParentClassTemplate(SymbolClassNameTemplate *);

  private:
    NodeBlockClass * m_classBlock;
    SymbolClassNameTemplate * m_parentTemplate;
    bool m_quarkunion;
    bool m_stub;
    BV8K m_defaultValue; //BitVector
    bool m_isreadyDefaultValue;
    UTI m_superClass; //single inheritance
    bool m_bitsPacked;

    void assignClassArgValuesInStubCopy();

    void generateHeaderPreamble(File * fp);
    void genIfndefForHeaderFile(File * fp);
    void genEndifForHeaderFile(File * fp);
    void generateHeaderIncludes(File * fp);

    void genMangledTypesHeaderFile(FileManager * fm);  //obsolete

    void generateMain(FileManager * fm);

    static std::string firstletterTolowercase(const std::string s);

    VT m_vtable;

  };

}

#endif //end SYMBOLCLASS_H
