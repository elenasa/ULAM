#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t223_test_parser_simplewithparens)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Int(32) barne(32);  Int(32) test() {  barne 1 3 +b 8 * = barne return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A { Int barne; Int test() { barne = (1 + 3) * 8; return barne; } }"); // case with parens
      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t223_test_parser_simplewithparens)
  
} //end MFM


