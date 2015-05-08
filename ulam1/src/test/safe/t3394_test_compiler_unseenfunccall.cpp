#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3394_test_compiler_unseenfunccall)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_A { /* empty class block */ }Ue_R { Int(32) test() {  typedef Colony(3u) Ish;  Colony(3u) ish;  ish ( 7u )setTailAge . 0 return } }\nUq_Colony { typedef Unsigned(UNKNOWN) Tail;  constant Unsigned(32) widthc = NONREADYCONST;  typedef Telomeree(width) Telo;  Telomeree(width) t( constant Unsigned(32) width = NONREADYCONST; );  <NOMAIN> }\nUq_Telomeree { constant Unsigned(32) width = NONREADYCONST;  typedef Unsigned(UNKNOWN) Tail;  Unsigned(UNKNOWN) age(0);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by 3392
      // Colony, with args, is Unseen at typedef time.
      bool rtn2 = fms->add("A.ulam", "use R;\nuse Colony;\n element A{\n}\n");
      bool rtn3 = fms->add("R.ulam", "element R{\n Int test() {typedef Colony(3u) Ish;\n Ish ish;\n ish.setTailAge(7u);\n return 0;\n}\n}");
      bool rtn1 = fms->add("Colony.ulam","use Telomeree;\n quark Colony(Unsigned widthc){\n typedef Telomeree(widthc) Telo;\n typedef Telo.Tail Tail;\n Telo t;\n Void setTailAge(Unsigned newage) {\n t.setAge((Tail) newage);\n }\n }");
      bool rtn4 = fms->add("Telomeree.ulam","quark Telomeree(Unsigned width){\n typedef Unsigned(width) Tail;\n Tail age;\nTail getAge(){\n return age;\n}\nVoid setAge(Tail newAge){\nage = newAge;\n}\n }");

      if(rtn1 && rtn2 && rtn3 && rtn4)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3394_test_compiler_unseenfunccall)

} //end MFM
