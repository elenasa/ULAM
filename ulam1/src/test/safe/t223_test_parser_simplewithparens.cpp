#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t223_test_parser_simplewithparens)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int barne(32);  Int test() {  barne 1 3 +b 8 * = barne return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Int barne; Int test() { barne = (1 + 3) * 8; return barne; } }"); // case with parens
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t223_test_parser_simplewithparens)
  
} //end MFM


