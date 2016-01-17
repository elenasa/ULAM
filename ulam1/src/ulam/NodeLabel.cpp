#include <stdio.h>
#include "NodeLabel.h"
#include "CompilerState.h"

namespace MFM {

  NodeLabel::NodeLabel(s32 labelnum, CompilerState & state) : Node(state), m_labelnum(labelnum) {}

  NodeLabel::NodeLabel(const NodeLabel& ref) : Node(ref), m_labelnum(ref.m_labelnum) {}

  NodeLabel::~NodeLabel() {}

  Node * NodeLabel::instantiate()
  {
    return new NodeLabel(*this);
  }

  void NodeLabel::print(File * fp)
  {
    printNodeLocation(fp);  //has same location as it's node
    UTI myut = getNodeType();
    char id[255];
    if((myut == Nav) || (myut == Nouti))
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
    else if(myut == Hzy)
      sprintf(id,"%s<HAZYTYPE>\n", prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n", prettyNodeName().c_str(), m_state.getUlamTypeNameByIndex(myut).c_str());
    fp->write(id);
  }

  void NodeLabel::printPostfix(File * fp)
  {
    fp->write(" _");
    fp->write_decimal(m_labelnum);
    fp->write(getName());
  }

  UTI NodeLabel::checkAndLabelType()
  {
    UTI nodeType = Void;
    setNodeType(nodeType);
    return nodeType;
  }

  const char * NodeLabel::getName()
  {
    return ":";
  }

  const std::string NodeLabel::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  EvalStatus NodeLabel::eval()
  {
    return NORMAL;
  }

  void NodeLabel::genCode(File * fp, UlamValue& uvpass)
  {
    //no indent for label
    fp->write(m_state.getLabelNumAsString(m_labelnum).c_str());
    fp->write(":\n");

    m_state.indent(fp);
    fp->write("__attribute__((__unused__));\n");
  } //genCode

} //end MFM
