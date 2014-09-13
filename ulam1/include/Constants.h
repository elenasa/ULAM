/**                                        -*- mode:C++ -*-
 * Constants.h Useful common constants for ULAM
 *
 * Copyright (C) 2014 The Regents of the University of New Mexico.
 * Copyright (C) 2014 Ackleyshack LLC.
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
  \file Constants.h Useful common constants for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \gpl
*/

#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace MFM {

  enum STORAGE { IMMEDIATE = 0, EVENTWINDOW, STACK, EVALRETURN, UNPACKEDSTRUCT};
  enum PACKFIT { UNPACKED = 0, PACKED, PACKEDLOADABLE};

#define WritePacked(p) (p == PACKED || p == PACKEDLOADABLE)

#ifndef BITSPERATOM
#define BITSPERATOM (96)
#endif //BITSPERATOM
  
#ifndef ATOMFIRSTSTATEBITPOS
#define ATOMFIRSTSTATEBITPOS (25)
#endif //ATOMFIRSTSTATEBITPOS

#ifndef MAXSTATEBITS
#define MAXSTATEBITS (BITSPERATOM - ATOMFIRSTSTATEBITPOS)
#endif //MAXSTATEBITS

#ifndef MAXBITSPERQUARK
#define MAXBITSPERQUARK (32)
#endif //MAXBITSPERQUARK

#ifndef MAXBITSPERINT
#define MAXBITSPERINT (32)
#endif //MAXBITSPERINT

#ifndef BITSPERBOOL
#define BITSPERBOOL (1)
#endif //BITSPERBOOL


#ifndef BASE10
#define BASE10 (10)
#endif //BASE10

} //MFM

#endif //CONSTANTS_H
