#include <stdio.h>
#include "NodeBreakStatement.h"
#include "CompilerState.h"

namespace MFM {

  NodeBreakStatement::NodeBreakStatement(CompilerState & state) : Node(state), m_gotolabelnum(0) {}

  NodeBreakStatement::NodeBreakStatement(s32 gotolabelnum, CompilerState & state) : Node(state), m_gotolabelnum(gotolabelnum) {}

  NodeBreakStatement::NodeBreakStatement(const NodeBreakStatement& ref) : Node(ref), m_gotolabelnum(ref.m_gotolabelnum) {}

  NodeBreakStatement::~NodeBreakStatement() {}

  Node * NodeBreakStatement::instantiate()
  {
    return new NodeBreakStatement(*this);
  }

  void NodeBreakStatement::print(File * fp)
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
  } //print

  void NodeBreakStatement::printPostfix(File * fp)
  {
    fp->write(" ");
    fp->write(getName());
  }

  const char * NodeBreakStatement::getName()
  {
    return "break";
  }

  const std::string NodeBreakStatement::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  UTI NodeBreakStatement::checkAndLabelType(Node * thisparentnode)
  {
    UTI nodeType = Void;
    setNodeType(nodeType);
    return nodeType;
  }

  EvalStatus NodeBreakStatement::eval()
  {
    if(m_gotolabelnum == 0)
      return BREAK;
    return NORMAL; //switch break (t41016)
  }

  void NodeBreakStatement::genCode(File * fp, UVPass& uvpass)
  {
    if(m_gotolabelnum == 0)
      {
	m_state.indentUlamCode(fp);
	fp->write("break; //out of nearest loop"); GCNL;
      }
    else
      {
	//goto for switch (t41017)
	m_state.indentUlamCode(fp);
	fp->write("goto ");
	fp->write(m_state.getLabelNumAsString(m_gotolabelnum).c_str());
	fp->write("; //break out of switch"); GCNL;
      }
  } //genCode

} //end MFM
