#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t228_test_parser_declintonly)
  {
    std::string GetAnswerKey()
    {
      //return std::string(" a x +b PI *\n2\n");
      return std::string("Ue_A { Int a(0);  <NOMAIN> }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("a.ulam","( a + x ) * PI;"); 
      bool rtn1 = fms->add("A.ulam","element A{Int a;}"); 
      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t228_test_parser_declintonly)
  
} //end MFM


