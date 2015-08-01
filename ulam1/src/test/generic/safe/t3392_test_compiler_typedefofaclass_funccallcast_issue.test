#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3392_test_compiler_typedefofaclass_funccallcast_issue)
  {
    std::string GetAnswerKey()
    {
      //test bug fix: casting func call arg using typedef of another class
      return std::string("Exit status: 7\nUe_R { Ish2 ish( Colony(3u) colony( typedef Unsigned(3) Tail;  constant Unsigned(32) widthc = 3;  typedef Telomeree(3u) Telo;  Telomeree(3u) t( constant Unsigned(32) width = 3;  typedef Unsigned(3) Tail;  Unsigned(3) age(7); ); ); );  Int(32) test() {  ish colony ( 7u cast )setTailAge . . ish colony t ( )getAge . . . cast return } }\nUq_Telomeree { constant Unsigned(32) width = NONREADYCONST;  typedef Unsigned(UNKNOWN) Tail;  Unsigned(UNKNOWN) age(0);  <NOMAIN> }\nUq_Colony { typedef Unsigned(UNKNOWN) Tail;  constant Unsigned(32) widthc = NONREADYCONST;  typedef Telomeree(width) Telo;  Telomeree(width) t( constant Unsigned(32) width = NONREADYCONST; );  <NOMAIN> }\nUq_Ish2 { Colony(3u) colony( typedef Unsigned(3) Tail;  constant Unsigned(32) widthc = 3;  typedef Telomeree(3u) Telo;  Telomeree(3u) t( constant Unsigned(32) width = 3;  typedef Unsigned(3) Tail;  Unsigned(3) age(7); ); );  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by 3383,
      // likes Telomeree first!
      bool rtn3 = fms->add("R.ulam", "use Telomeree;\n use Colony;\n use Ish2;\n  element R{\nIsh2 ish;\n Int test() {  /*ish.colony.t.setAge((Unsigned(3)) 7);\n*/ ish.colony.setTailAge(7u);\n return ish.colony.t.getAge();\n}\n}");

      bool rtn1 = fms->add("Colony.ulam","quark Colony(Unsigned widthc){\n typedef Telomeree(widthc) Telo;\n typedef Telo.Tail Tail;\n Telo t;\n Void setTailAge(Unsigned newage) {\n t.setAge((Tail) newage);\n }\n }");
      bool rtn2 = fms->add("Ish2.ulam","quark Ish2{\nColony(3u) colony;\n }");
      bool rtn4 = fms->add("Telomeree.ulam","quark Telomeree(Unsigned width){\n typedef Unsigned(width) Tail;\n Tail age;\nTail getAge(){\n return age;\n}\nVoid setAge(Tail newAge){\nage = newAge;\n}\n }");

      if(rtn1 && rtn2 && rtn3 && rtn4)
	return std::string("R.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3392_test_compiler_typedefofaclass_funccallcast_issue)

} //end MFM
