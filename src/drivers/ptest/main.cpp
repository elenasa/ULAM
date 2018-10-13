// #include "ulam.h"
#include <iostream>
#include <strings.h>  /* for rindex */
#include "SourceStream.h"
#include "FileManagerStdio.h"
#include "FileStdio.h"
#include "FileString.h"
#include "Compiler.h"
#include "UlamUtil.h"
#include "TestCase_EndToEndCompilerPerformance.h"

namespace MFM
{
  struct PTestDriver {
    const char * m_progname;
    FileManagerStdio m_fms;
    File * m_testFile;
    std::string m_testPath; // supplied arg
    std::string m_testFullPath; // same or expanded path

    void doVersion()
    {
      fprintf(stdout,"gtest-%d.%d.%d %08x%06x\n",
              1,0,0,BUILD_DATE,BUILD_TIME);
      exit(0);
    }

    void doHelp()
    {
      fprintf(stderr,
              "Usage: %s [SWITCH] path/to/ulam.test\n"
              "\n"
              " -h      Print this help and exit\n"
              " -V      Print version info and exit\n"
              "\n",
              m_progname);
      exit(0);
    }

    void die(std::string msg)
    {
      fprintf(stderr,
              "ERROR: %s\n"
              "\n"
              "Try: %s -h\n"
              "for help\n",
              msg.c_str(), m_progname);
      exit(1);
    }


    PTestDriver(int argc, char ** argv)
      : m_fms(".")
      , m_testFile(0)
    {
      if (argc == 0) die("No program name?");
      m_progname = argv[0];
      --argc; ++argv;

      if (argc > 1) die("Too many arguments");
      if (argc < 1) die("Too few arguments");

      if (!strcmp(argv[0],"-h")) doHelp();
      if (!strcmp(argv[0],"-V")) doVersion();

      // Assume it's a test name.
      m_testPath = argv[0];
    }

    ~PTestDriver()
    {
      delete m_testFile;
    }

    bool runTest()
    {
      FileStdio errOut(stderr, WRITE);
      m_testFile = m_fms.open(m_testPath, READ, m_testFullPath);
      if (!m_testFile) die("could not read: " + m_testPath);

      TestCase_EndToEndCompilerPerformance tcg(m_testFile);
      return tcg.RunTests(&errOut);
    }

  };
} /* namespace MFM */

int main(int argc, char ** argv)
{
  MFM::PTestDriver driver(argc, argv);
  bool ret = driver.runTest();
  return ret? 0 : 1;
} //main
