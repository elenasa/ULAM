#include "TestCase_EndToEndIO.h"
#include "SourceStream.h"
#include "Lexer.h"
#include "Preparser.h"
#include "Token.h"

namespace MFM {

  bool TestCase_EndToEndIO::RunTests(File * errorOutput)
  {
    // Do some stuff right here to run the tests
    // return true if all okay.
    // output messaes to errorOutput and return false
    // if anything goes wrong
      FileManagerString * fm = new FileManagerString(".");
      if(!fm)
	{
	  errorOutput->write("Error in making new file manager...aborting");
	  return false;
	}
      
      
      std::string astr = PresetTest(fm);
      if (astr.empty())
	{
	  errorOutput->write("Error presetting test...aborting");
	  delete fm;
	  return false;
	}

      //File to accumulate test results
      File * fsResults = fm->open("results", WRITE);
      
      if(!fsResults)
	{
	  errorOutput->write("Error in opening results for write...aborting");
	  delete fm;
	  return false;
	}      

      bool rtn;
      if( (rtn = GetTestResults(fm, astr, fsResults)) )
	{
	  rtn = CheckResults(fm, errorOutput);
	}
      else
	{
	  std::string errresults;
	  if(fm->get("results", errresults))
	    {
	      errorOutput->write(errresults.c_str());
	    }
	  else
	    {
	      errorOutput->write("Error, and output missing.");
	    }
	}

      //done with writing results
      delete fsResults;
      delete fm;
      return rtn;
    }


  bool TestCase_EndToEndIO::GetTestResults(FileManager * fm, std::string startstr, File * output)
  {
    CompilerState cs;
    SourceStream ss(fm, cs);      
    
    Lexer * Lex = new Lexer(ss, cs);
    Preparser * PP =  new Preparser(Lex, cs);
    
    if (ss.push(startstr))
      {
	while(1)
	  {
	    Token Tok;
	    // should this be generalized?
	    if(PP->getNextToken(Tok))
	      {
		Tok.print(output, &cs);
		if(Tok.m_type == TOK_EOF) break;
	      }
	    else
	      {
		output->write("GetNextTokenFailed: "); // << strerror(errno) << std::endl;
		delete Lex;
		delete PP;
		return false;
	      }
	  }
      }
    
    delete Lex;
    delete PP;
    
    return true;
  }
  

  bool TestCase_EndToEndIO::CompareResultsWithAnswerKey(FileManagerString * fms, File * errorOutput)
  {      
    bool rtn = true;
    std::string results;
    
    if(fms->get("results", results))
	{
	  //compare results with answer
	  if(results.compare(GetAnswerKey()) != 0)
	    {
	      std::string str("INVALID results:\n");
	      str.append(results);
	      errorOutput->write(str.c_str());
	      rtn = false;
	    }
	}
      else
	{
	  std::string str("MISSING results <");
	  str.append(results);
	  str.append(">");
	  errorOutput->write(str.c_str());
	  rtn = false;
	}
   
      return rtn;
    }
     

  
} //end MFM
