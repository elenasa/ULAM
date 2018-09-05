#include <iostream>
#include <stdio.h>
#include "Compiler.h"
#include "FileManagerStdio.h"
#include "TestCase_EndToEndCompilerPerformance.h"

namespace MFM {

  TestCase_EndToEndCompilerPerformance::TestCase_EndToEndCompilerPerformance(File * input)
    : TestCase_EndToEndCompilerGeneric(input) {

    //more of share/ulam/stdlib (note, unlike 'gen' tests which use spike
    //native .tcc, 'perf' uses share natives -- see src/test/generic/Makefile)
    m_stdlibUlamFiles.push_back("EventWindow.ulam");
    m_stdlibUlamFiles.push_back("AtomUtils.ulam");
    m_stdlibUlamFiles.push_back("C2D.ulam");
    m_stdlibUlamFiles.push_back("C2D.inc");
    m_stdlibUlamFiles.push_back("MDist.ulam");
    m_stdlibUlamFiles.push_back("Fail.ulam");
    m_stdlibUlamFiles.push_back("ByteStream.ulam");
    m_stdlibUlamFiles.push_back("ByteStreamString.ulam");

    //any others?
    m_stdlibUlamFiles.push_back("ByteStreamArray.ulam");
    m_stdlibUlamFiles.push_back("ColorUtils.ulam");
    m_stdlibUlamFiles.push_back("DebugUtils.ulam");
    m_stdlibUlamFiles.push_back("Logger.ulam");
    m_stdlibUlamFiles.push_back("Once.ulam");
    m_stdlibUlamFiles.push_back("Random.ulam");
    m_stdlibUlamFiles.push_back("SiteUtils.ulam");
    m_stdlibUlamFiles.push_back("SiteVisitor.ulam");
    m_stdlibUlamFiles.push_back("UMod.ulam");
    m_stdlibUlamFiles.push_back("WindowScanner.ulam");
    m_stdlibUlamFiles.push_back("WindowServices.ulam");
    m_stdlibUlamFiles.push_back("XTimer.ulam");
  }

  bool TestCase_EndToEndCompilerPerformance::CheckResults(FileManagerString * fms, File * errorOutput)
  {
    std::string errresults;
    if(fms->get("results", errresults))
      {
	errorOutput->write(errresults.c_str());
      }
    else
      {
	errorOutput->write("Error, and output missing.");
      }
    return true; //no check for performance testing
  }

} //end MFM
