#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t227_test_parser_multiplestmts_binaryops)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Int c(0);  Int d(1);  Int test() {  Int a;  Int b;  a b c 1 = = = c a b +b = c c a -b b -b = a c b * = b b b / = d a b +b c +b = a b +b c +b return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      // multiple statements, each of the binary ops, where all idents == 1
      // when using the same variable for each result, the final value is returned throughout.
      // to see intermediate results use different variables:
      bool rtn1 = fms->add("A.ulam","element A{Int c, d; Int test() { Int a, b; a = b = c = 1; c = a + b; c = c - a - b; a = c * b; b = b / b; d = (a + b + c); return (a + b + c); } }"); 
      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t227_test_parser_multiplestmts_binaryops)
  
} //end MFM


