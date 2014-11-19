#include <stdio.h>
#include "NodeBlockClassEmpty.h"
#include "CompilerState.h"

namespace MFM {

  NodeBlockClassEmpty::NodeBlockClassEmpty(NodeBlock * prevBlockNode, CompilerState & state): NodeBlockClass(prevBlockNode, state, NULL)
  {}

  NodeBlockClassEmpty::~NodeBlockClassEmpty()
  {}


  void NodeBlockClassEmpty::print(File * fp)
  {
    printNodeLocation(fp);
    fp->write("NodeBlockClassEmpty<NOTYPE>\n");
    fp->write("-----------------NodeBlockClassEmpty\n");
  }


  void NodeBlockClassEmpty::printPostfix(File * fp)
  {
    fp->write(m_state.getUlamTypeByIndex(getNodeType())->getUlamTypeUPrefix().c_str());  //e.g. Ue_Foo
    fp->write(getName());  //unmangled

    fp->write(" { /* empty class block */ }");
  }


  const char * NodeBlockClassEmpty::getName()
  {
    //return "{}";
    return m_state.getUlamTypeByIndex(getNodeType())->getUlamKeyTypeSignature().getUlamKeyTypeSignatureName(&m_state).c_str(); 
  }


  const std::string NodeBlockClassEmpty::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UTI NodeBlockClassEmpty::checkAndLabelType()
  {
    setNodeType(Void);
    return getNodeType();
  }


  EvalStatus NodeBlockClassEmpty::eval()
  {
    return NORMAL;
  }

  u32 NodeBlockClassEmpty::countNativeFuncDecls()
  {
    return 0;
  }


} //end MFM
