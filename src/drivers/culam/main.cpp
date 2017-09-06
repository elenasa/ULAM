// #include "ulam.h"
#include <iostream>
#include <strings.h>  /* for rindex */
#include "SourceStream.h"
#include "FileManagerStdio.h"
#include "FileStdio.h"
#include "FileString.h"
#include "Compiler.h"
#include "UlamUtil.h"

static const char * progname;

static void doVersion()
{
  fprintf(stdout,"culam-%d.%d.%d %08x%06x\n",
          1,0,0,BUILD_DATE,BUILD_TIME);
  exit(0);
}

static void doHelp()
{
  fprintf(stderr,
          "Usage: %s [SWITCHES] CLASS CLASS...\n"
          "  Each CLASS is the name of an ulam element \n"
          "  (or quark or union) to be compiled.\n"
          "\n"
          "Switches:\n"
          " -h      Print this help and exit\n"
          " -V      Print version info and exit\n"
          " -o DIR  Set output directory to DIR\n"
          " -i DIR  Add DIR to input directory search list\n"
          " -g      Debug\n"
          "\n",
          progname);
  exit(0);
}

namespace MFM
{
  class DriverState {
  public:
    DriverState()
      : m_srcFileManager(new FileManagerStdio("."))
      , m_outFileManager(0)
      , m_sourceFile(0)
      , m_stdout(new FileStdio(stdout, MFM::WRITE))
      , m_stderr(new FileStdio(stderr, MFM::WRITE))
      , m_doDebug(false)
    { }

    ~DriverState() {
      delete m_srcFileManager; m_srcFileManager = 0;
      delete m_outFileManager; m_outFileManager = 0;
      delete m_sourceFile;     m_sourceFile = 0;
      delete m_stdout;         m_stdout = 0;
      delete m_stderr;         m_stderr = 0;
    }

    /** Usage error abort */
    void UDie(const char * msg, const char * extra = 0)
    {
      std::cerr << "Error: " << msg;
      if (extra)
        std::cerr << "'" << extra << "'";
      std::cerr << std::endl;
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

    void doDebug()
    {
      std::cerr << "DEBUG: #LINE enabled" << std::endl;
      m_doDebug = true;
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

    void SetOutputDir(const char * dirOrNull)
    {
      std::string dir;

      if (!dirOrNull) dirOrNull = "";
      dir = dirOrNull;

      if (dir=="") dir = ".";

      std::cout << "OUTPUT DIRECTORY: " << dir << std::endl;

      if (m_outFileManager) IDie("SetDirectory");

      m_outFileManager = new FileManagerStdio(dir);
      if(!m_outFileManager) {
        UDie("Couldn't init output file manager for: ",dir.c_str());
      }
    }

    void AddSharedInputDir()
    {

#ifdef ULAM_SHARE_DIR  /* Search shared files if configured for it */
#define YY(s) XX(s)    /* expand */
#define XX(s) #s       /* stringify */
      m_srcFileManager->addReadDir(YY(ULAM_SHARE_DIR) "/ulam/stdlib");
#undef XX
#undef YY
#endif /* ULAM_SHARE_DIR */

    }

    bool EndsWith(std::string const & value, std::string const & ending)
    {
      if (ending.size() > value.size()) return false;
      return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
    }

    void AddClassName(const char * path)
    {
      std::string classname = path;
      const char * ext = ".ulam";
      if (EndsWith(classname,ext))
        {
          classname = classname.substr(0,classname.length()-strlen(ext));
        }
      if (classname.length() == 0)
        UDie("Illegal class name (too short):",path);

      for (u32 i = 0; i < classname.length(); ++i)
        {
          char c = classname[i];
          if (!isalnum(c))
            {
              std::string err = "Illegal class name (illegal char '";
              err += c;
              err += "'):";
              UDie(err.c_str(), path);
            }
        }

      if (classname[0] < 'A' || classname[0] > 'Z')
        UDie("Illegal class name (not initial capital):",path);

      m_classfiles.push_back(classname + ext);
    }

    void AddInputDir(char * path)
    {
      m_srcFileManager->addReadDir(path);
    }

    /** return exit status:
        0 for good enough to continue,
        non-zero for any fatal errors
    */
    int RunCompilation(Compiler & C)
    {
      /*
      m_stdout->write("BEGINNING COMPILATION\n");
      m_stderr->write("Reading from: m_sourceFile\n");
      m_stderr->write("Writing to:   ?? locations inside m_fileManager\n");
      m_stderr->write("Errors to:    m_stderr\n");
      m_stderr->write("Returning:    0 iff compilation should continue on to C-level\n");
      */
      C.setLinesForDebug(m_doDebug);

      int status = C.compileFiles(m_srcFileManager, m_classfiles, m_outFileManager, m_stderr);
      if (status == 0) {
        m_targetMap = C.getMangledTargetsMap();
        m_memberMap = C.getMangledClassMembersMap();
      }
      return status;
    }

    TargetMap::const_iterator TargetMapBegin() const
    {
      return m_targetMap.begin();
    }

    TargetMap::const_iterator TargetMapEnd() const
    {
      return m_targetMap.end();
    }

    ClassMemberMap::const_iterator ClassMemberMapBegin() const
    {
      return m_memberMap.begin();
    }

    ClassMemberMap::const_iterator ClassMemberMapEnd() const
    {
      return m_memberMap.end();
    }

    ClassMemberMap & GetClassMemberMap()
    {
      return m_memberMap;
    }

  private:
    FileManagerStdio * m_srcFileManager;
    FileManagerStdio * m_outFileManager;
    File * m_sourceFile;
    File * m_stdout;
    File * m_stderr;
    std::vector<std::string> m_classfiles;
    TargetMap m_targetMap;
    ClassMemberMap m_memberMap;
    bool m_doDebug;
  };
} /* namespace MFM */

int main(int argc, char ** argv)
{
  try {
    MFM::DriverState ds;

    progname = argv[0];
    --argc; ++argv;

    const char * outdir = 0;
    bool noMoreSwitches = false;

    ds.AddClassName("UrSelf.ulam"); //insure VTable order

    for (; argc > 0; --argc, ++argv)
      {
        const char * arg = argv[0];
        if (arg[0] == '-' && noMoreSwitches)
          {
            ds.UDie("Too late for switch: ", arg);
          }
        if (!strcmp(arg,"-h")) doHelp();
        if (!strcmp(arg,"-V")) doVersion();
        if (!strcmp(arg,"-o"))
          {
            if (outdir) ds.UDie("-o can appear at most once");
            if (argc <= 0) ds.UDie("Must have argument after -o");
            --argc; ++argv;
            outdir = argv[0];
            continue;
          }
        if (!strcmp(arg,"-i"))
          {
            if (argc <= 0) ds.UDie("Must have argument after -i");
            --argc; ++argv;
            ds.AddInputDir(argv[0]);
            continue;
          }

        if (!strcmp(arg,"-g"))
	  {
	    ds.doDebug();
       	    continue;
	  }

        if (arg[0] == '-') ds.UDie("Unrecognized switch: ", arg);

        noMoreSwitches = true;  // Whatever it is, it's not a switch

        ds.AddClassName(arg);
      }
    if (outdir)
      ds.SetOutputDir(outdir);
    else
      ds.SetOutputDir(".");

    ds.AddSharedInputDir();

    MFM::Compiler c;
    int result = ds.RunCompilation(c);
    if (result == 0)
      {
	//process UlamElementInfo here: (including quarks)
        for(MFM::TargetMap::const_iterator i = ds.TargetMapBegin(); i != ds.TargetMapEnd(); ++i)
          {
            std::cerr
              << "ULAM INFO: "  // Magic cookie text! ulam.tmpl recognizes it! emacs *compilation* doesn't!
	      << "TARGET "
              << MFM::HexEscape(c.getFullPathLocationAsString(i->second.m_loc))
              << " " << i->second.m_className
              << " " << i->first
              << " " << i->second.m_bitsize
              << " " << (i->second.m_hasTest?"test":"notest")
	      << " " << (i->second.m_classType == MFM::UC_QUARK ? "quark": (i->second.m_classType == MFM::UC_ELEMENT ? "element" : (i->second.m_classType == MFM::UC_TRANSIENT ? "transient" : (i->second.m_classType == MFM::UC_LOCALSFILESCOPE ? "localsfilescope": "fudge"))))
	      << " " << MFM::HexEscape(i->second.m_classSignature)
	      << " " << MFM::HexEscape(i->second.m_baseClassSignature)
	      << " " << MFM::HexEscape(i->second.m_structuredComment)
              << std::endl;
          }

        for(MFM::ClassMemberMap::const_iterator i = ds.ClassMemberMapBegin(); i != ds.ClassMemberMapEnd(); ++i)
          {
	    const MFM::ClassMemberDesc * cmd = i->second.getClassMemberDesc();
	    std::cerr
	      << "ULAM INFO: "  // Magic cookie text! ulam.tmpl recognizes it! emacs *compilation* doesn't!
	      << cmd->getMemberKind() << " "
	      << MFM::HexEscape(c.getFullPathLocationAsString(cmd->m_loc))
	      << " " << cmd->m_mangledClassName
	      << " " << cmd->m_mangledType
	      << " " << MFM::HexEscape(cmd->m_memberName)
	      << " " << cmd->m_mangledMemberName;

	    if(cmd->hasValue())
	      std::cerr << " " << cmd->getValueAsString();

	    std::cerr << " " << MFM::HexEscape(cmd->m_structuredComment)
		      << std::endl;
	  }
      }
    return result;
  }
  catch (int status) {
    return status;
  }
} //main
