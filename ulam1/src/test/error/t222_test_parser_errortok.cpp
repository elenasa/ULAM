#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t222_test_parser_errortok)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int a(5);  Int test() {  a 3 2 +b = a return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","use foo;  Int a; Int test() {a = @3 + @2; use bar; return a; load bar;");
      bool rtn2 = fms->add("foo.ulam","ulam {  ");  //recovers from invalid character
      bool rtn3 = fms->add("bar.ulam","} \n ");
      
      if(rtn1 && rtn2 && rtn3)
	return std::string("a.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t222_test_parser_errortok)
  
} //end MFM


