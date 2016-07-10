#include <stdio.h>
#include <string>
#include <iostream>
#include "SourceStream.h"
#include "FileManagerString.h"
#include "FileString.h"


//spike1 accepts a single filename on the command line, which it
//push'es into the SourceStream to start processing.

static const char OPEN_NAME = '[';
static const char CLOSE_NAME = ']';

static std::string ans_11 = "hello no[c.ULAM\n] go yeCCC]DDDs\n world !\nrubout between the anglebrackets here: <\377>\nbye\n";


static bool PrecreateTest_11(MFM::FileManagerString * fms)
{

  bool rtn1 = fms->add("a.ulam","hello [b.ulam] world [b.ulam]!\n[f.ulam]\nbye\n");

  bool rtn2 = fms->add("b.ulam", "n[b.ulam]o[c.ULAM\n] g[a.ulam]o ye[c.ulam]s\n");

  bool rtn3 = fms->add("c.ulam", "CCC[a.ulam]]DDD");

  bool rtn4 = fms->add("f.ulam", "rubout between the anglebrackets here: <\377>");
  return (rtn1 && rtn2 && rtn3 && rtn4);
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
  //  if (argc < 2) {
  //    // Tell the user how to run the program
  //    std::cerr << "Usage: " << argv[0] << " filename" << std::endl;
  //    return 1;
  //  }

  //std::string firstname(argv[1]);
  std::string firstname("a.ulam");

  std::string filenameonly;
  std::string dirnameonly;

  ExtractDirectoryFileNames(firstname, dirnameonly, filenameonly);

  // std::cerr << "Extracting dir <" << dirnameonly << ">, filenameonly <" <<filenameonly << ">" << std::endl;

  MFM::FileManagerString * fm = new MFM::FileManagerString(dirnameonly);
  if(!fm)
    {
      std::cerr << "Error in making new file manager...aborting" << std::endl;
      return 1;
    }

  //hardcoded per DHA
  if (!PrecreateTest_11((MFM::FileManagerString *) (fm)))
    {
      std::cerr << "Error in precreating test 11...aborting" << std::endl;
      delete fm;
      return 1;
    }

  MFM::SourceStream ss(fm);
  std::string fname;   //to build
  bool filenameSeen = false;

  //FileString to accumulate test results
  MFM::File * fsResults = fm->open("results", MFM::WRITE);

  if(!fsResults)
    {
      delete fm;
      return 1;
    }
  
  if (ss.push(filenameonly))
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
		    fsResults->write(c);
		  }
		else
		  {
		    filenameSeen = false;
		    // If the push returns false, spike1
		    //  outputs the brackets and enclosed bytes literally
		    if(!ss.push(fname))
		      {
			//std::cout << OPEN_NAME << fname << CLOSE_NAME;
			fsResults->write(OPEN_NAME);
			//for(size_t i = 0; i < fname.length(); fsResults->write(fname[i++]));
			fsResults->write(fname.c_str());
			fsResults->write(CLOSE_NAME);
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
		    fsResults->write(c);
		  }
		break;
	      }
	    };
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
      if(results.compare(ans_11) != 0)
	{
	  rtn = -1;
	  std::cout << "spike12s: INVALID results <" << results << ">" << std::endl; 
	}
    }
  else
    {
      std::cerr << "spike12s: MISSING results <" << results << ">" << std::endl; 
    }
  
  delete fm;
  return rtn;
}






