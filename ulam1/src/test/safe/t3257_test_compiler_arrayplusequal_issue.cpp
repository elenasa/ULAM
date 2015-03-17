#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3257_test_compiler_arrayplusequal_issue)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_B { typedef Unsigned(8) Count;  typedef Unsigned(8) CountArray[4];  typedef Unsigned(2) Slot;  Unsigned(8) hitCount[4](1,0,0,0);  Int(32) test() {  ( 0 cast 1 cast )hit 0 return ; } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      /*
	 typedef Unsigned(8) Count;
	 typedef Count CountArray[4];
	 typedef Unsigned(2) Slot;
	 CountArray hitCount;

	 Void hit(Slot slot, Unsigned weight) {
	    hitCount[slot] += weight;
	 }
	 // //! WindowUtils.ulam:75:     hitCount[slot] += weight;
	 */
      bool rtn1 = fms->add("B.ulam","element B {\n typedef Unsigned(8) Count;\n typedef Count CountArray[4];\n typedef Unsigned(2) Slot;\n	 CountArray hitCount;\n	 Void hit(Slot slot, Unsigned weight) {\n hitCount[slot] += weight;\n }\n Int test(){\n hit((Slot) 0, (Unsigned) 1);\n return 0;\n;}\n}\n");

      // test system quark with native overloaded print funcs; assert
      //bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1)
	return std::string("B.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3257_test_compiler_arrayplusequal_issue)

} //end MFM
