#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t227_test_parser_multiplestmts_binaryops)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Int a(1);  Int b(1);  Int c(1);  Int test() {  a b c 1 = = = c a b +b = f a b -b = g c d * = h d d / = e f +b g +b h +b return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      // multiple statements, each of the binary ops, where all idents == 1
      // when using the same variable for each result, the final value is returned throughout.
      // to see intermediate results use different variables:
      bool rtn1 = fms->add("A.ulam","element A{Int a, b, c; Int test() { a = b = c = 1; c = a + b; c = c - a - b; a = c * b; b = b / b; return (a + b + c); } }"); 
      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t227_test_parser_multiplestmts_binaryops)
  
} //end MFM


