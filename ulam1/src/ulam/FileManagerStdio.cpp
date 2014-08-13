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

  File * FileManagerStdio::open(std::string path, enum Mode mode)
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
	stdio = new FileStdio(fp, mode);
	if(mode == APPEND)
	  {
	    fseek(fp,0,SEEK_END); //go to end of file
	  }
      }

    return stdio;
  }
      

}
