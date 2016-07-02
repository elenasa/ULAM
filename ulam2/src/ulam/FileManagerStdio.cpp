#include <iostream>
#include "FileManagerStdio.h"
#include "FileStdio.h"

namespace MFM {

  //ordered by enum Mode
  static const std::string ModeStr[4] = { "r", "w" , "a" , "r+" };

  FileManagerStdio::FileManagerStdio(std::string directoryPath): m_dirPath(directoryPath)
  {}

  FileManagerStdio::~FileManagerStdio()
  {}

  void FileManagerStdio::addReadDir(std::string readDir)
  {
    m_otherReadDirs.push_back(readDir);
  }

  File * FileManagerStdio::open(const std::string path, enum Mode mode, std::string & resultpath)
  {
    FILE * fp;
    FileStdio * stdio = NULL;
    std::string fullpath;

    if(path[0] == '/')
      {
	fullpath = path;
      }
    else
      {
	// concatenate to path to m_dirPath
	fullpath = m_dirPath + "/" + path;
      }

	fp = fopen(fullpath.c_str(), ModeStr[mode].c_str());

    if(fp)
      {
	//std::cerr << "FileManagerStdio::open creating new FileStdio, path <" << fullpath.c_str() << ">" << std::endl;
        resultpath = fullpath;
	stdio = new FileStdio(fp, mode);
	if(mode == APPEND)
	  {
	    fseek(fp,0,SEEK_END); //go to end of file
	  }
      }
    else if (mode == READ && path[0] != '/')
      {
        //Open for relative path READ failed.  If we have other dirs
        //to try, try them.  If they all fail, leave errno holding the
        //error from the last one.
        for (u32 i = 0; i < m_otherReadDirs.size(); ++i)
          {
            fullpath = m_otherReadDirs[i] + "/" + path;
            fp = fopen(fullpath.c_str(), ModeStr[mode].c_str());
            if(fp)
              {
                resultpath = fullpath;
                stdio = new FileStdio(fp, mode);
                break;
              }
          }
      }

    return stdio;
  }


}
