#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t217_test_parser_controlif)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Bool b(true);  Int a(2);  Int test() {  b true = b ! cond a 1 = if a 2 = else a return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Bool b; Int a; Int test() { b = true; if( !b ) a = 1; else a = 2; return a;} }");
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t217_test_parser_controlif)
  
} //end MFM


