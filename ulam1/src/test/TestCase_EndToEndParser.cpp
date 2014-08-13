#include <iostream>
#include <stdio.h>
#include "TestCase_EndToEndParser.h"
#include "SourceStream.h"
#include "Lexer.h"
#include "Preparser.h"
#include "Token.h"
#include "Parser.h"

namespace MFM {


  bool TestCase_EndToEndParser::GetTestResults(FileManager * fm, std::string startstr, File * output)
  {
    CompilerState cs;
    SourceStream ss(fm, cs);      
    
    Lexer * Lex = new Lexer(ss, cs);
    Preparser * PP =  new Preparser(Lex, cs);
    Parser * P = new Parser(PP, cs);
    bool rtn = false;

    if (ss.push(startstr))
      {
	Node *  programme = P->parseProgram(output); //will be compared to answer
	//Node *  programme = P->parseProgram();
	
	if(programme != NULL)
	  {
	    if(cs.m_err.getErrorCount() == 0)
	      {
		programme->checkAndLabelType();  //side-effect
	    
		//debug only: tests' answer comparisons will fail.
		//programme->print(output);
		//output->write("\n");
		if(cs.m_err.getErrorCount() == 0)
		  {
		    programme->eval();   //side-effect, or UlamValue val

		    if(cs.m_err.getErrorCount() == 0)
		      {
			programme->printPostfix(output);
			output->write("\n");

			//char valstr[32];
			//val.getUlamValueAsString(valstr);
			//output->write(valstr);
		      }
		    else
		      output->write("evalParseTree unrecoverable EVAL FAILURE.\n");
		  }
		else
		  output->write("labelParseTree unrecoverable TYPE LABEL FAILURE.\n");
	      }
	    else
	      output->write("parseProgram unrecoverable PARSE FAILURE.\n");		
	  
	    delete programme;
	    programme = NULL;
	    rtn = true;
	  }
	else
	  {
	    output->write("parseProgram unrecoverable PARSE FAILURE.");
	    //	    output->write("--------------------------------------------------------------------------------");
	  }
      }
    else
      {
	output->write("parseProgram failed to start SourceStream.");
      }
    
    //  while(ss.read() != -1); //EOF

    delete P;
    delete PP;
    delete Lex;    
    return rtn;
  }

  

  bool TestCase_EndToEndParser::CheckResults(FileManagerString * fms, File * errorOutput)
  {    
    return CompareResultsWithAnswerKey(fms,errorOutput);
  }
      
} //end MFM
