#include <iostream>
#include <stdio.h>
#include "TestCase_EndToEndCompiler.h"
#include "Compiler.h"

namespace MFM {

  bool TestCase_EndToEndCompiler::GetTestResults(FileManager * fm, std::string startstr, File * output)
  {
    Compiler C;
    bool rtn = false;

    // error messages appended to output are compared to answer
    if(C.parseProgram(fm, startstr, output) == 0)
      {
	rtn = true;
	if(C.checkAndTypeLabelProgram(output) == 0)
	  {
	    //#define SKIP_EVAL
#ifndef SKIP_EVAL
	    if(C.testProgram(output) == 0)
	      {
		C.printPostFix(output);
		C.generateCodedProgram(output);
	      }
	    else
	      output->write("Unrecoverable Program Test FAILURE.\n");
#else
	    // skip eval..
	    C.generateCodedProgram(output);
#endif
	  }
	else
	  output->write("Unrecoverable Program Type Label FAILURE.\n");
      }
    else
      {
	output->write("Unrecoverable Program Parse FAILURE.\n");
      }
    return rtn;
  } //GetTestResults


  bool TestCase_EndToEndCompiler::CheckResults(FileManagerString * fms, File * errorOutput)
  {
    return CompareResultsWithAnswerKey(fms,errorOutput);
  }

} //end MFM
