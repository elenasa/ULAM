/**                                        -*- mode:C++ -*-
 * SymbolClassNameTemplate.h -  Class Symbol "Template" for ULAM
 *
 * Copyright (C) 2015 The Regents of the University of New Mexico.
 * Copyright (C) 2015 Ackleyshack LLC.
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
  \file SymbolClassNameTemplate.h -  Class Symbol "Template" for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2015 All rights reserved.
  \gpl
*/

#ifndef SYMBOLCLASSNAMETEMPLATE_H
#define SYMBOLCLASSNAMETEMPLATE_H

#include "SymbolClassName.h"

namespace MFM{

  class SymbolClassNameTemplate : public SymbolClassName
  {
  public:
    SymbolClassNameTemplate(Token id, UTI utype, NodeBlockClass * classblock, CompilerState& state);
    virtual ~SymbolClassNameTemplate();

    virtual void getTargetDescriptorsForClassInstances(TargetMap& classtargets);

    void addParameterSymbol(SymbolConstantValue * argSym);
    u32 getNumberOfParameters();
    u32 getTotalParameterSlots();
    Symbol * getParameterSymbolPtr(u32 n);

    virtual bool isClassTemplate();
    bool isClassTemplate(UTI cuti);
    bool findClassInstanceByUTI(UTI uti, SymbolClass * & symptrref);
    bool findClassInstanceByArgString(UTI cuti, SymbolClass *& csymptr);

    void addClassInstanceUTI(UTI uti, SymbolClass * symptr);
    void addClassInstanceByArgString(UTI uti, SymbolClass * symptr);

    bool pendingClassArgumentsForStubClassInstance(UTI instance);

    SymbolClass * makeAStubClassInstance(Token typeTok, UTI cuti); //to hold class args, and cUTI
    void copyAStubClassInstance(UTI instance, UTI newuti, UTI context);

    /** replaces temporary class argument names, updates the ST, and the class type */
    void fixAnyClassInstances();

    //    virtual bool statusUnknownConstantExpressionsInClassInstances();

    bool statusNonreadyClassArgumentsInStubClassInstances();
    bool constantFoldClassArgumentsInAStubClassInstance(UTI instance);

    virtual std::string formatAnInstancesArgValuesAsAString(UTI instance);
    std::string formatAnInstancesArgValuesAsCommaDelimitedString(UTI instance);

    //helpers while deep instantiation
    bool hasInstanceMappedUTI(UTI instance, UTI auti, UTI& mappedUTI);
    bool mapInstanceUTI(UTI instance, UTI auti, UTI mappeduti);
    bool fullyInstantiate();

    virtual Node * findNodeNoInAClassInstance(UTI instance, NNO n);

    void updateLineageOfClassInstanceUTI(UTI cuti); //during full instantiateion

    virtual void checkCustomArraysOfClassInstances();
    virtual void checkDuplicateFunctionsForClassInstances();
    virtual void calcMaxDepthOfFunctionsForClassInstances();
    virtual void checkAndLabelClassInstances();
    virtual u32 countNavNodesInClassInstances();
    virtual bool setBitSizeOfClassInstances();
    virtual void printBitSizeOfClassInstances();
    virtual void packBitsForClassInstances();

    virtual void testForClassInstances(File * fp);

    virtual void generateCodeForClassInstances(FileManager * fm);

    virtual void generateIncludesForClassInstances(File * fp);

    virtual void generateForwardDefsForClassInstances(File * fp);

    virtual void generateTestInstanceForClassInstances(File * fp, bool runtest);

   protected:

  private:
    //ordered class parameters
    std::vector<SymbolConstantValue *> m_parameterSymbols;  // like named constants; symbols owned by m_ST.
    std::map<UTI, SymbolClass* > m_scalarClassInstanceIdxToSymbolPtr;
    std::map<UTI, SymbolClass* > m_scalarClassInstanceIdxToSymbolPtrTEMP; //iteration in progress
    std::map<std::string, SymbolClass* > m_scalarClassArgStringsToSymbolPtr; //merged set
    std::map<UTI, std::map<UTI,UTI> > m_mapOfTemplateUTIToInstanceUTIPerClassInstance;

    void mergeClassInstancesFromTEMP();
    bool takeAnInstancesArgValues(SymbolClass * fm, SymbolClass * to);
    bool copyAnInstancesArgValues(SymbolClass * fm, SymbolClass * to);
    void cloneAnInstancesUTImap(SymbolClass * fm, SymbolClass * to);
  };

}

#endif //end SYMBOLCLASSNAMETEMPLATE_H
