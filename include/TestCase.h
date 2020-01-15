/**                                        -*- mode:C++ -*-
 * TestCase.h -  Basic handling of Test Cases for ULAM
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
  \file TestCase.h -  Basic handling of Test Cases for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2017 All rights reserved.
  \gpl
*/


#ifndef TESTCASE_H
#define TESTCASE_H

#include "File.h"

#define BEGINTESTCASE(n)			\
  struct TestCase_##n : public TestCase

#define ENDTESTCASE(n) ; static TestCase_##n the_instance;	\
  TestCase * n = &the_instance;


namespace MFM{

  class TestCase
  {
  public:

    virtual ~TestCase() {}

    virtual bool RunTests(File * errorOutput) = 0;

  private:

  };

}

#endif //end TESTCASE_H
