// #include "ulam.h"
#include <iostream>
#include <strings.h>  /* for rindex */
#include "SourceStream.h"
#include "FileManagerStdio.h"
#include "FileStdio.h"
#include "FileString.h"
#include "Compiler.h"

static const char * progname;
static void doHelp()
{
  fprintf(stderr,
          "Usage:\n"
          " %s -h              Print this help and exit\n"
          " %s -V              Print version info and exit\n"
          " %s FILE.ulam       Compile FILE.ulam in its own dir\n"
          " %s FILE.ulam DIR   Compile FILE.ulam with results to DIR\n",
          progname, progname, progname, progname);
  exit(0);
}

namespace MFM
{
  class DriverState {
  public:
    DriverState()
      : m_srcFileManager(0)
      , m_outFileManager(0)
      , m_sourceFile(0)
      , m_stdout(new FileStdio(stdout, MFM::WRITE))
      , m_stderr(new FileStdio(stderr, MFM::WRITE))
      , m_hasTestMethod(false)
      , m_isAQuark(false)
    { }

    ~DriverState() {
      delete m_srcFileManager; m_srcFileManager = 0;
      delete m_outFileManager; m_outFileManager = 0;
      delete m_sourceFile;     m_sourceFile = 0;
      delete m_stdout;         m_stdout = 0;
      delete m_stderr;         m_stderr = 0;
    }

    /** Usage error abort */
    void UDie(const char * msg)
    {
      std::cerr << "Error: " << msg << std::endl;
      std::cerr << "Type: '" << progname << " -h'   for help"  << std::endl;
      throw 1;
    }

    /** Internal error abort */
    void IDie(const char * msg)
    {
      std::cerr << "INTERNAL ERROR: " << msg << std::endl;
      std::cerr << "Exiting"  << std::endl;
      throw 1;
    }

    void SplitPath(char * path, std::string & dir, std::string & file)
    {
      char * lastSlash = rindex(path, '/');

      if (!lastSlash) {
        dir = "";
        file = path;
      } else {
        *lastSlash = '\0';
        dir = path;
        file = lastSlash + 1;
      }
    }

    std::string GetMangledTarget()
    {
      return m_mangledTarget;
    }

    std::string HasTestMethod()
    {
      return m_hasTestMethod ? (m_isAQuark ? "QTEST" : "TEST") : "NOTEST";
    }

    void SetTarget(char * path, char * outpath)
    {
      std::string dir;
      std::string file;
      SplitPath(path, dir, file);

      if (dir=="")
        {
          dir = ".";
        }
      if (file == "")
        {
          UDie("Missing .ulam file after last slash");
        }

      std::cout << "SOURCE DIRECTORY: " << dir << " TARGET FILE: " << file << std::endl;

      if (m_srcFileManager) IDie("SetDirectory");

      m_srcFileManager = new FileManagerStdio(dir);
      if(!m_srcFileManager) {
        UDie("Couldn't init source file manager - bad directory?");
      }

#ifdef ULAM_SHARE_DIR  /* Search shared files if configured for it */
#define YY(s) XX(s)    /* expand */
#define XX(s) #s       /* stringify */
      m_srcFileManager->addReadDir(YY(ULAM_SHARE_DIR) "/ulam");
#undef XX
#undef YY
#endif /* ULAM_SHARE_DIR */

      m_filename = file;

      std::string outdir = dir;
      if (outpath)
        outdir = outpath;

      m_outFileManager = new FileManagerStdio(outdir);
      if(!m_outFileManager) {
        UDie("Couldn't init output file manager - bad directory?");
      }

    }

    /** return exit status:
        0 for good enough to continue,
        non-zero for any fatal errors
    */
    int RunCompilation()
    {
      /*
      m_stdout->write("BEGINNING COMPILATION\n");
      m_stderr->write("Reading from: m_sourceFile\n");
      m_stderr->write("Writing to:   ?? locations inside m_fileManager\n");
      m_stderr->write("Errors to:    m_stderr\n");
      m_stderr->write("Returning:    0 iff compilation should continue on to C-level\n");
      */

      Compiler C;
      int status = C.compileProgram(m_srcFileManager, m_filename, m_outFileManager, m_stderr);
      if (status == 0) {
        m_mangledTarget = C.getMangledTarget();
	m_hasTestMethod = C.hasTheTestMethod();
	m_isAQuark = C.targetIsAQuark();
      }
      return status;
    }

  private:
    FileManagerStdio * m_srcFileManager;
    FileManagerStdio * m_outFileManager;
    File * m_sourceFile;
    File * m_stdout;
    File * m_stderr;
    std::string m_filename;
    std::string m_mangledTarget;
    bool m_hasTestMethod;
    bool m_isAQuark;
  };
} /* namespace MFM */

int main(int argc, char ** argv)
{
  try {
    MFM::DriverState ds;

    progname = argv[0];
    --argc; ++argv;
    if (argc < 1)
      ds.UDie("Need at least one argument");
    if (argc > 2)
      ds.UDie("Need at most two arguments");
    if (!strcmp(argv[0],"-h"))
      {
        doHelp();
      }
    if (!strcmp(argv[0],"-V"))
      {
        fprintf(stdout,"culam-%d.%d.%d %08x%06x\n",
                1,0,0,BUILD_DATE,BUILD_TIME);
        exit(0);
      }
    ds.SetTarget(argv[0], argc > 1 ? argv[1] : 0);

    int result = ds.RunCompilation();
    if (result == 0)
      std::cerr << ds.GetMangledTarget() << " " << ds.HasTestMethod() << std::endl;
    return result;
  }
  catch (int status) {
    return status;
  }
} //main

