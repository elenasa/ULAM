/**                                        -*- mode:C++ -*-
 * SymbolClassName.h -  Basic Class Symbol Name for ULAM
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
  \file SymbolClassName.h -  Basic Class Symbol Name for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2015 All rights reserved.
  \gpl
*/

#ifndef SYMBOLCLASSNAME_H
#define SYMBOLCLASSNAME_H

#include "SymbolClass.h"
#include "SymbolConstantValue.h"

namespace MFM{

  class SymbolClassName : public SymbolClass
  {
  public:
    SymbolClassName(u32 id, UTI utype, NodeBlockClass * classblock, CompilerState& state);
    virtual ~SymbolClassName();

    virtual bool isClassTemplate();

    virtual void getTargetDescriptorsForClassInstances(TargetMap& classtargets);

    virtual bool statusUnknownConstantExpressionsInClassInstances();

    virtual Node * findNodeNoInAClassInstance(UTI instance, NNO n);
    //virtual void constantFoldIncompleteUTIOfClassInstance(UTI instance, UTI auti);

    virtual std::string formatAnInstancesArgValuesAsAString(UTI instance);

    virtual void updateLineageOfClass();

    virtual void checkCustomArraysOfClassInstances();
    virtual void checkDuplicateFunctionsForClassInstances();
    virtual void calcMaxDepthOfFunctionsForClassInstances();

    virtual void checkAndLabelClassInstances();
    void checkAndLabelClassFirst();
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
  };

}

#endif //end SYMBOLCLASSNAME_H
