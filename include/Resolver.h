/**                                        -*- mode:C++ -*-
 * Resolver.h -  Managing of Class UNKNOWN Subtrees for ULAM
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
  \file Resolver.h - Managing of Class UNKNOWN Subtrees for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2015-2017 All rights reserved.
  \gpl
*/


#ifndef RESOLVER_H
#define RESOLVER_H

#include <map>
#include <set>
#include <vector>
#include "itype.h"
#include "NodeConstantDef.h"
#include "UlamValue.h"

namespace MFM
{

  class CompilerState;  //forward
  class SymbolClass; //forward

  //formerly a global in CompilerState; now class-specific (see SymbolClass)
  class Resolver
  {
  public:
    Resolver(UTI instance, CompilerState& state);
    ~Resolver();

    void addUnknownTypeToken(const Token& tok, UTI huti);
    Token removeKnownTypeToken(UTI huti);
    bool hasUnknownTypeToken(UTI huti);
    bool getUnknownTypeToken(UTI huti, Token & tok);
    bool statusUnknownType(UTI huti, SymbolClass * csym);
    bool statusAnyUnknownTypeNames(); //should be resolved after parsingxb
    u32 reportAnyUnknownTypeNames();

    //these exist in a stubs only!
    bool assignClassArgValuesInStubCopy();
    u32 countNonreadyClassArgs();
    bool statusNonreadyClassArguments(SymbolClass * stubcsym);
    void linkConstantExpressionForPendingArg(NodeConstantDef * ceNode);
    bool pendingClassArgumentsForClassInstance();
    void setContextForPendingArgValues(UTI context);
    UTI getContextForPendingArgValues();
    void setContextForPendingArgTypes(UTI context);
    UTI getContextForPendingArgTypes();

    bool mapUTItoUTI(UTI fmuti, UTI touti);
    bool findMappedUTI(UTI auti, UTI& mappedUTI);

    void cloneUTImap(SymbolClass * csym);
    void cloneUnknownTypesTokenMap(SymbolClass * csym);

  protected:

  private:
    std::vector<NodeConstantDef *> m_nonreadyClassArgSubtrees; //constant expr to resolve, and empty for a class' args.
    std::map<UTI, UTI> m_mapUTItoUTI; //mult-purpose: instantiating stubs; unknown typedefs from another class
    std::map<UTI, Token> m_unknownTypeTokens; //possible unknown typedef from ancestor class

    CompilerState& m_state;
    UTI m_classUTI;
    UTI m_classContextUTIForPendingArgValues; //used to evaluate pending class arg values in use-context
    UTI m_classContextUTIForPendingArgTypes; //used to evaluate pending class arg types in stub context

    bool constantFoldNonreadyClassArgs(SymbolClass * stubcsym);

    void clearLeftoverSubtrees();
    void clearLeftoverNonreadyClassArgSubtrees();
    void clearLeftoverUnknownTypeTokens();

    bool checkUnknownTypeToResolve(UTI huti, const Token& tok);
    bool checkUnknownTypeAsClassArgument(UTI huti, const Token& tok, SymbolClass * csym);
  };

}

#endif //RESOLVER_H
