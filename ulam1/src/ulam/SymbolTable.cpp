#include <assert.h>
#include <stdio.h>
#include "SymbolTable.h"
#include "SymbolFunction.h"
#include "NodeBlockFunctionDefinition.h"

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


  void SymbolTable::labelTableOfFunctions()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;  
	if(sym->isFunction())
	  {
	    NodeBlockFunctionDefinition * func = ((SymbolFunction *) sym)->getFunctionNode();
	    assert(func); //how would a function symbol be without a body?
	    func->checkAndLabelType();
	  }
	it++;
      }
  }


  void SymbolTable::genCodeForTableOfFunctions(File * fp)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;  
	if(sym->isFunction())
	  {
	    NodeBlockFunctionDefinition * func = ((SymbolFunction *) sym)->getFunctionNode();
	    assert(func); //how would a function symbol be without a body?
	    func->genCode(fp);
	  }
	it++;
      }
  }


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
	    u32 depth = ((SymbolFunction *) sym)->getFunctionNode()->getMaxDepth();
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
