/**                                        -*- mode:C++ -*-
 * SymbolClass.h -  Basic handling of Class Symbols for ULAM
 *
 * Copyright (C) 2014-2020 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2020 Ackleyshack LLC.
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
  \file SymbolClass.h -  Basic handling of Class Symbols for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2020 All rights reserved.
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
#include "VirtualTable.h" /* VT */
#include "BaseClassTable.h"
#include "BitVector.h"
#include <vector>

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

    u32 getBaseClassCount();
    UTI getBaseClass(u32 item);
    s32 isABaseClassItem(UTI puti);

    bool isDirectSharedBase(u32 item) const;
    u32 getNumberSharingBase(u32 item) const;
    u32 countDirectSharedBases() const;
    u32 findDirectSharedBases(std::map<UTI, u32>& svbmapref);

    void appendBaseClass(UTI baseclass, bool sharedbase);
    void updateBaseClass(UTI oldclasstype, u32 item, UTI newbaseclass);
    void setBaseClass(UTI baseclass, u32 item, bool sharedbase = true);
    s32 getBaseClassRelativePosition(u32 item) const;
    void setBaseClassRelativePosition(u32 item, u32 pos);

    UTI getSharedBaseClass(u32 item);
    s32 isASharedBaseClassItem(UTI buti);
    s32 isASharedBaseClassItemSearch(UTI buti);
    u32 getSharedBaseClassCount() const;

    s32 getSharedBaseClassRelativePosition(u32 item) const;
    void setSharedBaseClassRelativePosition(u32 item, u32 pos);

    void setClassBlockNode(NodeBlockClass * node);

    NodeBlockClass * getClassBlockNode();

    SymbolClassNameTemplate * getParentClassTemplate();

    virtual const std::string getMangledPrefix();

    ULAMCLASSTYPE getUlamClass(); //helper

    void setQuarkUnion();

    bool isQuarkUnion();

    bool isStub();

    void unsetStub();

    UTI getCustomArrayType(); //by function return type

    u32 getCustomArrayIndexTypeFor(Node * rnode, UTI& idxuti, bool& hasHazyArgs);

    bool hasCustomArrayLengthof();

    bool trySetBitsizeWithUTIValues(s32& totalbits);
    bool determineSharedBasesAndTotalBitsize(s32& sharedbitssaved, s32& sharedbitsize);

    void printBitSizeOfClass();

    bool getDefaultQuark(u32& dqref);
    bool getPackedDefaultValue(u64& dpkref);
    bool getDefaultValue(BV8K& dvref); //return true if ready

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

    bool assignRegistryNumber(u32 n); //ulam-4
    u32 getRegistryNumber(); //ulam-4, assign when UTI is ok ulam-5
    bool assignElementType(ELE_TYPE n); //ulam-4
    bool assignEmptyElementType(); //ulam-4
    ELE_TYPE getElementType(); //ulam-4

    virtual void generateCode(FileManager * fm);

    void generateAsOtherInclude(File * fp);
    void generateAllIncludesForTestMain(File * fp);

    void generateAsOtherForwardDef(File * fp);

    void generateTestInstance(File * fp, bool runtest);

    void addTargetDescriptionMapEntry(TargetMap& classtargets, u32 scid);

    void addClassMemberDescriptionsMapEntry(ClassMemberMap& classmembers);

    void initVTable(s32 initialmax);
    void updateVTable(u32 idx, SymbolFunction * fsym, UTI kinuti, UTI origuti, bool isPure);
    s32 getVTableSize();
    s32 getOrigVTableSize();
    VT& getVTableRef();
    u32 convertVTstartoffsetmap(std::map<u32, u32> & mapbyrnref); //returns count
    u32 getVTstartoffsetOfRelatedOriginatingClass(UTI origuti);
    u32 getVTableIndexForOriginatingClass(u32 idx);
    bool isPureVTableEntry(u32 idx);
    UTI getClassForVTableEntry(u32 idx);
    UTI getOriginatingClassForVTableEntry(u32 idx);
    void notePureFunctionSignatures();
    std::string getMangledFunctionNameForVTableEntry(u32 idx);
    std::string getMangledFunctionNameWithTypesForVTableEntry(u32 idx);
    u32 getVFuncIndexForVTableEntry(u32 idx);
    u32 getVFuncNameSignatureIdForVTableEntry(u32 idx);
    struct VTEntry getVTableEntry(u32 idx);
    struct VTEntry getOrigVTableEntry(u32 idx);

    bool isAbstract();
    bool checkAbstractClassError();

    void buildIsBitVectorByRegNum(BV8K& bitvecref);

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
    bool m_bitsPacked;
    u32 m_registryNumber; //ulam-4

    ELE_TYPE m_elementType; //ulam-4

    BasesTable m_basestable;
    BasesTable m_sharedbasestable; //ulam-5

    void clearBaseAsShared(u32 item);
    void setNumberSharingBase(u32 item, u32 numshared);
    s32 isABaseClassItemSearch(UTI buti);
    u32 getNumberSharingSharedBase(u32 item) const;
    void appendSharedBaseClass(UTI baseclass, u32 numshared);
    void updateSharedBaseClass(UTI oldclasstype, u32 item, UTI newbaseclass);

    void assignClassArgValuesInStubCopy();

    bool resolveHasMappedUTI(UTI auti, UTI& mappedUTI);

    void generateHeaderPreamble(File * fp);
    void genIfndefForHeaderFile(File * fp);
    void genEndifForHeaderFile(File * fp);
    void generateHeaderIncludes(File * fp);

    void genMangledTypesHeaderFile(FileManager * fm);  //obsolete

    void generateMain(FileManager * fm);

    static std::string firstletterTolowercase(const std::string s);

    BasesTableTypeMap m_basesVTstart; //includes entire hierarchy and self
    VT m_vtable;
    VT m_vownedVT;
    bool m_vtableinitialized;

    bool setVTstartoffsetOfRelatedOriginatingClass(UTI origuti, u32 startoffset);
  };

}

#endif //end SYMBOLCLASS_H
