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

  UTI NodeParameterDef::checkAndLabelType()
  {
    UTI nodeType = NodeConstantDef::checkAndLabelType();
    if(nodeType != Nav)
      {
	UlamType * nut = m_state.getUlamTypeByIndex(nodeType);
	u32 wordsize = nut->getTotalWordSize();
	if(wordsize > MAXBITSPERINT)
	  {
	    std::ostringstream msg;
	    msg << "Model Parameter '" << m_state.m_pool.getDataAsString(m_cid).c_str();
	    msg << "' must fit in " << MAXBITSPERINT << " bits";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    nodeType = Nav;
	    setNodeType(Nav);
	  }
      }
    return nodeType;
  }

  void NodeParameterDef::checkForSymbol()
  {
    assert(!m_constSymbol);

    //in case of a cloned unknown
    NodeBlock * currBlock = getBlock();
    m_state.pushCurrentBlockAndDontUseMemberBlock(currBlock);

    Symbol * asymptr = NULL;
    if(m_state.alreadyDefinedSymbol(m_cid, asymptr))
      {
	if(asymptr->isModelParameter())
	  {
	    m_constSymbol = (SymbolParameterValue *) asymptr;
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "(1) <" << m_state.m_pool.getDataAsString(m_cid).c_str();
	    msg << "> is not a model parameter, and cannot be used as one";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "(2) Model Parameter <" << m_state.m_pool.getDataAsString(m_cid).c_str();
	msg << "> is not defined, and cannot be used";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      }
    m_state.popClassContext(); //restore
  } //checkForSymbol

  void NodeParameterDef::genCode(File * fp, UlamValue& uvpass)
  {
    m_state.indent(fp);
    fp->write("// declared as extern (below)\n");

  } //genCode

  void NodeParameterDef::genCodeExtern(File * fp, bool declOnly)
  {
    assert(m_constSymbol->isDataMember());

    //    UTI cuti = m_state.getCompileThisIdx();
    //ULAMCLASSTYPE classtype = m_state.getUlamTypeByIndex(cuti)->getUlamClass();

    UTI vuti = m_constSymbol->getUlamTypeIdx();
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);

    m_state.indent(fp);

    if(declOnly)
      fp->write("extern ");

    //common to both decl and def
    fp->write(vut->getImmediateStorageTypeAsString().c_str());
    fp->write(" ");
    fp->write(m_constSymbol->getMangledName().c_str());

    if(!declOnly)
      {
	fp->write("(");
	fp->write(m_nodeExpr->getName()); //initialize default value
	fp->write(")");
      }
    fp->write("; //model parameter\n");
  } //genCodeExtern

} //end MFM
