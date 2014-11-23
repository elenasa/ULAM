// #include "ulam.h"
#include <iostream>
#include <strings.h>  /* for rindex */
#include "SourceStream.h"
#include "FileManagerStdio.h"
#include "FileStdio.h"
#include "FileString.h"

static const char * progname;

namespace MFM
{
  class DriverState {
  public:
    DriverState()
      : m_fileManager(0)
      , m_sourceFile(0)
      , m_stdout(new FileStdio(stdout, MFM::WRITE))
      , m_stderr(new FileStdio(stderr, MFM::WRITE))
    { }

    ~DriverState() {
      delete m_fileManager; m_fileManager = 0;
      delete m_sourceFile;  m_sourceFile = 0;
      delete m_stdout;      m_stdout = 0;
      delete m_stderr;      m_stderr = 0;
    }

    /** Usage error abort */
    void UDie(const char * msg)
    {
      std::cerr << "Error: " << msg << std::endl;
      std::cerr << "Usage: " << progname << " PATH/TO/FILE.ulam"  << std::endl;
      throw 1;
    }

    /** Internal error abort */
    void IDie(const char * msg)
    {
      std::cerr << "INTERNAL ERROR: " << msg << std::endl;
      std::cerr << "Exiting"  << std::endl;
      throw 1;
    }

    void SetTarget(char * path)
    {
      char * lastSlash = rindex(path, '/');

      std::string dir;
      std::string file;
      if (!lastSlash) {
        dir = ".";
        file = path;
      } else {
        *lastSlash = '\0';
        dir = path;
        file = lastSlash + 1;
      }

      std::cout << "IN DIRECTORY: " << dir << " COMPILE: " << file << std::endl;

      if (m_fileManager) IDie("SetDirectory");

      m_fileManager = new FileManagerStdio(dir);
      if(!m_fileManager) {
        UDie("Couldn't init file manager - bad directory?");
      }

      m_sourceFile = m_fileManager->open(file, MFM::READ);
      if(!m_sourceFile) {
        UDie("Couldn't open source file - bad filename?");
      }
    }

    /** return exit status:
        0 for good enough to continue,
        non-zero for any fatal errors
    */
    int RunCompilation()
    {
      m_stderr->write("RUN COMPILATION HERE\n");
      m_stderr->write("Reading from: m_sourceFile\n");
      m_stderr->write("Writing to:   ?? locations inside m_fileManager\n");
      m_stderr->write("Errors to:    m_stderr\n");
      m_stderr->write("Returning:    0 iff compilation should continue on to C-level\n");
      return 1;
    }

  private:
    FileManagerStdio * m_fileManager;
    File * m_sourceFile;
    File * m_stdout;
    File * m_stderr;
  };
} /* namespace MFM */

int main(int argc, char ** argv)
{
  try {
    MFM::DriverState ds;

    progname = argv[0];
    --argc; ++argv;
    if (argc != 1)
      ds.UDie("Need just one argument");

    ds.SetTarget(argv[0]);

    return ds.RunCompilation();
  }
  catch (int status) {
    return status;
  }
}

#if 0
  MFM::initTests();

  int failed = 0;
  int passed = 0;

  //  for(std::vector<MFM::TestCase*>::iterator it = MFM::m_tests.begin(); it != MFM::m_tests.end(); it++)
  for(unsigned int i=0; i < MFM::m_tests.size(); i++)
    {
      //File to accumulate test results
      MFM::File * errorOutput = fm->open("results", MFM::WRITE);
      if(!errorOutput)
	{
	  delete fm;
	  return 1;
	}

      if ( (MFM::m_tests[i])->RunTests(errorOutput))
	passed++;
      else
	{
	  failed++;
	  std::string results;
	  fm->get("results", results);
	  std::cerr << "TestCase: <" << MFM::m_test_strings[i] << "> FAILED!!\n" << results << std::endl;
	  std::cerr << "--------------------------------------------------------------------------------" << std::endl;
	}

      delete errorOutput;  //don't want to accumulate between tests, or leak, delete closes.

    }

  if(failed > 0)
    {
      std::cerr << "TestCase Summary: " << passed << " passed, " << failed << " failed." << std::endl;
    }

  delete fm;
  MFM::m_tests.clear();
  return (failed > 0);
}
#endif
