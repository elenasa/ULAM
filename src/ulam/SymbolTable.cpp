#include <assert.h>
#include <stdio.h>
#include <iostream>
#include "SymbolTable.h"
#include "CompilerState.h"

namespace MFM {

  SymbolTable::SymbolTable(CompilerState& state): m_state(state) { }

  SymbolTable::SymbolTable(const SymbolTable& ref) : m_state(ref.m_state)
  {
    std::map<u32, Symbol* >::const_iterator it = ref.m_idToSymbolPtr.begin();
    while(it != ref.m_idToSymbolPtr.end())
      {
	u32 fid = it->first;
	Symbol * found = it->second;
	Symbol * cloned = found->clone();
	addToTable(fid, cloned);
	it++;
      }
  }

  SymbolTable::~SymbolTable()
  {
    //need to delete contents; ownership transferred here!!
    this->clearTheTable();
  }

  void SymbolTable::clearTheTable()
  {
    std::map<u32, Symbol* >::iterator it = this->m_idToSymbolPtr.begin();
    while(it != this->m_idToSymbolPtr.end())
      {
	Symbol * symptr = it->second;
	delete symptr;
	it->second = NULL;
	it++;
      }
    this->m_idToSymbolPtr.clear();
  } //clearTheTable

  bool SymbolTable::isInTable(u32 id, Symbol * & symptrref)
  {
    if(m_idToSymbolPtr.empty()) return false;
    std::map<u32, Symbol* >::iterator it = m_idToSymbolPtr.find(id);
    if(it != m_idToSymbolPtr.end())
      {
	symptrref = it->second;
	assert( symptrref->getId() == id);
	return true;
      }
    return false;
  } //isInTable

  void SymbolTable::addToTable(u32 id, Symbol* sptr)
  {
    std::pair<std::map<u32, Symbol *>::iterator, bool> reti;
    reti = m_idToSymbolPtr.insert(std::pair<u32,Symbol*> (id,sptr));
    assert(reti.second); //false if already existed, i.e. not added (leak?)
  }

  void SymbolTable::replaceInTable(u32 oldid, u32 newid, Symbol * s)
  {
    std::map<u32,Symbol*>::iterator it = m_idToSymbolPtr.find(oldid);
    if(it != m_idToSymbolPtr.end())
      {
	Symbol * oldsym = it->second;
	assert(oldsym == s);
	m_idToSymbolPtr.erase(it);
      }
    addToTable(newid, s);
  } //replaceInTable

  void SymbolTable::replaceInTable(Symbol * oldsym, Symbol * newsym)
  {
    assert(oldsym->getId() == newsym->getId());
    std::map<u32,Symbol*>::iterator it = m_idToSymbolPtr.find(oldsym->getId());
    if(it != m_idToSymbolPtr.end())
      {
	Symbol * oldsymptr = it->second;
	assert(oldsymptr == oldsym);
	it->second = newsym;
	delete(oldsymptr);
      }
  } //replaceInTable (overload)

  bool SymbolTable::removeFromTable(u32 id, Symbol *& rtnsymptr)
  {
    bool rtnok = false;
    std::map<u32,Symbol*>::iterator it = m_idToSymbolPtr.find(id);
    if(it != m_idToSymbolPtr.end())
      {
	rtnsymptr = it->second;
	m_idToSymbolPtr.erase(it); //doesn't delete, up to caller
	rtnok = true;
      }
    return rtnok;
  } //removeFromTable

  Symbol * SymbolTable::getSymbolPtr(u32 id)
  {
    if(m_idToSymbolPtr.empty()) return NULL;

    std::map<u32,Symbol*>::iterator it = m_idToSymbolPtr.find(id);
    if(it != m_idToSymbolPtr.end())
      {
	return it->second;
      }
    return NULL; //impossible!!
  } //getSymbolPtr

  u32 SymbolTable::getTableSize()
  {
    return (m_idToSymbolPtr.empty() ? 0 : m_idToSymbolPtr.size());
  }

  u32 SymbolTable::getTotalSymbolSize()
  {
    assert(0);
    return 0;
  } //getTotalSymbolSize

} //end MFM
