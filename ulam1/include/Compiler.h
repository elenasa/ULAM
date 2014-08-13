/* -*- c++ -*- */

#ifndef COMPILER_H
#define COMPILER_H

#include <string>
#include "CompilerState.h"
#include "Node.h"
#include "FileManager.h"
#include "File.h"

namespace MFM{

  class Compiler
  {

  public:
    Compiler();
    ~Compiler();

    u32 start(FileManager * fm, std::string startstr, File * output, Node *& rtnNode);
    u32 labelParseTree(Node * root, File * output);
    u32 evalParseTree(Node * root, File * output);
    void printPostFix(Node * root, File * output);
    void printParseTree(Node * root, File * output);

  private:

    CompilerState m_state;

  };

} //MFM namespace

#endif //end COMPILER_H
