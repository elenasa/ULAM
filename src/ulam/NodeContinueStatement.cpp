#include <stdio.h>
#include "NodeContinueStatement.h"
#include "CompilerState.h"

namespace MFM {

  NodeContinueStatement::NodeContinueStatement(s32 gotolabelnum, CompilerState & state) : Node(state), m_gotolabelnum(gotolabelnum) {}

  NodeContinueStatement::NodeContinueStatement(const NodeContinueStatement& ref) : Node(ref), m_gotolabelnum(ref.m_gotolabelnum) {}

  NodeContinueStatement::~NodeContinueStatement() {}

  Node * NodeContinueStatement::instantiate()
  {
    return new NodeContinueStatement(*this);
  }

  void NodeContinueStatement::print(File * fp)
  {
    printNodeLocation(fp);  //has same location as its node
    UTI myut = getNodeType();
    char id[255];
    if((myut == Nav) || (myut == Nouti))
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n", prettyNodeName().c_str(), m_state.getUlamTypeNameByIndex(myut).c_str());
    fp->write(id);
  }

  void NodeContinueStatement::printPostfix(File * fp)
  {
    fp->write(" ");
    fp->write(getName());
  }

  const char * NodeContinueStatement::getName()
  {
    return "goto";
  }

  const std::string NodeContinueStatement::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  UTI NodeContinueStatement::checkAndLabelType(Node * thisparentnode)
  {
    UTI nodeType = Void;
    setNodeType(nodeType);
    return nodeType;
  }

  EvalStatus NodeContinueStatement::eval()
  {
    return CONTINUE;
  }

  void NodeContinueStatement::genCode(File * fp, UVPass& uvpass)
  {
    m_state.indentUlamCode(fp);
    fp->write(getName());
    fp->write(" ");
    fp->write(m_state.getLabelNumAsString(m_gotolabelnum).c_str());
    fp->write("; //continue nearest loop"); GCNL;
  } //genCode

} //end MFM
