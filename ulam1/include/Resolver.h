/**                                        -*- mode:C++ -*-
 * Resolver.h -  Managing of Class UNKNOWN Subtrees for ULAM
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
  \file Resolver.h - Managing of Class UNKNOWN Subtrees for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2015 All rights reserved.
  \gpl
*/


#ifndef RESOLVER_H
#define RESOLVER_H

#include <map>
#include <set>
#include <vector>
#include "itype.h"
#include "UlamValue.h"
#include "UlamType.h"
#include "NodeTypeBitsize.h"
#include "NodeSquareBracket.h"
#include "NodeConstantDef.h"

namespace MFM
{

  class CompilerState;  //forward

  //formerly a global in CompilerState; now class-specific (see SymbolClass)
  class Resolver
  {
  public:
    Resolver(UTI instance, CompilerState& state);
    ~Resolver();

    NodeTypeBitsize * findUnknownBitsizeUTI(UTI auti) const;
    NodeSquareBracket * findUnknownArraysizeUTI(UTI auti) const;

    void cloneConstantExpressionSubtreesByUTI(UTI olduti, UTI newuti, const Resolver& templateRslvr);
    void cloneNamedConstantExpressionSubtrees(const Resolver& templateRslvr);

    bool statusUnknownConstantExpressions();
    bool statusNonreadyClassArguments();

    void constantFoldIncompleteUTI(UTI uti);

    void linkConstantExpression(UTI uti, NodeTypeBitsize * ceNode);
    void linkConstantExpression(UTI uti, NodeSquareBracket * ceNode);
    void linkConstantExpression(NodeConstantDef * ceNode);

    void linkArrayUTItoScalarUTI(UTI suti, UTI auti);
    void updatelinkedArrayUTIsWithKnownBitsize(UTI suti);
    std::map<UTI, std::set<UTI> >::iterator getLinkedArrayIterator();
    bool isLinkedArrayEnd(std::map<UTI, std::set<UTI> >::iterator it);

    //these exist in a shallow class only!
    void linkConstantExpressionForPendingArg(NodeConstantDef * ceNode);
    bool pendingClassArgumentsForClassInstance();

  protected:
    std::map<UTI, std::set<UTI> > m_scalarUTItoArrayUTIs; //help update array's bitsizes when scalar's is known

  private:
    std::map<UTI, NodeTypeBitsize *> m_unknownBitsizeSubtrees; //constant expr to resolve, and empty
    std::map<UTI, NodeSquareBracket *> m_unknownArraysizeSubtrees;  //constant expr to resolve, and empty

    std::set<NodeConstantDef *> m_nonreadyNamedConstantSubtrees; //constant expr to resolve, and empty; various scopes
    std::vector<NodeConstantDef *> m_nonreadyClassArgSubtrees; //constant expr to resolve, and empty for a class' args.

    CompilerState& m_state;
    UTI m_classUTI;

    bool statusUnknownBitsizeUTI();
    bool statusUnknownArraysizeUTI();
    bool statusNonreadyNamedConstants();

    bool constantFoldUnknownBitsize(UTI auti, s32& bitsize);
    bool constantFoldUnknownArraysize(UTI auti, s32& arraysize);

    bool constantFoldNonreadyClassArgs();

    void clearLeftoverSubtrees();
  };

}

#endif //RESOLVER_H
