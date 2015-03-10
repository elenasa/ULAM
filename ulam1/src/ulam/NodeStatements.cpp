#include <stdio.h>
#include "NodeStatements.h"
#include "CompilerState.h"

namespace MFM {

  NodeStatements::NodeStatements(Node * s, CompilerState & state) : Node(state), m_node(s), m_nodeNext(NULL) {}

  NodeStatements::NodeStatements(const NodeStatements& ref) : Node(ref)
  {
    if(ref.m_node)
      m_node = ref.m_node->instantiate();
    else
      m_node = NULL;

    if(ref.m_nodeNext)
      m_nodeNext = (NodeStatements *) ref.m_nodeNext->instantiate();
    else
      m_nodeNext = NULL;
  }

  NodeStatements::~NodeStatements()
  {
    delete m_nodeNext;
    m_nodeNext = NULL;

    delete m_node;
    m_node = NULL;
  }

  Node * NodeStatements::instantiate()
  {
    return new NodeStatements(*this);
  }

  void NodeStatements::updateLineage(NNO pno)
  {
    setYourParentNo(pno);
    if(m_node)
      m_node->updateLineage(getNodeNo());
    if(m_nodeNext)
      m_nodeNext->updateLineage(getNodeNo());
  }//updateLineage

  bool NodeStatements::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_node && m_node->findNodeNo(n, foundNode))
      return true;
    if(m_nodeNext && m_nodeNext->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeStatements::print(File * fp)
  {
    printNodeLocation(fp);  //has same location as it's node
    UTI myut = getNodeType();
    char id[255];
    if(myut == Nav)
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n", prettyNodeName().c_str(), m_state.getUlamTypeNameByIndex(myut).c_str());
    fp->write(id);

    if(m_node)
      m_node->print(fp);
    else
      fp->write(" <EMPTYSTMT>\n");

    if(m_nodeNext)
      m_nodeNext->print(fp);
    else
      fp->write(" <NONEXTSTMT>\n");
    sprintf(id,"-----------------%s\n", prettyNodeName().c_str());
    fp->write(id);
  } //print

  void NodeStatements::printPostfix(File * fp)
  {
    assert(m_node);    //e.g. bad decl

    if(m_node)
      m_node->printPostfix(fp);
    else
      fp->write(" <EMPTYSTMT>");

    if(m_nodeNext)
      m_nodeNext->printPostfix(fp);
  } //printPostfix

  const char * NodeStatements::getName()
  {
    return "Stmts";  //?
  }

  const std::string NodeStatements::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  UTI NodeStatements::checkAndLabelType()
  {
    assert(m_node);

    //unlike statements, blocks don't have an m_node
    m_node->checkAndLabelType();       //side-effect

    if(m_nodeNext)
      m_nodeNext->checkAndLabelType(); //side-effect

    //statements don't have types
    setNodeType(Void);
    return getNodeType();
  } //checkAndLabelType

  void NodeStatements::countNavNodes(u32& cnt)
  {
    if(m_node)
      m_node->countNavNodes(cnt);
    if(m_nodeNext)
      m_nodeNext->countNavNodes(cnt);
  } //countNavNodes

  EvalStatus NodeStatements::eval()
  {
    assert(m_node);

    evalNodeProlog(0);
    makeRoomForNodeType(m_node->getNodeType());
    EvalStatus evs = m_node->eval();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    //not the last one, so thrown out results and continue
    if(m_nodeNext)
      {
	evalNodeEpilog();  //Tue Aug 26 16:18:43 2014
	evs = m_nodeNext->eval();
	if(evs != NORMAL)
	  {
	    ////evalNodeEpilog();
	    //evalNodeEpilog(); //Tue Aug 26 16:18:56 2014
	    return evs;
	  }
      }
    else
      {
	//end, so copy to -1
	//	UlamValue rtnPtr(getNodeType(), 1, true, EVALRETURN);  //positive to current frame pointer
	//assignReturnValueToStack(rtnPtr);
	evalNodeEpilog();
      }
    return NORMAL;
  } //eval

  void NodeStatements::setNextNode(NodeStatements * s)
  {
    m_nodeNext = s;
  }

  void NodeStatements::packBitsInOrderOfDeclaration(u32& offset)
  {
    m_node->packBitsInOrderOfDeclaration(offset); //updates offset

    if(m_nodeNext)
      m_nodeNext->packBitsInOrderOfDeclaration(offset);
  } //packBitsInOrderOfDeclaration

  void NodeStatements::genCode(File * fp, UlamValue& uvpass)
  {
    Locator nodeloc = getNodeLocation();
    m_state.outputTextAsComment(fp, nodeloc);
    m_state.m_locOfNextLineText = nodeloc;  //during gen code here

#ifdef TMPVARBRACES
    m_state.indent(fp);
    fp->write("{\n");    //open for tmpvar arg's
    m_state.m_currentIndentLevel++;
#endif

    m_node->genCode(fp, uvpass);

#ifdef TMPVARBRACES
    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("}\n");  //close for tmpVar
#endif

    if(m_nodeNext)
      m_nodeNext->genCode(fp, uvpass);
  } //genCode

  void NodeStatements::genCodeToStoreInto(File * fp, UlamValue& uvpass)
  {
    m_node->genCodeToStoreInto(fp, uvpass);

    if(m_nodeNext)
      m_nodeNext->genCodeToStoreInto(fp, uvpass);
  } //genCodeToStoreInto

} //end MFM
