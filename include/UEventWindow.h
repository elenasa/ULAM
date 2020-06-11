/**                                        -*- mode:C++ -*-
 * UEventWindow.h -  Basic handling of Event Window for ULAM
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
  \file UEventWindow.h -  Basic handling of Event Window for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2017 All rights reserved.
  \gpl
*/


#ifndef EVENTWINDOW_H
#define EVENTWINDOW_H

#include <map>
#include "itype.h"
#include "USite.h"

namespace MFM
{

  class CompilerState;  //forward

#define MAXMANHATTANDIST (4)
#define MAXWIDTH (MAXMANHATTANDIST + MAXMANHATTANDIST + 1)

  // center is 0,0 (index 40); max manhattan distance is 4 in any direction
  // index 0 is (-4,-4); index 80 is (4,4)
  struct Coord {
    s8 x, y;
    Coord(): x(0), y(0) {} //default constructor to center
    Coord(s8 x1, s8 y1): x(x1), y(y1) {}
    ~Coord() {}
    u32 convertCoordToIndex(){return ((MAXMANHATTANDIST-y) * MAXWIDTH + (MAXMANHATTANDIST-x)); }
    static Coord convertIndexToCoord(u32 i) { return Coord( ((MAXMANHATTANDIST - i%MAXWIDTH)), ( MAXMANHATTANDIST - ((int)(i/MAXWIDTH)))); }

  };


  struct less_than_coord
  {
    inline bool operator() (const Coord c1, const Coord c2)
    {
      if(c1.x < c2.x) return true;
      if(c1.x > c2.x) return false;
      if(c1.y < c2.y) return true;
      if(c1.y > c2.y) return false;
      return false;
    }
  };


  class UEventWindow
  {
  public:
    UEventWindow(CompilerState & state);
    ~UEventWindow();

    void clear();
    void init();

    bool isValidSite(u32 index, Coord& c);
    static bool isValidSite(Coord c);

    bool isSiteLive(Coord c);
    void setSiteLive(Coord c, bool b);

    UlamValue loadAtomFromSite(u32 index); //converts index into 2D coord, returns Atom
    bool storeAtomIntoSite(u32 index, UlamValue uv); //converts index into 2D coord, updates Atom
    bool storeAtomIntoSite(Coord c, UlamValue v);

    void assignUlamValue(UlamValue pluv, UlamValue ruv);
    void assignUlamValuePtr(UlamValue pluv, UlamValue puv);

    UTI getSiteElementType(u32 index); //passes through to AtomValue at site
    void setSiteElementType(u32 index, UTI type);
    void setSiteElementType(Coord c, UTI type);

    UlamValue makePtrToCenter();
    UlamValue makePtrToSite(Coord c);

  private:
    std::map<Coord, USite, less_than_coord> m_diamondOfSites;
    CompilerState & m_state;

    bool getSite(Coord c, USite * & sref);

  };
}

#endif //EVENTWINDOW_H
