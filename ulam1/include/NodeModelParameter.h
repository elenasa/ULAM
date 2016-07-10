/**                                        -*- mode:C++ -*-
 * NodeModelParameter.h - Node handling Model Parameters for ULAM
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
  \file NodeModelParameter.h - Node handling Model Parameters for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2015 All rights reserved.
  \gpl
*/


#ifndef NODEMODELPARAMETER_H
#define NODEMODELPARAMETER_H

#include "NodeConstant.h"
#include "SymbolParameterValue.h"

namespace MFM{

  class NodeModelParameter : public NodeConstant
  {
  public:

    NodeModelParameter(Token tok, SymbolParameterValue * symptr, CompilerState & state);
    NodeModelParameter(const NodeModelParameter& ref);
    NodeModelParameter(const NodeIdent& iref);
    virtual ~NodeModelParameter();

    virtual Node * instantiate();

    virtual void printPostfix(File * fp);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual bool isAConstant();

    virtual FORECAST safeToCastTo(UTI newType);

    virtual bool assignClassArgValueInStubCopy();

    virtual void genCode(File * fp, UlamValue& uvpass);

    virtual void genCodeToStoreInto(File * fp, UlamValue& uvpass);

  protected:
    virtual void checkForSymbol();

  private:

  };

}

#endif //NODEMODELPARAMETER_H
