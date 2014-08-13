#include <errno.h>
#include <cstring>
#include <iostream>
#include "FileManagerString.h"
#include "FileString.h"

namespace MFM {

  FileManagerString::FileManagerString(std::string directoryPath): m_dirPath(directoryPath)
  {}


  FileManagerString::~FileManagerString()
  {
    m_pathToContents.clear();
  }


  File * FileManagerString::open(std::string path, enum Mode mode)
  {
    std::map<std::string,std::string>::iterator sit;
    FileString * fs = NULL;
    std::string fullpath;
    s32 status = 0;

    if(path[0] == '/')
      {
	fullpath = path;
      }
    else
      {
	// concatenate to path to m_dirPath 
	fullpath = m_dirPath + "/" + path;
      }

    sit = m_pathToContents.find(fullpath);

    if(sit == m_pathToContents.end())  //not found
      {
	switch(mode) 
	  {
	  case READ:
	  case EXTEND:
	    {
	      // must exist, error
	      status = 1;
	      break;
	    }
	  case WRITE:
	  case APPEND:
	    {
	      // create new content string if does not exist
	      m_pathToContents[fullpath] = "";
	      break;
	    }
	  default:
	    break;
	  };
      }
    else  //entry found in map
      {
	// clear content string if does exist
	if(mode == WRITE)
	  {
	    sit->second.clear(); 
	  }
      }

    if(!status)
      {
	fs = new FileString(m_pathToContents[fullpath], mode);
      }
    else
      {
	errno = ENOENT;  // no such file or directory
      }
    
    return fs;
  }
      

  bool FileManagerString::add(std::string path, std::string data)
  {
    File * fs = open(path, WRITE);
    if(! fs) return false;

    bool rtn = true;
    s32 ok = fs->write(data.c_str());
    if(ok < 0)
      {
	//std::cerr << "FMS: add() unable to write data, error [" << errno << "] "<< strerror(errno) << std::endl;
	rtn = false;
      }
    
    delete fs;
    return rtn;
  }


  bool FileManagerString::get(std::string path, std::string& data)
  {
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
    
    data.assign(m_pathToContents[fullpath]);
    return true;
  }

}
