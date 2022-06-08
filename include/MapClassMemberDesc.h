/**                                        -*- mode:C++ -*-
 * MapClassMemberDesc.h - Map of Class Members for ULAM
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
  \file MapClassMemberDesc.h -  Map of Class Members for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2015-2017 All rights reserved.
  \gpl
*/

#ifndef MAPCLASSMEMBERDESC_H
#define MAPCLASSMEMBERDESC_H

#include <map>
#include <string>
#include "itype.h"
#include "Locator.h"
#include "Symbol.h"

namespace MFM
{
  struct CompilerState; //forward

  struct ClassMemberDesc
  {
    ClassMemberDesc(Symbol * sym, UTI classtype, CompilerState & state);
    ClassMemberDesc(const ClassMemberDesc& cref);
    virtual ~ClassMemberDesc();

    virtual const ClassMemberDesc * clone() const = 0;

    Locator m_loc;
    std::string m_mangledClassName;
    std::string m_mangledType;
    std::string m_memberName;
    std::string m_mangledMemberName;
    std::string m_structuredComment;

    virtual std::string getMemberKind() const = 0;
    virtual bool getValue(u64& vref) const;
    virtual bool hasValue() const; //default is false
    virtual std::string getValueAsString() const;
  };

  struct ClassMemberDescHolder
  {
    ClassMemberDescHolder(ClassMemberDesc * cp) : m_classmemberdesc(cp) {}
    ClassMemberDescHolder(const ClassMemberDescHolder& cref) : m_classmemberdesc(cref.m_classmemberdesc->clone()){ }
    ~ClassMemberDescHolder() { delete m_classmemberdesc; m_classmemberdesc = NULL; }
    const ClassMemberDesc * getClassMemberDesc() const { return m_classmemberdesc; }
  private:
    const ClassMemberDesc * m_classmemberdesc;
  };

  //key is mangledMemberName, including the mangled class it belongs
  typedef std::map<std::string, struct ClassMemberDescHolder> ClassMemberMap;
}

#endif  /* MAPCLASSMEMBERDESC_H */
