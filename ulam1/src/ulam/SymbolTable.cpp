#include <assert.h>
#include <stdio.h>
#include "SymbolTable.h"
#include "SymbolFunctionName.h"
#include "SymbolVariable.h"
#include "CompilerState.h"

namespace MFM {

  SymbolTable::SymbolTable(){}
  SymbolTable::~SymbolTable()
  {
    // need to delete contents; ownership transferred here!!
    for(std::size_t i = 0; i < m_idToSymbolPtr.size(); i++)
      {
	delete m_idToSymbolPtr[i];
      }

    m_idToSymbolPtr.clear();
  }


  bool SymbolTable::isInTable(u32 id, Symbol * & symptrref)
  {
    std::map<u32, Symbol* >::iterator it = m_idToSymbolPtr.find(id);
    if(it != m_idToSymbolPtr.end())
      {
	symptrref = it->second;
	assert( symptrref->getId() == id);  //overkill?

	return true;
      }

    return false;
  }

  void SymbolTable::addToTable(u32 id, Symbol* sptr)
  {
    m_idToSymbolPtr.insert(std::pair<u32,Symbol*> (id,sptr));
  }


  Symbol * SymbolTable::getSymbolPtr(u32 id)
  {
    std::map<u32,Symbol*>::iterator it = m_idToSymbolPtr.find(id);
    if(it != m_idToSymbolPtr.end())
      {
	return it->second;
      }

    return NULL;  //impossible!!
  }


  void SymbolTable::genCodeForTableOfVariableDataMembers(File * fp, ULAMCLASSTYPE classtype, CompilerState * state)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;  
	if(!sym->isTypedef() && sym->isDataMember())
	  {
	    //updates the offset with the bit size of sym
	    ((SymbolVariable *) sym)->generateCodedVariableDeclarations(fp, classtype, state);
	  }
	it++;
      }
  }


  void SymbolTable::labelTableOfFunctions()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;  
	if(sym->isFunction())
	  {
	    ((SymbolFunctionName *) sym)->labelFunctions();
	  }
	it++;
      }
  }


  void SymbolTable::genCodeForTableOfFunctions(File * fp, bool declOnly, ULAMCLASSTYPE classtype, CompilerState * state)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;  
	if(sym->isFunction())
	  {
	    ((SymbolFunctionName *) sym)->generateCodedFunctions(fp, declOnly, classtype, state);
	  }
	it++;
      }
  }


  void SymbolTable::labelTableOfClasses(CompilerState& state)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;  
	assert(sym->isClass());
	if( ((SymbolClass *) sym)->getUlamClassType() == UC_INCOMPLETE)
	  {
	    std::ostringstream msg;
	    msg << "Incomplete Class <"  << sym->getUlamType()->getUlamTypeName(&state).c_str() << "> was never defined, fails labeling";
	    state.m_err.buildMessage("", msg.str().c_str(),__FILE__, __func__, __LINE__, MSG_ERR);
	    assert(0);
	  }
	else
	  {
	    NodeBlockClass * classNode = ((SymbolClass *) sym)->getClassBlockNode();
	    assert(classNode);
	    state.m_classBlock = classNode;
	    state.m_currentBlock = state.m_classBlock;

	    classNode->checkAndLabelType();
	    classNode->setNodeType(sym->getUlamType()); //resets class' type (was Void). sweet.

	    // separate pass...
	    //u32 totalbits = classNode->getBitSizesOfVariableSymbolsInTable(); //data members only
	    //sym->getUlamType()->setBitSize(totalbits);  //"scalar" Class bitsize
	  }
	it++;
      }
  }


  // separate pass...after labeling all classes is completed;
  bool SymbolTable::setBitSizeOfTableOfClasses(CompilerState& state)
  {
    bool aok = true;

    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;  
	assert(sym->isClass());
	if( ((SymbolClass *) sym)->getUlamClassType() == UC_INCOMPLETE)
	  {
	    std::ostringstream msg;
	    msg << "Incomplete Class <"  << sym->getUlamType()->getUlamTypeName(&state).c_str() << "> was never defined, fails sizing";
	    state.m_err.buildMessage("", msg.str().c_str(),__FILE__, __func__, __LINE__, MSG_ERR);
	    assert(0);
	  }
	else
	  {
	    NodeBlockClass * classNode = ((SymbolClass *) sym)->getClassBlockNode();
	    assert(classNode);
	    state.m_classBlock = classNode;
	    state.m_currentBlock = state.m_classBlock;

	    u32 totalbits = classNode->getBitSizesOfVariableSymbolsInTable(); //data members only
	    if(totalbits == 0)
	      {
		//std::ostringstream msg;
		//msg << "setting zero bit size symbol!! " << sym->getUlamType()->getUlamKeyTypeSignature().getUlamKeyTypeSignatureAsString(&state).c_str();
		//state.m_err.buildMessage("", msg.str().c_str(),__FILE__, __func__, __LINE__, MSG_DEBUG);
		aok = false;
	      }
	    else
	      {
		UlamType * sut = sym->getUlamType();
		sut->setBitSize(totalbits);  //"scalar" Class bitsize
		//std::ostringstream msg;
		//msg << "symbol size is aok (=" << totalbits << ", total= " << sut->getTotalBitSize() << ") " << sut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureAsString(&state).c_str();
		//state.m_err.buildMessage("", msg.str().c_str(),__FILE__, __func__, __LINE__, MSG_DEBUG);
	      }
	  }
	it++;
      }
    return aok;
  }


  u32 SymbolTable::getTotalVariableSymbolsBitSize(CompilerState& state)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    u32 totalsizes = 0;

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(!sym->isFunction());
	if(!sym->isTypedef())
	  {
	    UlamType * sut = sym->getUlamType();
	    s32 symsize = calcVariableSymbolTypeSize(sut,state);

	    if(symsize < 0)
	      {
		std::ostringstream msg;
		msg << "cycle error!! " << sut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureAsString(&state).c_str();
		state.m_err.buildMessage("", msg.str().c_str(),__FILE__, __func__, __LINE__, MSG_ERR);
	      }
	    else
	      {
		sut->setBitSize(symsize);  //total bits NOT including arrays
		//std::ostringstream msg;
		//msg << "symbol size is " << symsize << " (total = " << sut->getTotalBitSize() << ") " << sut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureAsString(&state).c_str();
		//state.m_err.buildMessage("", msg.str().c_str(),__FILE__, __func__, __LINE__, MSG_DEBUG);
	      }
	    totalsizes += sut->getTotalBitSize();
	  }
	it++;
      }
    return totalsizes;
  }


  s32 SymbolTable::calcVariableSymbolTypeSize(UlamType * argut, CompilerState& state)
  {
    s32 totbitsize = -1;
    if(argut->getUlamClassType() == UC_NOTACLASS)
      {
	totbitsize = argut->getBitSize();
	assert(totbitsize);
	return totbitsize;
      }

    //not a primitive
    if(argut->getArraySize() > 0)
      {
	if((totbitsize = argut->getBitSize()) > 0)
	  {
	    return totbitsize;
	  }
	if(totbitsize < 0)
	  {
	    //error! cycle
	    return -1;
	    assert(0);
	  }
	else // totbitsize == 0
	  {
	    argut->setBitSize(-1);  //before the recusive call..
	    //get base type
	    SymbolClass * csym = NULL;
	    if(state.alreadyDefinedSymbolClass(argut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureNameId(), csym))
	      {
		UlamType * but = csym->getUlamType();
		return calcVariableSymbolTypeSize(but, state);
	      }
	  }
      }
    else  // not primitive type, not array
      {
	if((totbitsize = argut->getBitSize()) > 0)
	  {
	    return totbitsize;
	  }

	if(totbitsize < 0)
	  {
	    //error! cycle
	    return -1;
	  }

	else //totbitsize == 0
	  {
	    //get base type
	    SymbolClass * csym = NULL;
	    if(state.alreadyDefinedSymbolClass(argut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureNameId(), csym))
	      {
		UlamType * but = csym->getUlamType();
		s32 bsize;
		
		if((bsize = but->getBitSize()) > 0)
		  {
		    return bsize;
		  }
		else if(bsize < 0)
		  {
		    //error! cycle..replace with message
		    return bsize;
		  }
		else
		  {
		    // ==0, redo variable total
		    NodeBlockClass * classblock = csym->getClassBlockNode();
		    assert(classblock);
		    assert(classblock != state.m_classBlock);
		    
		    bsize = classblock->getBitSizesOfVariableSymbolsInTable(); //data members only
		    return bsize;
		  }
	      }
	  } //totbitsize == 0
      } //not primitive, not array
    return -1;
  } 


  void SymbolTable::packBitsForTableOfClasses()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;  
	assert(sym->isClass());
	((SymbolClass *) sym)->getClassBlockNode()->packBitsForVariableDataMembers();	
	it++;
      }
  }


#ifdef OPTIMIZE_PACKED_BITS
  // currently, packing is done by Nodes since the order of declaration is available;
  // but in case we may want to optimize the layout someday, 
  // we keep this here since all the symbols are available in one place.
  void SymbolTable::packBitsForTableOfVariableDataMembers()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    u32 offsetIntoAtom = 0;

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;  
	if(!sym->isTypedef() && sym->isDataMember())
	  {
	    //updates the offset with the bit size of sym
	    ((SymbolVariable *) sym)->setPosOffset(offsetIntoAtom);
	    offsetIntoAtom += sym->getUlamType()->getTotalBitSize();  // times array size 
	  }
	it++;
      }
  }
#endif

  //bypasses THIS class being compiled
  void SymbolTable::generateIncludesForTableOfClasses(File * fp, CompilerState& state)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;  
	assert(sym->isClass());
	
	if(sym->getId() != state.m_compileThisId)
	  {
	    state.indent(fp);
	    fp->write("#include \"");
	    fp->write(state.getFileNameForAClassHeader(sym->getId()).c_str());
	    fp->write("\"\n");
	  }
	it++;
      }
    //fp->write("\n");
  }


#if 0
  //not sure we use this; go back and forth between the files that are output
  // if just this class, then NodeProgram can start the ball rolling
  void SymbolTable::genCodeForTableOfClasses(FileManager * fm, CompilerState& state)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;  
	assert(sym->isClass());

	//output header/body for THIS class only
	if(sym.getId() == state.m_compileThisId)
	  {
	    NodeBlockClass * classNode = ((SymbolClass *) sym)->getClassBlockNode();
	    assert(classNode);

	    m_state.m_classBlock = classNode;
	    m_state.m_currentBlock = m_state.m_classBlock;

	    ULAMCLASSTYPE uct = ((SymbolClass *) sym)->getUlamClassType();
	    if(uct == UC_ELEMENT)
	      {
		//output both header and body in separate files
		File * fp = fm->fopen(state.getFileNameForAClassHeader(sym.getId()).c_str(),WRITE);
		assert(fp);

		classNode->genCode(fp);	
		delete fp;

		// output body for This Class only
		File * fpb = fm->fopen(state.getFileNameForThisClassBody().c_str(),WRITE);
		assert(fpb);
		
		classNode->genCodeBody(fpb);	
		delete fpb;
	      }
	    else
	      {
		// for quarks output template struct in .h
		assert(uct == UC_QUARK);
		File * fp = fm->fopen(state.getFileNameForAClassHeader(sym.getId()).c_str(),WRITE);
		assert(fp);

		classNode->genCodeBody(fp);	
		delete fp;
	      }
	  }
	it++;
      }
  }
#endif


  u32 SymbolTable::getTableSize()
  {
    return (m_idToSymbolPtr.empty() ? 0 : m_idToSymbolPtr.size());
  }


  u32 SymbolTable::getTotalSymbolSize()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    u32 totalsizes = 0;

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isFunction())
	  {
	    u32 depth = ((SymbolFunctionName *) sym)->getDepthSumOfFunctions();
	    totalsizes += depth;
	  }
	else
	  {
	    u32 arraysize = sym->getUlamType()->getArraySize();
	    totalsizes += (arraysize > 0 ? arraysize : 1);
	  }
	it++;
      }
    return totalsizes;
  }



} //end MFM
