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
    //Node * programme = C.start(fm,startstr,output);
    Node * programme = NULL;
    if(C.start(fm,startstr, output, programme) == 0)
      {
	rtn = true;
	if(C.labelParseTree(programme, output) == 0)
	  if(C.evalParseTree(programme, output) == 0)
	    C.printPostFix(programme, output);
	  else
	    output->write("evalParseTree unrecoverable EVAL FAILURE.\n");
	else
	  output->write("labelParseTree unrecoverable TYPE LABEL FAILURE.\n");
      }
    else
      {	
	output->write("parseProgram unrecoverable PARSE FAILURE.\n");
	//	output->write("--------------------------------------------------------------------------------");
      }
    
    delete programme;
    return rtn;
  }

  

  bool TestCase_EndToEndCompiler::CheckResults(FileManagerString * fms, File * errorOutput)
  {    
    return CompareResultsWithAnswerKey(fms,errorOutput);
  }
      
} //end MFM
