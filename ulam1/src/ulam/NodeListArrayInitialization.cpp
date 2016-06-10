#include <stdio.h>
#include <assert.h>
#include "NodeListArrayInitialization.h"
#include "NodeTerminal.h"
#include "CompilerState.h"
#include "SymbolVariable.h"

namespace MFM{

  NodeListArrayInitialization::NodeListArrayInitialization(CompilerState & state) : NodeList(state)
  {
    setNodeType(Void); //initialized to Void
  }

  NodeListArrayInitialization::NodeListArrayInitialization(const NodeListArrayInitialization & ref) : NodeList(ref) { }

  NodeListArrayInitialization::~NodeListArrayInitialization() { }

  Node * NodeListArrayInitialization::instantiate()
  {
    return new NodeListArrayInitialization(*this);
  }

  const char * NodeListArrayInitialization::getName()
  {
    return "arrayinitlist";
  }

  const std::string NodeListArrayInitialization::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  bool NodeListArrayInitialization::isAConstant()
  {
    bool rtnc = true;
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	rtnc |= m_nodes[i]->isAConstant();
      }
    return rtnc;
  }

  UTI NodeListArrayInitialization::checkAndLabelType()
  {
    //the size of the list may be less than the array size
    UTI rtnuti = Node::getNodeType(); //init to Void; //ok
    if(rtnuti == Hzy)
      rtnuti = Void; //resets
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	UTI puti = m_nodes[i]->checkAndLabelType();
	if(!m_state.okUTItoContinue(puti))
	  {
	    std::ostringstream msg;
	    msg << "Argument " << i + 1 << " has a problem";
	    if(puti == Nav)
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    else
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		m_state.setGoAgain();
	      }
	    rtnuti = puti;
	  }
	else if(puti == Void)
	  {
	    std::ostringstream msg;
	    msg << "Invalid type for array item " << i + 1;
	    msg << " initialization: Void";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    rtnuti = Nav;
	  }
	else if(!m_nodes[i]->isAConstant())
	  {
	    std::ostringstream msg;
	    msg << "Constant value expression for array item " << i + 1;
	    msg << ", initialization is not a constant";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    rtnuti = Nav;
	  }
      }
    setNodeType(rtnuti);
    return rtnuti;
  } //checkAndLabelType

  bool NodeListArrayInitialization::foldInitExpression()
  {
    bool rtnok = true;
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	rtnok |= foldInitExpression(i);
	if(!rtnok)
	  break;
      }
    return rtnok;
  }

  bool NodeListArrayInitialization::foldInitExpression(u32 n)
  {
    UTI foldeduti = m_nodes[n]->constantFold(); //c&l possibly redone

    //insure constant value fits in its array's bitsize
    UTI scalaruti = m_state.getUlamTypeAsScalar(Node::getNodeType());
    if(UlamType::compareForArgumentMatching(foldeduti, scalaruti, m_state) != UTIC_SAME)
      {
	FORECAST scr = m_nodes[n]->safeToCastTo(scalaruti);
	if(scr == CAST_CLEAR)
	  {
	    //must complicate the parse tree (t3769 Int(4) == -1);
	    if(!Node::makeCastingNode(m_nodes[n], scalaruti, m_nodes[n]))
	      {
		std::ostringstream msg;
		msg << "Array item " << n + 1 << " has a casting problem";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		foldeduti = Nav; //no casting node
	      }
	    else
	      foldeduti = m_nodes[n]->getNodeType(); //casted item
	  }
	else
	  {
	    std::ostringstream msg;
	    if(m_state.getUlamTypeByIndex(foldeduti)->getUlamTypeEnum() == Bool)
	      msg << "Use a comparison operator";
	    else
	      msg << "Use explicit cast";
	    msg << " to return ";
	    msg << m_state.getUlamTypeNameBriefByIndex(foldeduti).c_str();
	    msg << " as ";
	    msg << m_state.getUlamTypeNameBriefByIndex(scalaruti).c_str();
	    if(scr == CAST_BAD)
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		foldeduti = Nav;
	      }
	    else
	      {
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT);
		foldeduti = Hzy;
	      }
	  }
      }
    return m_state.okUTItoContinue(foldeduti);
  } //foldInitExpression

  bool NodeListArrayInitialization::buildArrayValueInitialization(BV8K& bvtmp)
  {
    bool rtnok = true;
    u32 n = m_nodes.size();
    for(u32 i = 0; i < n; i++)
      {
	rtnok |= buildArrayItemInitialValue(i, bvtmp);
	if(!rtnok)
	  break;
      }

    if(rtnok)
      {
	UTI nuti = Node::getNodeType();
	s32 arraysize = m_state.getArraySize(nuti);
	assert(arraysize > 0);
	//propagate last value for any remaining items not initialized
	for(s32 i = n; i < arraysize; i++)
	  {
	    rtnok |= buildArrayItemInitialValue(n - 1, bvtmp);
	    if(!rtnok)
	      break;
	  }
      }
    return rtnok;
  } //buildArrayValueInitialization

  bool NodeListArrayInitialization::buildArrayItemInitialValue(u32 n, BV8K& bvtmp)
  {
    UTI nuti = Node::getNodeType();
    UTI luti = m_nodes[n]->getNodeType();

    evalNodeProlog(0); //new current frame pointer
    makeRoomForNodeType(luti); //a constant expression
    EvalStatus evs = eval(n);
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return false;
      }

    UlamValue ituv = m_state.m_nodeEvalStack.popArg();

    u64 foldedconst;
    u32 itemlen = m_state.getBitSize(nuti);

    if(itemlen <= MAXBITSPERINT)
      foldedconst = ituv.getImmediateData(itemlen, m_state);
    else if(itemlen <= MAXBITSPERLONG)
      foldedconst = ituv.getImmediateDataLong(itemlen, m_state);
    else
      assert(0);

    evalNodeEpilog();

    bvtmp.WriteLong(n * itemlen, itemlen, foldedconst);
    return true;
  } //buildArrayItemInitialValue

  void NodeListArrayInitialization::genCode(File * fp, UVPass& uvpass)
  {
    UTI nuti = Node::getNodeType();
    assert(!m_state.isScalar(nuti));
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    // returns a u32 array of the proper length built with BV8K
    // as tmpvar in uvpass
    // need parent (NodeVarDecl) to get initialized value (BV8K)
    NNO pno = Node::getYourParentNo();
    //m_state.pushCurrentBlockAndDontUseMemberBlock(currBlock); //push again
    Node * parentNode = m_state.findNodeNoInThisClass(pno);
    //m_state.popClassContext(); //restore
    if(!parentNode)
      {
	std::ostringstream msg;
	msg << "Array value '" << getName();
	msg << "' cannot be initialized at this time while compiling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	msg << " Parent required";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	assert(0); //parent required
	return;
      }

    SymbolVariable * varsym = NULL;
    AssertBool gotSymbol = parentNode->getSymbolPtr((Symbol *&) varsym);
    assert(gotSymbol);
    assert(varsym->hasInitValue());

    BV8K dval;
    AssertBool gotInitVal = varsym->getInitValue(dval);
    assert(gotInitVal);

    u32 uvals[ARRAY_LEN8K];
    dval.ToArray(uvals); //the magic!

    u32 tmpvarnum = m_state.getNextTmpVarNumber();
    TMPSTORAGE nstor = nut->getTmpStorageTypeForTmpVar();
    u32 nwords = nut->getTotalNumberOfWords();

    //static constant array of u32's from BV8K, of proper length:
    //similar to CS::genCodeClassDefaultConstantArray,
    // except indentUlamCode, to a tmpvar, and no BitVector.
    m_state.indentUlamCode(fp);
    fp->write("const u32 ");
    fp->write(m_state.getTmpVarAsString(nuti, tmpvarnum, nstor).c_str());
    fp->write("[");
    fp->write_decimal_unsigned(nwords); // proper length == [nwords]
    fp->write("] = { ");

    for(u32 w = 0; w < nwords; w++)
      {
	std::ostringstream dhex;
	dhex << "0x" << std::hex << uvals[w];

	if(w > 0)
	  fp->write(", ");

	fp->write(dhex.str().c_str());
      }
    fp->write(" };\n");

    uvpass = UVPass::makePass(tmpvarnum, nstor, nuti, m_state.determinePackable(nuti), m_state, 0, varsym->getId());
  } //genCode

} //MFM
