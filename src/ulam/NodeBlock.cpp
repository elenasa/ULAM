#include <stdio.h>
#include "NodeBlock.h"
#include "CompilerState.h"

namespace MFM {

  NodeBlock::NodeBlock(NodeBlock * prevBlockNode, CompilerState & state, NodeStatements * s) : NodeStatements(NULL, state), m_ST(state), m_prevBlockNode(prevBlockNode), m_nodeEndingStmt(this)
  {
    if(s)
      {
	setNextNode(s);
	setLastStatementPtr(s);
      }
  }

  NodeBlock::NodeBlock(const NodeBlock& ref) : NodeStatements(ref), m_ST(ref.m_ST) /* deep copy */, m_prevBlockNode(NULL), m_nodeEndingStmt(ref.m_nodeEndingStmt) {}

  NodeBlock::~NodeBlock()
  {
    //do not delete prevBlockNode
  }

  Node * NodeBlock::instantiate()
  {
    return new NodeBlock(*this);
  }

  void NodeBlock::updateLineage(NNO pno)
  {
    if(getPreviousBlockPointer() == NULL)
      setPreviousBlockPointer(m_state.getCurrentBlock()); //t41283, not for filescope locals
    else
      assert(getPreviousBlockPointer() == m_state.getCurrentBlock());

    m_state.pushCurrentBlock(this);

    setYourParentNo(pno);
    //has no m_node
    if(m_nodeNext)
      m_nodeNext->updateLineage(getNodeNo());

    m_state.popClassContext(); //restores previousBlockNode
  } //updateLineage

  bool NodeBlock::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_nodeNext && m_nodeNext->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeBlock::checkAbstractInstanceErrors()
  {
    if(m_nodeNext)
      m_nodeNext->checkAbstractInstanceErrors(); //work done by NodeVarDecl
  }

  void NodeBlock::print(File * fp)
  {
    printNodeLocation(fp);
    UTI myut = getNodeType();
    char id[255];
    if((myut == Nav) || (myut == Nouti))
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
    else if(myut == Hzy)
      sprintf(id,"%s<HAZYTYPE>\n", prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n", prettyNodeName().c_str(), m_state.getUlamTypeNameByIndex(myut).c_str());
    fp->write(id);

    if(m_nodeNext)
      m_nodeNext->print(fp);

    sprintf(id,"-----------------%s\n", prettyNodeName().c_str());
    fp->write(id);
  } //print

  void NodeBlock::printPostfix(File * fp)
  {
    fp->write(" {");
    // has no m_node!
    if(m_nodeNext)
      m_nodeNext->printPostfix(fp);
    else
      fp->write(" <EMPTYSTMT>"); //not an error

    fp->write(" }");
  } //printPostifx

  const char * NodeBlock::getName()
  {
    return "{}";
  }

  const std::string NodeBlock::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  void NodeBlock::setLastStatementPtr(NodeStatements * laststmt)
  {
    m_nodeEndingStmt = laststmt;
  }

  NodeStatements * NodeBlock::getLastStatementPtr()
  {
    assert(m_nodeEndingStmt != NULL);
    return m_nodeEndingStmt;
  }

  void NodeBlock::appendNextNode(Node * node)
  {
    assert(node);
    NodeStatements * nextNode = new NodeStatements(node, m_state);
    assert(nextNode);
    assert(m_nodeEndingStmt);
    m_nodeEndingStmt->setNextNode(nextNode);
    m_nodeEndingStmt = nextNode;
  } //appendNextNode

  UTI NodeBlock::checkAndLabelType()
  {
    assert(m_nodeNext);
    //especially important for template instances (prev ptr nullified on instantiation)
    //and extremely important to skip for filescope locals (only one per file) e.g. t41283;
    if(getPreviousBlockPointer() == NULL)
      setPreviousBlockPointer(m_state.getCurrentBlock());
    else
      assert(getPreviousBlockPointer() == m_state.getCurrentBlock());

    m_state.pushCurrentBlock(this);

    m_nodeNext->checkAndLabelType();

    m_state.popClassContext(); //restores m_prevBlockNode

    setNodeType(Void); //blocks don't have types
    return getNodeType();
  } //checkAndLabelType

  void NodeBlock::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    Node::countNavHzyNoutiNodes(ncnt, hcnt, nocnt); //missing
    if(m_nodeNext)
      m_nodeNext->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
  }

  EvalStatus NodeBlock::eval()
  {
    if(!m_nodeNext) return evalErrorReturn();

    // block stack needed for symbol lookup during eval of virtual func call on as-conditional auto
    m_state.pushCurrentBlock(this);
    EvalStatus evs = m_nodeNext->eval();
    m_state.popClassContext(); //restore
    return evs;
  } //eval

  void NodeBlock::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    // can't use ST because it includes the hidden and rest of args; unordered.
    u32 max2 = depth;
    if(m_nodeNext)
      m_nodeNext->calcMaxDepth(max2, maxdepth, base);

    maxdepth = max2 > maxdepth ? max2 : maxdepth; //no change to depth here
  } //calcMaxDepth

  bool NodeBlock::isIdInScope(u32 id, Symbol * & symptrref)
  {
    return m_ST.isInTable(id, symptrref);
  }

  void NodeBlock::addIdToScope(u32 id, Symbol * symptr)
  {
    assert(symptr->getBlockNoOfST() == getNodeNo()); //set by Symbol constr, based on m_currentBlock
    m_ST.addToTable(id, symptr);
  }

  void NodeBlock::replaceIdInScope(u32 oldid, u32 newid, Symbol * symptr)
  {
    assert(symptr->getBlockNoOfST() == getNodeNo());
    m_ST.replaceInTable(oldid, newid, symptr);
  }

  void NodeBlock::replaceIdInScope(Symbol * oldsym, Symbol * newsym)
  {
    assert(oldsym->getBlockNoOfST() == getNodeNo());
    newsym->setBlockNoOfST(getNodeNo()); //update ST block no in new symbol
    m_ST.replaceInTable(oldsym, newsym);
  }

  bool NodeBlock::removeIdFromScope(u32 id, Symbol *& rtnsymptr)
  {
    return m_ST.removeFromTable(id, rtnsymptr);
  }

  NodeBlock * NodeBlock::getPreviousBlockPointer()
  {
    return m_prevBlockNode;
  }

  void NodeBlock::setPreviousBlockPointer(NodeBlock * b)
  {
    assert(b != this); //invariant (t41283)
    m_prevBlockNode = b;
  }

  bool NodeBlock::isAClassBlock()
  {
    return false;
  }

  bool NodeBlock::isASwitchBlock()
  {
    return false;
  }

  u32 NodeBlock::getNumberOfSymbolsInTable()
  {
    return m_ST.getTableSize();
  }

  u32 NodeBlock::getSizeOfSymbolsInTable()
  {
    return m_ST.getTotalSymbolSize();
  }

  s32 NodeBlock::getBitSizesOfVariableSymbolsInTable()
  {
    if(m_ST.getTableSize() == 0)
      return EMPTYSYMBOLTABLE; //should allow no variable data members

    return m_ST.getTotalVariableSymbolsBitSize();
  }

  s32 NodeBlock::getMaxBitSizeOfVariableSymbolsInTable()
  {
    if(m_ST.getTableSize() == 0)
      return EMPTYSYMBOLTABLE; //should allow no variable data members

    return m_ST.getMaxVariableSymbolsBitSize();
  }

  u32 NodeBlock::findTypedefNameIdByType(UTI uti)
  {
    return m_ST.findTypedefSymbolNameIdByTypeInTable(uti); //0 == not found
  }

  SymbolTable * NodeBlock::getSymbolTablePtr()
  {
    return &m_ST;
  }

  void NodeBlock::genModelParameterImmediateDefinitions(File * fp)
  {
    m_ST.genModelParameterImmediateDefinitionsForTableOfVariableDataMembers(fp);
  }

  void NodeBlock::addMemberDescriptionsToInfoMap(ClassMemberMap& classmembers)
  {
    //has no m_node
    if(m_nodeNext)
      m_nodeNext->addMemberDescriptionToInfoMap(this->getNodeType(), classmembers);
  }

  void NodeBlock::genCode(File * fp, UVPass& uvpass)
  {
    m_state.indentUlamCode(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;
    m_nodeNext->genCode(fp, uvpass);
    m_state.m_currentIndentLevel--;

    m_state.indentUlamCode(fp);
    fp->write("}\n");
  } //genCode


} //end MFM
