/**                                        -*- mode:C++ -*-
 * SymbolClassName.h -  Basic Class Symbol Name for ULAM
 *
 * Copyright (C) 2015-2020 The Regents of the University of New Mexico.
 * Copyright (C) 2015-2020 Ackleyshack LLC.
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
  \file SymbolClassName.h -  Basic Class Symbol Name for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2015-2020 All rights reserved.
  \gpl
*/

#ifndef SYMBOLCLASSNAME_H
#define SYMBOLCLASSNAME_H

#include "SymbolClass.h"
#include "SymbolConstantValue.h"
#include "ElementTypeGenerator.h"

namespace MFM{

  class SymbolClassName : public SymbolClass
  {
  public:
    SymbolClassName(const Token& id, UTI utype, NodeBlockClass * classblock, CompilerState& state);

    virtual ~SymbolClassName();

    void resetUnseenClassLocation(const Token& identTok);

    virtual void setStructuredComment();

    virtual void getTargetDescriptorsForClassInstances(TargetMap& classtargets);

    virtual void getClassMemberDescriptionsForClassInstances(ClassMemberMap& classmembers);

    virtual bool isClassTemplate();

    virtual void setSuperClassForClassInstance(UTI superclass, UTI instance);
    virtual UTI getSuperClassForClassInstance(UTI instance);

    virtual void appendBaseClassForClassInstance(UTI baseclass, UTI instance, bool sharedbase);
    virtual u32 getBaseClassCountForClassInstance(UTI instance);
    virtual UTI getBaseClassForClassInstance(UTI instance, u32 item);
    virtual bool updateBaseClassforClassInstance(UTI instance, UTI oldbase, UTI newbaseuti);

    virtual Node * findNodeNoInAClassInstance(UTI instance, NNO n);

    virtual std::string formatAnInstancesArgValuesAsAString(UTI instance, bool dereftypes = false);
    virtual void generatePrettyNameAndSignatureOfClassInstancesAsUserStrings();
    virtual std::string generatePrettyNameOrSignature(UTI instance, bool signa, bool argvals);

    virtual bool hasInstanceMappedUTI(UTI instance, UTI auti, UTI& mappedUTI);
    virtual bool mapInstanceUTI(UTI instance, UTI auti, UTI mappeduti);

    virtual void updateLineageOfClass();

    virtual void checkCustomArraysOfClassInstances();
    virtual void checkDuplicateFunctionsForClassInstances();
    virtual void calcMaxDepthOfFunctionsForClassInstances();
    virtual bool calcMaxIndexOfVirtualFunctionsForClassInstances();
    virtual void checkAbstractInstanceErrorsForClassInstances();

    virtual void checkAndLabelClassInstances();
    void checkAndLabelClassFirst();
    virtual void countNavNodesInClassInstances(u32& ncnt, u32& hcnt, u32& nocnt);

    virtual bool statusUnknownTypeInClassInstances(UTI huti);
    virtual bool statusUnknownTypeNamesInClassInstances();
    virtual u32 reportUnknownTypeNamesInClassInstances();
    virtual u32 reportClassInstanceNamesThatAreTooLong();

    virtual bool setBitSizeOfClassInstances();
    virtual void printBitSizeOfClassInstances();
    virtual void packBitsForClassInstances();
    virtual void printUnresolvedVariablesForClassInstances();

    virtual void buildDefaultValueForClassInstances();

    virtual void testForClassInstances(File * fp);

    virtual void assignRegistrationNumberForClassInstances(); //ulam-4,ulam-5

    virtual void generateCodeForClassInstances(FileManager * fm);

    virtual void generateIncludesForClassInstances(File * fp);
    virtual void generateAllIncludesForTestMainForClassInstances(File * fp);

    virtual void generateForwardDefsForClassInstances(File * fp);

    virtual void generateTestInstanceForClassInstances(File * fp, bool runtest);

   protected:

  private:

  };

}

#endif //end SYMBOLCLASSNAME_H
