#include <stdio.h>
#include "MapFunctionDesc.h"
#include "NodeFuncDecl.h"
#include "SymbolFunctionName.h"
#include "CompilerState.h"

namespace MFM {

  NodeFuncDecl::NodeFuncDecl(SymbolFunction * symfunc, CompilerState & state) : Node(state), m_funcSymbolPtr(symfunc), m_fid(getfunctionid()), m_ordernum(getfunctionordernum()) { }

  NodeFuncDecl::NodeFuncDecl(const NodeFuncDecl& ref) : Node(ref), m_funcSymbolPtr(NULL), m_fid(ref.m_fid), m_ordernum(ref.m_ordernum) {}

  NodeFuncDecl::~NodeFuncDecl() {}

  Node * NodeFuncDecl::instantiate()
  {
    return new NodeFuncDecl(*this);
  }

  u32 NodeFuncDecl::getfunctionid() const
  {
    assert(m_funcSymbolPtr);
    return m_funcSymbolPtr->getId();
  }

  u32 NodeFuncDecl::getfunctionordernum() const
  {
    assert(m_funcSymbolPtr);
    return m_funcSymbolPtr->getOrderNumber();
  }

  void NodeFuncDecl::print(File * fp) { }

  void NodeFuncDecl::printPostfix(File * fp) { }

  void NodeFuncDecl::noteTypeAndName(UTI cuti, s32 totalsize, u32& accumsize) { }

  void NodeFuncDecl::genTypeAndNameEntryAsComment(File * fp, s32 totalsize, u32& accumsize) { }

  UTI NodeFuncDecl::checkAndLabelType()
  {
    if(!m_funcSymbolPtr)
      {
	//template instance: find cloned function symbol ptr with function id and order number
	bool hazykin = false;
	Symbol * fnsymptr = NULL;
	if(m_state.isFuncIdInClassScope(m_fid, fnsymptr, hazykin))
	  {
	    assert(fnsymptr->isFunction()); //t41084, t41332
	    m_funcSymbolPtr = ((SymbolFunctionName *) fnsymptr)->getFunctionSymbolByOrderNumber(m_ordernum);
	  }
      }

    if(!m_funcSymbolPtr)
      {
	std::ostringstream msg;
	msg << "Function symbol '";
	msg << m_state.m_pool.getDataAsString(m_fid).c_str();
	msg << "' cannot be found";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	setNodeType(Nav);
	return Nav;
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

  void NodeFuncDecl::addMemberDescriptionToInfoMap(UTI classType, ClassMemberMap& classmembers)
  {
    assert(m_funcSymbolPtr);
    FunctionDesc * descptr = new FunctionDesc(m_funcSymbolPtr, classType, m_state);
    assert(descptr);

    //concat mangled class and parameter names to avoid duplicate keys into map
    std::ostringstream fullMangledName;
    fullMangledName << descptr->m_mangledClassName << "_" << descptr->m_mangledMemberName;
    classmembers.insert(std::pair<std::string, struct ClassMemberDesc *>(fullMangledName.str(), descptr));
  }

  void NodeFuncDecl::genCode(File * fp, UVPass& uvpass)
  {
    //not done here..see NodeBlockFunctionDefinition
  } //genCode

  void NodeFuncDecl::generateFunctionInDeclarationOrder(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    assert(m_funcSymbolPtr);
    m_funcSymbolPtr->generateFunctionDeclaration(fp, declOnly, classtype); //2nd purpose
  }

} //end MFM
