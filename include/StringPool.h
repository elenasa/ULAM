/**                                        -*- mode:C++ -*-
 * StringPool.h -  Basic String Pool management for ULAM
 *
 * Copyright (C) 2014-2017 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2017 Ackleyshack LLC.
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
  \file StringPool.h -  Basic String Pool management for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2017 All rights reserved.
  \gpl
*/


#ifndef STRINGPOOL_H
#define STRINGPOOL_H

#include <stdio.h>
#include <string>
#include <string.h>
#include <map>
#include "itype.h"


namespace MFM
{

  class StringPool
  {
  public:

    StringPool();

    StringPool(std::string str); //for StringPoolUser's default constructor

    StringPool(const StringPool& spref);

    ~StringPool();

    virtual u32 getIndexForDataString(std::string str);    //< makes a new entry in map, vector if nonexistent

    u32 getIndexForNumberAsString(u32 num);

    virtual const std::string & getDataAsString(u32 dataindex);

  protected:

    std::map<u32, std::string> m_dataAsString; //<string indexed by dataindex (may be sequential or not)
    std::map<std::string,u32> m_stringToDataIndex; //< value indexes into m_dataAsString; avoid duplicates
  private:

  };
}

#endif  /* STRINGPOOL_H */
