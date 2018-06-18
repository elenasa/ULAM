#include <iostream>
#include <stdio.h>
#include "Compiler.h"
#include "FileManagerStdio.h"
#include "TestCase_EndToEndCompilerPerformance.h"

namespace MFM {

  TestCase_EndToEndCompilerPerformance::TestCase_EndToEndCompilerPerformance(File * input)
    : TestCase_EndToEndCompilerGeneric(input) { }

  bool TestCase_EndToEndCompilerPerformance::CheckResults(FileManagerString * fms, File * errorOutput)
  {
    return true; //no check for performance testing
  }

} //end MFM
