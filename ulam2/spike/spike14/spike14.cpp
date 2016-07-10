// #include "ulam.h"
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <errno.h>
#include "SourceStream.h"
#include "FileManagerStdio.h"
#include "FileStdio.h"
#include "FileManagerString.h"
#include "FileString.h"
#include "Locator.h"
#include "Lexer.h"
#include "Preparser.h"


//eulam accepts a single filename on the command line, which it
//push'es into the SourceStream to start processing.

static std::string ans_14 = "bang/b.ulam:1.1:TOK_IDENTIFIER(bar)\nno/c_ake/c.ulam:1.1:TOK_IDENTIFIER(CCC)\nbang/b.ulam:2.15:TOK_SEMICOLON\na.ulam:1.11:TOK_SEMICOLON\na.ulam:2.11:TOK_SEMICOLON\nbang/b.ulam:1.1:TOK_IDENTIFIER(bar)\nbang/b.ulam:2.15:TOK_SEMICOLON\na.ulam:2.24:TOK_SEMICOLON\na.ulam:2.36:TOK_SEMICOLON\na.ulam:3.0:TOK_EOF\n";


static bool PrecreateTest_14(MFM::FileManagerString * fms)
{

  bool rtn1 = fms->add("a.ulam","use bang.b;\nuse bang.b; load bang.b; use bang.b;\n");

  bool rtn2 = fms->add("bang/b.ulam", "bar\nuse no.c_ake.c;");

  bool rtn3 = fms->add("no/c_ake/c.ulam", "CCC");

  return (rtn1 && rtn2 && rtn3);
}


static void ExtractDirectoryFileNames(std::string firstfilename, std::string& pathout, std::string& fnameout)
{
  // find path and fileame
  size_t found = firstfilename.find_last_of("/");
  
  fnameout = firstfilename.substr(found+1); // just the filename
  if(found == std::string::npos)
    {
      pathout = ".";
    }
  else
    {
      pathout = firstfilename.substr(0,found);
    }
}


int main(int argc, char ** argv)
{  
  // Check the number of parameters
  if (argc < 2) {
    // Tell the user how to run the program
    std::cerr << "Usage: " << argv[0] << " filename" << std::endl;
    return 1;
  }

  std::string firstname(argv[1]);
  std::string filenameonly;
  std::string dirnameonly;

  ExtractDirectoryFileNames(firstname, dirnameonly, filenameonly);

  MFM::FileManagerString * fm = new MFM::FileManagerString(dirnameonly);
  if(!fm)
    {
      std::cerr << "Error in making new file manager...aborting" << std::endl;
      return 1;
    }


  //hardcoded per DHA
  if (!PrecreateTest_14((MFM::FileManagerString *) (fm)))
    {
      std::cerr << "Error in precreating test 14...aborting" << std::endl;
      delete fm;
      return 1;
    }


  MFM::SourceStream ss(fm);

  //File to accumulate test results
  MFM::File * fsResults = fm->open("results", MFM::WRITE);
  
  if(!fsResults)
    {
      delete fm;
      return 1;
    }

  MFM::Lexer * Lex = new MFM::Lexer(ss);
  MFM::Preparser * PP =  new MFM::Preparser(Lex);

  if (ss.push(filenameonly))
    {
      while(1)
	{
	  MFM::Token Tok;
	  if(PP->GetNextToken(Tok))
	    {
	      Tok.print(fsResults, PP);
	      if(Tok.m_type == MFM::TOK_EOF) break;
	    }
	  else
	    {
	      std::cerr << "main: GetNextTokenFailed: " << strerror(errno) << std::endl;
	    }
	}
    }

   //done with writing results
  delete fsResults;
  fsResults = NULL;

  int rtn = 0;
  std::string results;

  if(fm->get("results", results))
    {
      //compare results with answer
      if(results.compare(ans_14) != 0)
	{
	  rtn = -1;
	  std::cout << "spike14: INVALID results <" << results << ">" << std::endl; 
	}
      else
	{
	  std::cout << "spike14: GOOD results <" << results << ">" << std::endl; 
	}
    }
  else
    {
      std::cerr << "spike12s: MISSING results <" << results << ">" << std::endl; 
    }

  delete fm;
  delete Lex;
  delete PP;
  return rtn;
}

