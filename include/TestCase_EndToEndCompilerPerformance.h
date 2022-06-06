/**                                        -*- mode:C++ -*-
 * TestCase_EndToEndCompilerPerformance.h -  Load ULAM compiler test details from a file for Performance Timing
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
  \file TestCase_EndToEndCompilerPerformance.h -  Load ULAM compiler test details from a file for Performance Timing
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2018 All rights reserved.
  \gpl
*/


#ifndef TESTCASEENDTOENDCOMPILERPERFORMANCE_H
#define TESTCASEENDTOENDCOMPILERPERFORMANCE_H

#include "TestCase_EndToEndCompilerGeneric.h"
//#include <sstream>
//#include <vector>

namespace MFM{

  class TestCase_EndToEndCompilerPerformance : public TestCase_EndToEndCompilerGeneric
  {
  public:

    TestCase_EndToEndCompilerPerformance(File * input) ;

    virtual ~TestCase_EndToEndCompilerPerformance() { }

    virtual bool CheckResults(FileManagerString * fms, File * errorOutput);

  protected:

    virtual inline bool SkipEval() { return true; }

    //virtual void AddShareUlamFilesToCompile(std::vector<std::string> * ftcvecptr);

  private:

  };

}

#endif //end TESTCASEENDTOENDCOMPILERPERFORMANCE_H
