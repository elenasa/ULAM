#include <stdio.h>
#include "NodeFuncDecl.h"
#include "SymbolFunctionName.h"
#include "CompilerState.h"

namespace MFM {

  NodeFuncDecl::NodeFuncDecl(SymbolFunction * symfunc, CompilerState & state) : Node(state), m_funcSymbolPtr(symfunc)
  {
    assert(symfunc);
    m_fid = symfunc->getId();
    m_ordernum = symfunc->getOrderNumber();
  }

  NodeFuncDecl::NodeFuncDecl(const NodeFuncDecl& ref) : Node(ref), m_fid(ref.m_fid), m_ordernum(ref.m_ordernum), m_funcSymbolPtr(NULL) {}

  NodeFuncDecl::~NodeFuncDecl() {}

  Node * NodeFuncDecl::instantiate()
  {
    return new NodeFuncDecl(*this);
  }

  void NodeFuncDecl::print(File * fp) { }

  void NodeFuncDecl::printPostfix(File * fp) { }

  void NodeFuncDecl::noteTypeAndName(UTI cuti, s32 totalsize, u32& accumsize) { }

  void NodeFuncDecl::genTypeAndNameEntryAsComment(File * fp, s32 totalsize, u32& accumsize) { }

  UTI NodeFuncDecl::checkAndLabelType()
  {
    if(!m_funcSymbolPtr)
      {
	bool hazykin = false;
	Symbol * fnsymptr = NULL;
	if(m_state.isFuncIdInClassScope(m_fid, fnsymptr, hazykin))
	  {
	    assert(fnsymptr->isFunction()); //t41084, t41332
	    m_funcSymbolPtr = ((SymbolFunctionName *) fnsymptr)->getFunctionSymbolByOrderNumber(m_ordernum);
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Function symbol '";
	    msg << m_state.m_pool.getDataAsString(m_fid).c_str();
	    msg << "' cannot be found";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    setNodeType(Nav);
	    return Nav;
	  }
      }
    assert(m_funcSymbolPtr);
    UTI nodeType = m_funcSymbolPtr->getUlamTypeIdx();
    setNodeType(nodeType);
    return nodeType;
  }

  const char * NodeFuncDecl::getName()
  {
    return "()";
  }

  const std::string NodeFuncDecl::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  TBOOL NodeFuncDecl::packBitsInOrderOfDeclaration(u32& offset)
  {
    return TBOOL_TRUE; //bypass
  }

  void NodeFuncDecl::calcMaxIndexOfVirtualFunctionInOrderOfDeclaration(SymbolClass* csym, s32& maxidx)
  {
    assert(m_funcSymbolPtr);
    m_funcSymbolPtr->calcMaxIndexOfVirtualFunction(csym, maxidx); //entire reason for existing!
  }

  EvalStatus NodeFuncDecl::eval()
  {
    return NORMAL;
  }

  bool NodeFuncDecl::buildDefaultValue(u32 wlen, BV8K& dvref)
  {
    return true; //bypass
  }

  void NodeFuncDecl::genCodeConstantArrayInitialization(File * fp)
  {}

  void NodeFuncDecl::generateBuiltinConstantClassOrArrayInitializationFunction(File * fp, bool declOnly)
  {}

  void NodeFuncDecl::generateTestInstance(File * fp, bool runtest)
  { /* noop */ }

  void NodeFuncDecl::generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount)
  {}

  void NodeFuncDecl::addMemberDescriptionToInfoMap(UTI classType, ClassMemberMap& classmembers) { }

  void NodeFuncDecl::genCode(File * fp, UVPass& uvpass)
  {
    //not done here..
  } //genCode

  void NodeFuncDecl::generateFunctionInDeclarationOrder(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    assert(m_funcSymbolPtr);
    m_funcSymbolPtr->generateFunctionDeclaration(fp, declOnly, classtype); //2nd purpose
  }

} //end MFM
