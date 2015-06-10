#include <stdlib.h>
#include "NodeParameterDef.h"
#include "NodeTerminal.h"
#include "CompilerState.h"


namespace MFM {

  NodeParameterDef::NodeParameterDef(SymbolParameterValue * symptr, NodeTypeDescriptor * nodetype, CompilerState & state) : NodeConstantDef(symptr, nodetype, state) {}

  NodeParameterDef::NodeParameterDef(const NodeParameterDef& ref) : NodeConstantDef(ref) {}

  NodeParameterDef::~NodeParameterDef() {}

  Node * NodeParameterDef::instantiate()
  {
    return new NodeParameterDef(*this);
  }

  void NodeParameterDef::printPostfix(File * fp)
  {
    NodeConstantDef::printPostfix(fp);
    fp->write("parameter");
  }

  const std::string NodeParameterDef::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  void NodeParameterDef::fixPendingArgumentNode()
  {
    assert(0);
  }

  void NodeParameterDef::genCode(File * fp, UlamValue& uvpass)
  {
    assert(m_constSymbol->isDataMember());

    UTI vuti = m_constSymbol->getUlamTypeIdx();
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);

    m_state.indent(fp);
    fp->write("mutable ");

    fp->write(vut->getImmediateStorageTypeAsString().c_str()); //for C++ local vars, ie non-data members
    fp->write(" ");
    fp->write(m_constSymbol->getMangledName().c_str());
    fp->write(";\n");
  } //genCode

  void NodeParameterDef::genCodeConstructorInitialization(File * fp)
  {
    fp->write(", ");
    fp->write(m_constSymbol->getMangledName().c_str());
    fp->write("(");
    fp->write(m_nodeExpr->getName());
    fp->write(")");
  } //genCodeConstructorInitialization

  // like NodeVarDecl for model parameters
  void NodeParameterDef::generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount)
  {
    UTI nuti = getNodeType();

    //output a case of switch statement
    m_state.indent(fp);
    fp->write("case ");
    fp->write_decimal(dmcount);
    fp->write(": { static UlamClassDataMemberInfo i(\"");
    fp->write(m_state.getUlamTypeByIndex(nuti)->getUlamTypeMangledName().c_str());
    fp->write("\", \"");
    fp->write(m_state.m_pool.getDataAsString(m_constSymbol->getId()).c_str());
    fp->write("\", ");
    fp->write("0u); return i; }\n"); //pos offset is 0

    dmcount++; //increment data member count
  } //generateUlamClassInfo



} //end MFM
