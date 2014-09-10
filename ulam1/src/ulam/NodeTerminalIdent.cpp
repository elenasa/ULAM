#include <stdlib.h>
#include "NodeTerminalIdent.h"
#include "CompilerState.h"
#include "NodeBlockClass.h"
#include "SymbolVariableDataMember.h"
#include "SymbolVariableStack.h"
#include "SymbolTypedef.h"

namespace MFM {

  NodeTerminalIdent::NodeTerminalIdent(Token tok, SymbolVariable * symptr, CompilerState & state) : NodeTerminal(tok, state), m_varSymbol(symptr) {}

  NodeTerminalIdent::~NodeTerminalIdent(){}


  void NodeTerminalIdent::printPostfix(File * fp)
  {
    fp->write(" ");
    fp->write(getName());
  }


  const char * NodeTerminalIdent::getName()
  {
    return m_state.getDataAsString(&m_token).c_str();
  }


  const std::string NodeTerminalIdent::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UTI NodeTerminalIdent::checkAndLabelType()
  {
    UTI it = Nav;  //init

    //use was before def, look up in class block
    if(m_varSymbol == NULL)
      {
	Symbol * asymptr = NULL;
	//if(m_state.m_classBlock->isIdInScope(m_token.m_dataindex,asymptr))
	//if(m_state.isIdInClassScope(m_token.m_dataindex,asymptr))
	if(m_state.alreadyDefinedSymbol(m_token.m_dataindex,asymptr))
	  {
	    if(!asymptr->isFunction())
	      {
		m_varSymbol = (SymbolVariable *) asymptr;
		//assert(m_varSymbol->isDataMember()); //could be a local variable
	      }
	    else
	      {
		std::ostringstream msg;
		msg << "(1) <" << m_state.m_pool.getDataAsString(m_token.m_dataindex).c_str() << "> is not a variable, and cannot be used as one";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      }
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "(2) <" << m_state.m_pool.getDataAsString(m_token.m_dataindex).c_str() << "> is not defined, and cannot be used";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }	  
      }

    if(m_varSymbol)
      {
	it = m_varSymbol->getUlamTypeIdx();
      }
    
    setNodeType(it);
    
    //if(it->isScalar())
    setStoreIntoAble(true);

    return it;
  }


  EvalStatus NodeTerminalIdent::eval()
  {
    assert(m_varSymbol);
    evalNodeProlog(0); //new current frame pointer

    //return the ptr for an array; square bracket will resolve down to the immediate data
    UlamValue uv;
    UlamValue uvp = makeUlamValuePtr();
    if(m_state.isScalar(getNodeType()))
      uv = m_state.getPtrTarget(uvp);
    else
      uv = uvp;

    //copy result UV to stack, -1 relative to current frame pointer
    assignReturnValueToStack(uv);

    evalNodeEpilog();
    return NORMAL;
  }


  EvalStatus NodeTerminalIdent::evalToStoreInto()
  {
    assert(m_varSymbol);
    assert(isStoreIntoAble());

    evalNodeProlog(0);         //new current node eval frame pointer

    UlamValue rtnUVPtr = makeUlamValuePtr();
 
    //copy result UV to stack, -1 relative to current frame pointer
    assignReturnValuePtrToStack(rtnUVPtr);

    evalNodeEpilog();
    return NORMAL;
  }
    

  UlamValue NodeTerminalIdent::makeUlamValuePtr()
  {
    UlamValue ptr;
    ULAMCLASSTYPE classtype = m_state.getUlamTypeByIndex(getNodeType())->getUlamClass();
    if(classtype == UC_ELEMENT)
      {
	// ptr to explicit atom or element, (e.g. 'f' in f.a=1;) to become new m_currentObjPtr
	ptr = UlamValue::makePtr(m_varSymbol->getStackFrameSlotIndex(), STACK, getNodeType(), false, m_state); 
      }
    else
      {
	if(m_varSymbol->isDataMember())
	  {
	    //#define VERIFYHIDDENARG2
#ifdef VERIFYHIDDENARG2
	    {
	      // Curious: is m_currentObjPtr the same as the "hidden" first arg on the STACK?
	      UTI futi = m_state.m_currentFunctionReturnType;
	      s32 firstArgSlot = -1;
	      if(m_state.determinePackable(futi))
		firstArgSlot--;
	      else
		{
		  s32 arraysize = m_state.getArraySize(futi);    
		  firstArgSlot -= (arraysize > 0 ? arraysize: 1);
		}
	      
	      UlamValue firstArg = m_state.m_funcCallStack.loadUlamValueFromSlot(firstArgSlot);
	      assert(firstArg.getUlamValueTypeIdx() == m_state.m_currentObjPtr.getUlamValueTypeIdx());
	      assert(firstArg.getUlamValueTypeIdx() == Ptr);
	      assert(firstArg.getPtrTargetType() == m_state.m_currentObjPtr.getPtrTargetType());
	      assert(firstArg.getPtrStorage() == m_state.m_currentObjPtr.getPtrStorage());
	      assert(firstArg.getPtrPos() == m_state.m_currentObjPtr.getPtrPos());
	      //////////////////////////////////////////////////////
	    }
#endif

	    // return ptr to this data member within the m_currentObjPtr (also same as "hidden" first arg)
	    // 'pos' modified by this data member symbol's packed bit position
	    ptr = UlamValue::makePtr(m_state.m_currentObjPtr.getPtrSlotIndex(), m_state.m_currentObjPtr.getPtrStorage(), getNodeType(), m_state.determinePackable(getNodeType()), m_state, m_state.m_currentObjPtr.getPtrPos() + m_varSymbol->getPosOffset());
	  }
	else
	  {
	    //local variable on the stack; could be array ptr!
	    ptr = UlamValue::makePtr(m_varSymbol->getStackFrameSlotIndex(), STACK, getNodeType(), m_state.determinePackable(getNodeType()), m_state);
	  }
      }
    
    return ptr;
  }
  
  
  bool NodeTerminalIdent::getSymbolPtr(Symbol *& symptrref)
  {
    symptrref = m_varSymbol;
    return true;
  }


  bool NodeTerminalIdent::installSymbolTypedef(Token aTok, u32 bitsize, u32 arraysize, Symbol *& asymptr)
  {
    // ask current scope block if this variable name is there; 
    // if so, nothing to install return symbol and false
    // function names also checked when currentBlock is the classblock.
    if(m_state.m_currentBlock->isIdInScope(m_token.m_dataindex,asymptr))
      {
	return false;    //already there
      }

    ULAMTYPE bUT = m_state.getBaseTypeFromToken(aTok);
    bitsize = ((bitsize == 0 && bUT != Class) ? (bUT == Bool ? BITSPERBOOL : 32) : bitsize); //temporary!!!

    //type names begin with capital letter..and the rest can be either case
    u32 basetypeNameId  = m_state.getTokenAsATypeNameId(aTok); //Int, etc; 'Nav' if invalid

    UlamKeyTypeSignature key(basetypeNameId, bitsize, arraysize);

    // o.w. build symbol, first the base type (with array size)
    UTI uti = m_state.makeUlamType(key, bUT);  
    //UlamType * ut = m_state.getUlamTypeByIndex(uti);

    //create a symbol for this new ulam type, a typedef, with its type
    SymbolTypedef * symtypedef = new SymbolTypedef(m_token.m_dataindex, uti, m_state);
    m_state.addSymbolToCurrentScope(symtypedef);

    //gets the symbol just created by makeUlamType
    return (m_state.m_currentBlock->isIdInScope(m_token.m_dataindex,asymptr));  //true
  }


  //see also NodeBinaryOpSquareBracket
  bool NodeTerminalIdent::installSymbolVariable(Token aTok, u32 arraysize, Symbol *& asymptr)
  {
    // ask current scope block if this variable name is there; 
    // if so, nothing to install return symbol and false
    // function names also checked when currentBlock is the classblock.
    if(m_state.m_currentBlock->isIdInScope(m_token.m_dataindex,asymptr))
      {
	if(!(asymptr->isFunction()) && !(asymptr->isTypedef()))
	  m_varSymbol = (SymbolVariable *) asymptr;  //updates Node's symbol, if is variable
	return false;    //already there
      }

    // verify typedef exists for this scope; or is a primitive keyword type
    // if a primitive (array size 0), we may need to make a new arraysize type for it;
    // or if it is a class type (quark, element).
    UTI aut = Nav;
    bool brtn = false;

    if(m_state.getUlamTypeByTypedefName(aTok.m_dataindex, aut))
      {
	brtn = true;
      }
    else
      {
	if(Token::getSpecialTokenWork(aTok.m_type) == TOKSP_TYPEKEYWORD)
	  {
	    //UlamTypes automatically created for the base types with different array sizes.
	    //but with typedef's "scope" of use, typedef needs to be checked first.
	    
	    // o.w. build symbol, first base type (with array size)
	    aut = m_state.makeUlamType(aTok, 0, arraysize);
	    brtn = true;
	  }
	else 
	  {
	    // will substitute placeholder class type if it hasn't been seen yet
	    m_state.getUlamTypeByClassToken(aTok, aut);  
	    brtn = true;
	  }
      }

    if(brtn)
      {
	SymbolVariable * sym = makeSymbol(aut);
	m_state.addSymbolToCurrentScope(sym); //ownership goes to the block
	
	m_varSymbol = sym;
	asymptr = sym;
      }

    return brtn;
  }


  SymbolVariable *  NodeTerminalIdent::makeSymbol(UTI aut)
  {
    //adjust decl count and max_depth, used for function definitions
    u32 arraysize = m_state.getArraySize(aut);
   
    if(m_state.m_currentFunctionBlockDeclSize == 0)
      {
	// when current block and class block are the same, this is a data member
	// assert(m_state.m_currentBlock == (NodeBlock *) m_state.m_classBlock);
	// assert fails when using a data member inside a function block!!!
	//UTI but = aut;
	//
	// get UlamType for arrays
	//if(arraysize > 0)
	//  {
	//    but = m_state.getUlamTypeAsScalar(aut);
	//  }
	//
	//UlamValue val(aut, but);  //array, base ulamtype args
	//u32 baseslot = m_state.m_eventWindow.pushDataMember(aut,but);
	u32 baseslot = 1;  //no longer stored unpacked

	//variable-index, ulamtype, ulamvalue(ownership to symbol); always packed
	return (new SymbolVariableDataMember(m_token.m_dataindex, aut, baseslot, m_state)); 
      }

    bool packit = m_state.determinePackable(aut);

    //Symbol is a parameter; always on the stack
    if(m_state.m_currentFunctionBlockDeclSize < 0)
      {
	if(packit)
	  m_state.m_currentFunctionBlockDeclSize -= 1; //1 slot for scalar or packed array
	else
	  m_state.m_currentFunctionBlockDeclSize -= (arraysize > 0 ? arraysize : 1); //1 slot for scalar;
	
	return (new SymbolVariableStack(m_token.m_dataindex, aut, packit, m_state.m_currentFunctionBlockDeclSize + 1, m_state)); //slot after adjust, plus 1
      }

    //Symbol is a local variable, always on the stack 
    SymbolVariableStack * rtnLocalSym = new SymbolVariableStack(m_token.m_dataindex, aut, packit, m_state.m_currentFunctionBlockDeclSize, m_state); //slot before adjustment

    if(packit)
      m_state.m_currentFunctionBlockDeclSize += (arraysize > 0 ? 1 + 1 : 1);
    else
      m_state.m_currentFunctionBlockDeclSize += (arraysize > 0 ? arraysize + 1 : 1);
    
    //adjust max depth, excluding parameters and initial start value (=1)
    if(m_state.m_currentFunctionBlockDeclSize - 1 > m_state.m_currentFunctionBlockMaxDepth)
      m_state.m_currentFunctionBlockMaxDepth = m_state.m_currentFunctionBlockDeclSize - 1;
    
    return rtnLocalSym;
  }


  void NodeTerminalIdent::genCode(File * fp)
  {
    fp->write(m_varSymbol->getMangledName().c_str());
  }


} //end MFM
