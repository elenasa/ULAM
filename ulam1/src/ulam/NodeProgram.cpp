#include <stdio.h>
#include <map>
#include "NodeProgram.h"
#include "CompilerState.h"

namespace MFM {

  NodeProgram::NodeProgram(u32 id, CompilerState & state) : Node(state), m_root(NULL), m_compileThisId(id) {}

  NodeProgram::~NodeProgram()
  {
    //m_root deletion handled by m_programDefST;
  }


  void NodeProgram::updateLineage(Node * p)
  {
    assert(0);
  }


  void NodeProgram::setRootNode(NodeBlockClass * root)
  {
    m_root = root;
  }


  void NodeProgram::print(File * fp)
  {
    assert(0);
  }


  void NodeProgram::printPostfix(File * fp)
  {
    assert(0);
  }


  const char * NodeProgram::getName()
  {
    return  m_state.m_pool.getDataAsString(m_compileThisId).c_str();
  }


  const std::string NodeProgram::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UTI NodeProgram::checkAndLabelType()
  {
    // done by Symbol Table for all classes with a Test method
    assert(0);
    return Nav;
  } //checkAndLabelType


  //performed across classes starting at NodeBlockClass
  void NodeProgram::countNavNodes(u32& cnt)
  {
    assert(0);
    return;
  }


  EvalStatus NodeProgram::eval()
  {
    // done by Symbol Table for all classes with a Test method
    assert(0);
    return ERROR;
  } //eval


  void NodeProgram::generateCode(FileManager * fm)
  {
    assert(0);
    // done by Symbol Table for all classes
  } //generateCode


} //end MFM
