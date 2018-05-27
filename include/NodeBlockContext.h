/**                                        -*- mode:C++ -*-
 * NodeBlockContext.h - Basic handling of Contexts for ULAM
 *
 * Copyright (C) 2016-2018 The Regents of the University of New Mexico.
 * Copyright (C) 2016-2018 Ackleyshack LLC.
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
  \file NodeBlockContext.h - Basic handling of Contexts for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2016-2018 All rights reserved.
  \gpl
*/


#ifndef NODEBLOCKCONTEXT_H
#define NODEBLOCKCONTEXT_H

#include "NodeBlock.h"
#include "StringPoolUser.h"
#include "TargetMap.h"
#include "MapClassMemberDesc.h"
#include <set>

namespace MFM{

  class NodeBlockContext : public NodeBlock
  {
  public:

    NodeBlockContext(NodeBlock * prevBlockNode, CompilerState & state);

    NodeBlockContext(const NodeBlockContext& ref);

    virtual ~NodeBlockContext();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual bool isAClassBlock() = 0;

    virtual StringPoolUser& getUserStringPoolRef();
    virtual void setUserStringPoolRef(const StringPoolUser& spref); //for instantiated templates

    virtual bool hasStringDataMembers();

    bool classConstantsReady();

    virtual void addTargetDescriptionToInfoMap(TargetMap& classtargets, u32 scid) = 0;
    virtual void addMemberDescriptionsToInfoMap(ClassMemberMap& classmembers) = 0;

    virtual void generateTestInstance(File * fp, bool runtest) = 0;

    void addUlamTypeKeyToSet(UlamKeyTypeSignature key);
    void copyUlamTypeKeys(NodeBlockContext * toblock) const;
    bool hasUlamTypeKey(UlamKeyTypeSignature key);
    bool hasUlamType(UTI uti);
    bool searchHasAnyUlamTypeASubclassOf(UTI suti);
    void genUlamTypeImmediateDefinitions(File * fp);

  protected:
    StringPoolUser m_upool; //for double quoted strings only
    bool m_classConstantsReady;

  private:

  struct less_than_key
  {
    //see operator< in UlamKeyTypeSignature
    inline bool operator() (const UlamKeyTypeSignature key1, const UlamKeyTypeSignature key2)
    {
      return (key1 < key2);
    }
  };


    std::set<UlamKeyTypeSignature, less_than_key> m_hasUlamTypes; //deref complete keys; used to genCode include files

  };

}

#endif //end NODEBLOCKCONTEXT_H
