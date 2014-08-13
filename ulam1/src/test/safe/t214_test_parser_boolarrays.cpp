#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t214_test_parser_boolarrays)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Bool a[5](false,false,false,false,true);  Int main() {  a 1 3 +b [] true = 0 } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Bool a[5]; Int main() { a[1+3] = true; 0;} }");
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t214_test_parser_boolarrays)
  
} //end MFM


