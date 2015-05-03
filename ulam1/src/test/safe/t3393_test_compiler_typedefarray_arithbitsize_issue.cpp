#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3393_test_compiler_typedefarray_arithbitsize_issue)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 7\nUe_B { typedef Unsigned(2) Slot;  typedef Unsigned(8) Index;  typedef Unsigned(8) IndArr[4];  Unsigned(8) arr[4](0,3,0,0);  Int(32) test() {  ( 1 cast 7u )hit arr 1 [] cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // with renames of typedefs, and arrays based on typedef
      bool rtn1 = fms->add("B.ulam","element B {\ntypedef Unsigned(2) Slot;\n typedef Unsigned(8) Index;\ntypedef Index IndArr[4];\nIndArr arr;\n  Void hit(Slot slot, Unsigned weight) {\n arr[slot] = arr[slot] + weight;\n}\n Int test(){hit((Slot) 1, 7u);\n return arr[1];\n }\n }\n");


      if(rtn1)
	return std::string("B.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3393_test_compiler_typedefarray_arithbitsize_issue)

} //end MFM
