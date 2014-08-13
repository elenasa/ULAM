#include <string>
#include <iostream>
#include <errno.h>
#include "TestCase_EndToEndIO.h"
#include "SourceStream.h"

namespace MFM {

static const char OPEN_NAME = '[';
static const char CLOSE_NAME = ']';

  BEGINTESTCASEIO(t13_check_lex_recursive_onlyonce_file_include)
  {  
    std::string GetAnswerKey()
    {
      std::string answerkey = "hello no[c.ULAM\n] go yeCCC]DDDs\n world !\nrubout between the anglebrackets here: <\377>\nbye\n";
      return answerkey;
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","hello [b.ulam] world [b.ulam]!\n[f.ulam]\nbye\n");
      
      bool rtn2 = fms->add("b.ulam", "n[b.ulam]o[c.ULAM\n] g[a.ulam]o ye[c.ulam]s\n");
      
      bool rtn3 = fms->add("c.ulam", "CCC[a.ulam]]DDD");

      bool rtn4 = fms->add("f.ulam", "rubout between the anglebrackets here: <\377>");
      
      if(rtn1 && rtn2 && rtn3 && rtn4)
	return std::string("a.ulam");

      return std::string("");
    }
    

    bool CheckResults(FileManagerString * fms, File * errorOutput)
    {    
      return CompareResultsWithAnswerKey(fms,errorOutput);
    }
     

    bool GetTestResults(FileManager * fm, std::string startstr, File * output)
    {
      CompilerState cs;
      SourceStream ss(fm, cs);      
      std::string fname;   //to build
      bool filenameSeen = false;
      bool rtn = true;

      if (ss.push(startstr))
	{
	  int c;
	  while( (c = ss.read()) >= 0)
	    {
	      switch (c)
		{
		case OPEN_NAME:
		  {
		    // already within a filename, becomes part of fname
		    if(filenameSeen)
		      {
			fname.push_back(c);
		      }
		    filenameSeen = true;
		    break;
		  }
		case CLOSE_NAME:
		  {
		    // not within a filename so just a regular character
		    if(!filenameSeen)
		      {
			//std::cout << (char) c;
			output->write(c);
		      }
		    else
		      {
			filenameSeen = false;
			// If the push returns false, spike1
			//  outputs the brackets and enclosed bytes literally
			if(!ss.push(fname))
			  {
			    //std::cout << OPEN_NAME << fname << CLOSE_NAME;
			    output->write(OPEN_NAME);
			    //for(size_t i = 0; i < fname.length(); fsResults->write(fname[i++]));
			    output->write(fname.c_str());
			    output->write(CLOSE_NAME);
			  }
			fname.clear();
		      }
		    break;
		  }
		default:
		  {
		    if(filenameSeen)
		      {
			fname.push_back(c);
		      }
		    else
		      {
			//std::cout << (char) c;
			output->write(c);
		      }
		    break;
		  }
		};
	    } // end while
	}
      else
	{
	  rtn = false;
	}
      
      return rtn;
  }

    
  } //end test struct
  
  ENDTESTCASEIO(t13_check_lex_recursive_onlyonce_file_include)


} //end MFM


