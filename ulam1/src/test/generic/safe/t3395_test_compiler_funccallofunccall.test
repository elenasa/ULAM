#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3395_test_compiler_funccallofunccall)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_A { /* empty class block */ }Ue_R { Int(32) test() {  Colony(3u) c;  c ( c ( )getTailAge . )setTailAge . 0 cast return } }\nUq_Colony { typedef Unsigned(UNKNOWN) Tail;  constant Unsigned(32) widthc = NONREADYCONST;  typedef Telomeree(width) Telo;  Telomeree(width) t( constant Unsigned(32) width = NONREADYCONST; );  <NOMAIN> }\nUq_Telomeree { constant Unsigned(32) width = NONREADYCONST;  typedef Unsigned(UNKNOWN) Tail;  Unsigned(UNKNOWN) age(0);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by 3392
      // Colony, with args, is Unseen at typedef time.

      bool rtn3 = fms->add("A.ulam","use R;\n use Colony;\n element A {\n }\n");
      bool rtn2 = fms->add("R.ulam"," element R{\n Int test(){\nColony(3u) c;\n c.setTailAge(c.getTailAge());\n return 0;\n }\n}\n");

      bool rtn1 = fms->add("Colony.ulam","use Telomeree;\n quark Colony(Unsigned widthc){\n typedef Telomeree(widthc) Telo;\n typedef Telo.Tail Tail;\n Telo t;\n Void setTailAge(Tail newage) {\n t.setAge(newage);\n }\n Tail getTailAge() {\n return t.getAge();\n}\n }\n");
      bool rtn4 = fms->add("Telomeree.ulam","quark Telomeree(Unsigned width){\n typedef Unsigned(width) Tail;\n Tail age;\nTail getAge(){\n return age;\n}\nVoid setAge(Tail newAge){\nage = newAge;\n}\n }");

      if(rtn1 &&  rtn2 && rtn3 && rtn4)
	return std::string("A.ulam");
      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3395_test_compiler_funccallofunccall)

} //end MFM
