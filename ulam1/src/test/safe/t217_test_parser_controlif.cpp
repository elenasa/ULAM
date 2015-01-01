#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t217_test_parser_controlif)
  {
    std::string GetAnswerKey()
    {
      // removed 1-bit bool cast from constant
      //Ue_A { Bool(1) b(true);  Int(32) a(2);  Int(32) test() {  b true cast = b ! cast cond a 1 cast = if a 2 cast = else a return } }
      return std::string("Ue_A { Bool(1) b(true);  Int(32) a(2);  Int(32) test() {  b true cast = b ! cond a 1 cast = if a 2 cast = else a return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {\n Bool b;\n Int a;\n Int test() {\n b = true;\n if( !b )\n a = 1;\n else a = 2;\n return a;\n}\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASEPARSER(t217_test_parser_controlif)

} //end MFM


