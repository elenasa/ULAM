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
#include "TestCase.h"
#include "itype.h"

namespace MFM {
//static std::vector<bool(MFM::TestCase::*)(MFM::File *)> m_tests;
//#define XX(a) m_tests.push_back(a.RunTests(errorOutput));

//do this before extern XX
#define XX(a) #a,
  static const char * m_test_strings[] = {
#include "../../../build/test/tests.inc"
  };
#undef XX


#define XX(a) extern TestCase * (a);
#include "../../../build/test/tests.inc"
#undef XX


static std::vector<TestCase*> m_tests;
#define XX(a) m_tests.push_back((a));

void initTests()
  {
#include "../../../build/test/tests.inc"
  }
#undef XX

}  //end MFM


int main(int argc, char ** argv)
{  

  std::string testdir = ".";

  MFM::FileManagerString * fm = new MFM::FileManagerString(testdir);
  if(!fm)
    {
      std::cerr << "Error in making new file manager...aborting" << std::endl;
      return 1;
    }


  MFM::initTests();

  int failed = 0;
  int passed = 0;

  //  for(std::vector<MFM::TestCase*>::iterator it = MFM::m_tests.begin(); it != MFM::m_tests.end(); it++)
  for(unsigned int i=0; i < MFM::m_tests.size(); i++)
    {
      //File to accumulate test results
      MFM::File * errorOutput = fm->open("results", MFM::WRITE);      
      if(!errorOutput)
	{
	  delete fm;
	  return 1;
	}

      if ( (MFM::m_tests[i])->RunTests(errorOutput))
	passed++;
      else
	{
	  failed++;
	  std::string results;
	  fm->get("results", results);
	  std::cerr << "TestCase: <" << MFM::m_test_strings[i] << "> FAILED!!\n" << results << std::endl; 
	  std::cerr << "--------------------------------------------------------------------------------" << std::endl;
	}

      delete errorOutput;  //don't want to accumulate between tests, or leak, delete closes.

    }

  if(failed > 0)
    {
      std::cerr << "TestCase Summary: " << passed << " passed, " << failed << " failed." << std::endl;
    }
  
  delete fm;
  MFM::m_tests.clear();
  return (failed > 0);
}

