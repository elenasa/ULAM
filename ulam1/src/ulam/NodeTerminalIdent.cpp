#include <stdlib.h>
#include "NodeTerminalIdent.h"
#include "CompilerState.h"
#include "NodeBlockClass.h"
#include "SymbolVariableStatic.h"
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


  UlamType * NodeTerminalIdent::checkAndLabelType()
  {
    UlamType * it = m_state.getUlamTypeByIndex(Nav);  //Nav

    //use was before def, look up in class block
    if(m_varSymbol == NULL)
      {
	Symbol * asymptr;
	if(m_state.m_classBlock->isIdInScope(m_token.m_dataindex,asymptr))
	  {
	    if(!asymptr->isFunction())
	      {
		m_varSymbol = (SymbolVariable *) asymptr;
		assert(m_varSymbol->isDataMember());
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
	it = m_varSymbol->getUlamType();
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

    //copy result UV to stack, -1 relative to current frame pointer
    assignReturnValueToStack(m_varSymbol->getUlamValue(m_state));

    evalNodeEpilog();
    return NORMAL;
  }


  EvalStatus NodeTerminalIdent::evalToStoreInto()
  {
    assert(m_varSymbol);
    assert(isStoreIntoAble());

    evalNodeProlog(0); //new current frame pointer

    //copy result UV to stack, -1 relative to current frame pointer
    assignReturnValuePtrToStack(m_varSymbol->getUlamValueToStoreInto());

    evalNodeEpilog();
    return NORMAL;
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
    bitsize = (bitsize == 0 ? (bUT == Bool ? 8 : 32) : bitsize); //temporary!!!

    //type names begin with capital letter..and the rest can be either case
    //std::string basetypeName  = m_state.getTokenAsATypeName(aTok); //Int, etc; 'Nav' if invalid
    //assert(basetypeName.compare("Nav") != 0); //checked beforehand
    u32 basetypeNameId  = m_state.getTokenAsATypeNameId(aTok); //Int, etc; 'Nav' if invalid

    UlamKeyTypeSignature key(basetypeNameId, bitsize, arraysize);

    // o.w. build symbol, first the base type ( with array size)
    UTI uti = m_state.makeUlamType(key, bUT);  
    UlamType * ut = m_state.getUlamTypeByIndex(uti);

    //create a symbol for this new ulam type, a typedef, with its type
    SymbolTypedef * symtypedef = new SymbolTypedef(m_token.m_dataindex, ut);
    m_state.addSymbolToCurrentScope(symtypedef);

    //gets the symbol just created by makeUlamType
    return (m_state.m_currentBlock->isIdInScope(m_token.m_dataindex,asymptr));  //true
  }


  //see also NodeBinaryOpSquareBracket
  bool NodeTerminalIdent::installSymbol(Token aTok, u32 arraysize, Symbol *& asymptr)
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
    // if a primitive (array size 0), we may need to make a new arraysize type for it.
    std::string typeName  = m_state.getTokenAsATypeName(aTok); //Foo, Int, etc
    UlamType * aut = NULL;
    bool brtn = false;
    if(m_state.getUlamTypeByTypedefName(typeName.c_str(), aut))
      {
	brtn = true;
      }
    else
      {
	if(Token::getSpecialTokenWork(aTok.m_type) == TOKSP_TYPEKEYWORD)
	  {
	    //this was automatically creating UlamTypes for the base types with different array sizes.
	    //but with typedef's we want "scope" of use, so it needs to be checked.
	    
	    // o.w. build symbol, first base type ( with array size)
	    aut = m_state.makeUlamType(aTok, 0, arraysize);
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


  SymbolVariable *  NodeTerminalIdent::makeSymbol(UlamType * aut)
  {
    //adjust decl count and max_depth, used for function definitions
    u32 arraysize = aut->getArraySize();
   

    if(m_state.m_currentFunctionBlockDeclSize == 0)
      {
	// when current block and class block are the same, this is a data member
	// assert(m_state.m_currentBlock == (NodeBlock *) m_state.m_classBlock);
	// fails when using a data member inside a function block!!!
	UlamType * but = aut;

	// get UlamType for arrays
	if(arraysize > 0)
	  {
	    but = m_state.getUlamTypeAsScalar(aut);
	  }

	//UlamValue val(aut, but);  //array, base ulamtype args
	u32 baseslot = m_state.m_selectedAtom.pushDataMember(aut,but);

	//variable-index, ulamtype, ulamvalue(ownership to symbol)    
	return (new SymbolVariableStatic(m_token.m_dataindex, aut, baseslot)); 
      }

    //Symbol is a parameter; always on the stack
    if(m_state.m_currentFunctionBlockDeclSize < 0)
      {
	m_state.m_currentFunctionBlockDeclSize -= (arraysize > 0 ? arraysize : 1);   //1 slot for scalar
	return (new SymbolVariableStack(m_token.m_dataindex, aut, m_state.m_currentFunctionBlockDeclSize + 1)); //slot after adjust, plus 1
      }

    //Symbol is a local variable, always on the stack 
    SymbolVariableStack * rtnLocalSym = new SymbolVariableStack(m_token.m_dataindex, aut, m_state.m_currentFunctionBlockDeclSize); //slot before adjustment

    m_state.m_currentFunctionBlockDeclSize += (arraysize > 0 ? arraysize : 1);
    
    //adjust max depth, excluding parameters and initial start value (=1)
    if(m_state.m_currentFunctionBlockDeclSize - 1 > m_state.m_currentFunctionBlockMaxDepth)
      m_state.m_currentFunctionBlockMaxDepth = m_state.m_currentFunctionBlockDeclSize - 1;
    
    return rtnLocalSym;
  }


  void NodeTerminalIdent::genCode(File * fp)
  {
    fp->write(m_varSymbol->getMangledName(&m_state).c_str());
  }

} //end MFM
