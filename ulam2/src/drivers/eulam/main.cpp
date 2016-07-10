// #include "ulam.h"
#include <stdio.h>
#include <string>
#include <iostream>
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

  MFM::FileManagerStdio * fm = new MFM::FileManagerStdio(dirnameonly);
  if(!fm)
    {
      std::cerr << "Error in making new file manager...aborting" << std::endl;
      return 1;
    }
  MFM::CompilerState cs;
  MFM::SourceStream ss(fm, cs);

  //FileSdio to accumulate test results
  MFM::File * fsResults = fm->open("results", MFM::WRITE);
  
  if(!fsResults)
    {
      delete fm;
      return 1;
    }

  MFM::Lexer * Lex = new MFM::Lexer(ss, cs);
  MFM::Preparser * PP =  new MFM::Preparser(Lex, cs);

  if (ss.push(filenameonly))
    {
      while(1)
	{
	  MFM::Token Tok;
	  if(PP->getNextToken(Tok))
	    {
	      //Tok.print(fsResults, Lex);
	      Tok.print(fsResults, &cs);
	      if(Tok.m_type == MFM::TOK_EOF) break;
	    }
	  else
	    {
	      std::cerr << "main: GetNextTokenFailed" << std::endl;
	    }
	}
    }

  //done with writing results
  delete fsResults;
  delete fm;
  delete Lex;
  delete PP;
  return 0;
}

