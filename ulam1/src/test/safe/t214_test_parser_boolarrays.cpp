#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t214_test_parser_boolarrays)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Bool a[5](false,false,false,false,true);  Int test() {  a 1 3 +b [] true = 0 return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A { Bool a[5]; Int test() { a[1+3] = true; return 0;} }");
      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t214_test_parser_boolarrays)
  
} //end MFM


