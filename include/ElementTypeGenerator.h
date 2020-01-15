/**                                        -*- mode:C++ -*-
 * ElementTypeGenerator.h Generate MFM Element Types for ULAM
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
  \file ElementTypeGenerator.h Generate MFM Element Types for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2018 All rights reserved.
  \gpl
*/

#ifndef ELEMENTTYPEGENERATOR_H
#define ELEMENTTYPEGENERATOR_H

#include <map>
#include <set>
#include "Constants.h"

namespace MFM {

  class ElementTypeGenerator
  {
  public:
    ElementTypeGenerator(); //incremental version
    ElementTypeGenerator(u32 n);
    ~ElementTypeGenerator();

    void generateTypes();
    void beginIteration();
    ELE_TYPE getNextType(); //distribution

    ELE_TYPE makeNextType(); //incremental-version

  private:
    u32 m_maxNumberOfTypes; //generation
    u32 m_next; //distribution
    u32 m_counter; //generation
    std::map<u32, ELE_TYPE> m_sequentialTypeMap;
    std::set<ELE_TYPE> m_setOfKnownTypes;

    ELE_TYPE makeAType();
    bool addTypeToSet(ELE_TYPE type);
  };

} //MFM

#endif //ELEMENTTYPEGENERATOR_H
