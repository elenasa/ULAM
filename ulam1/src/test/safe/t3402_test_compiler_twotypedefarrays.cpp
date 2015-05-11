#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3402_test_compiler_twotypedefarrays)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 7\nUe_B { typedef Unsigned(2) Slot;  typedef Unsigned(8) Index;  typedef Unsigned(8) IndArr[4];  typedef Unsigned(8) IndArrTwo[2];  Unsigned(8) arr[4](0,7,0,0);  Unsigned(8) arr2[2](0,7);  Int(32) test() {  ( 1 cast 7u )hit arr2 1 [] arr 1 [] = arr 1 [] cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // with renames of typedefs, and arrays based on typedef
      bool rtn1 = fms->add("B.ulam","element B {\ntypedef Unsigned(2) Slot;\n typedef Unsigned(8) Index;\ntypedef Index IndArr[4];\n typedef Index IndArrTwo[2];\n IndArr arr;\n IndArrTwo arr2;\n Void hit(Slot slot, Unsigned weight) {\n arr[slot] = arr[slot] + weight;\n}\n Int test(){hit((Slot) 1, 7u);\n arr2[1] = arr[1];\n return arr[1];\n }\n }\n");


      if(rtn1)
	return std::string("B.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3402_test_compiler_twotypedefarrays)

} //end MFM
