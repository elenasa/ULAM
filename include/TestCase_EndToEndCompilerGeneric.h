/**                                        -*- mode:C++ -*-
 * TestCase_EndToEndCompilerGeneric.h -  Load ULAM compiler test details from a file
 *
 * Copyright (C) 2015-2018 The Regents of the University of New Mexico.
 * Copyright (C) 2015-2018 Ackleyshack LLC.
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
  \file TestCase_EndToEndCompilerGeneric.h -  Load ULAM compiler test details from a file
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2015-2018 All rights reserved.
  \gpl
*/


#ifndef TESTCASEENDTOENDCOMPILERGENERIC_H
#define TESTCASEENDTOENDCOMPILERGENERIC_H

#include "TestCase_EndToEndCompiler.h"
#include <sstream>
#include <vector>

namespace MFM{

  class TestCase_EndToEndCompilerGeneric : public TestCase_EndToEndCompiler
  {
  public:

    TestCase_EndToEndCompilerGeneric(File * input) ;

    virtual ~TestCase_EndToEndCompilerGeneric() { }

    /** setup test; return the string to start with */
    virtual std::string PresetTest(FileManagerString * fms) ;

    virtual bool GetTestResults(FileManager * fm, std::string startstr, File * output);

  protected:
    struct InputFile {
      std::string m_fileName;
      std::ostringstream m_contents;
      InputFile() { }
      InputFile& operator=(const InputFile & in)
      {
        m_fileName = in.m_fileName;
        m_contents.clear();
        m_contents.str(in.m_contents.str());
        return *this;
      }
      InputFile(const InputFile & in)
        : m_fileName(in.m_fileName)
        , m_contents()
      {
        m_contents << in.m_contents.str();
      }
    };

    std::vector<InputFile> m_inputFiles;
    std::vector<std::string> m_stdlibUlamFiles;
    std::vector<std::string> m_coreUlamFiles;

    virtual std::string GetAnswerKey() ;

    virtual inline bool SkipEval() { return false; }

  private:

    enum { START, IN_FILE, ANSWER } m_state;

    std::string m_currentLine;
    s32 getInput(u32 index) {
      if (index >= m_currentLine.length()) return -1;
      return m_currentLine.at(index);
    }

    std::string m_testName;

    std::string m_startFileName;

    std::ostringstream m_answerKey;

    void newInputFile(std::string str) ;
    InputFile & getLast() ;
    void newFileData(std::string str) ;
    void newAnswerKeyData(std::string str) ;

    void parseTest(File & input) ;
    void readLine(File & input) ;

    void die(std::string msg) ;

    void addShareUlamFilesToFileManager(FileManagerString & fms) ;
    void addStdlibUlamFiles(FileManagerString & fms) ;
    void addCoreUlamFiles(FileManagerString & fms) ;
  };

}

#endif //end TESTCASEENDTOENDCOMPILERGENERIC_H
