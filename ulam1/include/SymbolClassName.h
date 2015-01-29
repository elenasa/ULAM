/**                                        -*- mode:C++ -*-
 * SymbolClassName.h -  Class Symbol "Template" for ULAM
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
  \file SymbolClassName.h -  Class Symbol "Template" for ULAM
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
    SymbolClassName(const SymbolClassName& sref);
    virtual ~SymbolClassName();

    virtual Symbol * clone();

    void addParameterSymbol(SymbolConstantValue * argSym);
    u32 getNumberOfParameters();
    u32 getTotalSizeOfParameters();
    Symbol * getParameterSymbolPtr(u32 n);

    virtual bool isClassTemplate(UTI cuti);

    bool isClassInstance(UTI uti, SymbolClass * & symptrref);
    void addClassInstance(UTI uti, SymbolClass * symptr);

    /** replaces temporary class argument names, updates the ST, and the class type */
    void fixAnyClassInstances();

    std::string formatAnInstancesArgValuesAsAString(UTI instance);

    void checkAndLabelClassInstances(); // needed?

    bool setBitSizeOfClassInstances();
    void printBitSizeOfClassInstances();
    void packBitsForClassInstances();

    void generateCodeForClassInstances(FileManager * fm);

   protected:

  private:
    //ordered class parameters
    std::vector<SymbolConstantValue *> m_parameterSymbols;  // like named constants; symbols owned by m_ST.
    std::map<UTI, SymbolClass* > m_scalarClassInstanceIdxToSymbolPtr;

    bool trySetBitsizeWithUTIValues(UTI suti, s32& totalbits);
    bool takeAnInstancesArgValues(UTI instance);
    void resetParameterValuesUnknown();

    void printBitSizeOfClass(UTI suti);
  };

}

#endif //end SYMBOLCLASSNAME_H
