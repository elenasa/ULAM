#include <stdio.h>
#include <string>
#include <iostream>
#include "SourceStream.h"
#include "FileManagerStdio.h"
#include "FileStdio.h"
#include "Locator.h"

//spike1 accepts a single filename on the command line, which it
//push'es into the SourceStream to start processing.

static const char OPEN_NAME = '[';
static const char CLOSE_NAME = ']';


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


static void output_line_and_byte(MFM::SourceStream * ss, int c)
{
  if(!isspace(c))
    {
      MFM::Locator cloc = ss->GetLocator();
	  	
      std::cout << "'" << (unsigned char) c << "' " << cloc.GetLineNo() << "." << cloc.GetByteNo() << " " << ss->GetPathFromLocator(cloc) << std::endl;
    }
}


static void output_unfound_filename_line_and_byte(MFM::SourceStream * ss, std::string str)
{
  MFM::Locator cloc = ss->GetLocator();
  
  std::cout << "'" << OPEN_NAME << str << CLOSE_NAME << "' " << cloc.GetLineNo() << "." << cloc.GetByteNo() << " " << ss->GetPathFromLocator(cloc) << std::endl;
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

  MFM::SourceStream ss(fm);
  std::string fname;   //to build
  bool filenameSeen = false;

  //FileSdio to accumulate test results
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
		    output_line_and_byte(&ss, c);
		    //std::cout << (char) c ;
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
			output_unfound_filename_line_and_byte(&ss, fname);

			fsResults->write(OPEN_NAME);
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
		    output_line_and_byte(&ss, c);
		    fsResults->write(c);
		  }
		break;
	      }
	    };
	}
    }

  //done with writing results
  delete fsResults;
  delete fm;
  return 0;
}






