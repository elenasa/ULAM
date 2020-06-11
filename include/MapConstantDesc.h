/**                                        -*- mode:C++ -*-
 * MapConstantDesc.h - Map of Named Constants for ULAM
 *
 * Copyright (C) 2015-2017 The Regents of the University of New Mexico.
 * Copyright (C) 2015-2017 Ackleyshack LLC.
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
  \file MapConstantDesc.h -  Map of Named Constants for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2015-2017 All rights reserved.
  \gpl
*/

#ifndef MAPCONSTANTDESC_H
#define MAPCONSTANTDESC_H

#include "MapClassMemberDesc.h"
#include "SymbolConstantValue.h"

namespace MFM
{
  struct ConstantDesc : public ClassMemberDesc
  {

    ConstantDesc(SymbolConstantValue * csym, UTI classtype, CompilerState & state);
    ConstantDesc(const ConstantDesc& cref);

    ~ConstantDesc();

    virtual ClassMemberDesc * clone() const;

    virtual std::string getMemberKind() const;
    //virtual bool getValue(u64& vref) const;
    virtual bool hasValue() const;
    virtual std::string getValueAsString() const;

  private:
    //u64 m_val; //as Bits
    u32 m_len;
    BV8K m_val; //as Bits
  };

} //MFM

#endif  /* MAPCONSTANTDESC_H */
