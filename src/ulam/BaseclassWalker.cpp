#include "BaseclassWalker.h"

namespace MFM {

  BaseclassWalker::BaseclassWalker() : m_breadthfirst(true) {}

  BaseclassWalker::BaseclassWalker(bool breadthfirst) : m_breadthfirst(breadthfirst) {}

  BaseclassWalker::~BaseclassWalker()
  {
    m_bases.clear();
    m_seenset.clear();
  }

  void BaseclassWalker::init(UTI cuti)
  {
    assert(m_bases.empty());
    m_bases.push_back(cuti); //optional
  }

  bool BaseclassWalker::isDone()
  {
    return m_bases.empty();
  }

  void BaseclassWalker::addAncestorsOf(SymbolClass * csymptr)
  {
    assert(csymptr);
    u32 basecount = csymptr->getBaseClassCount() + 1; //include super
    for(u32 i = 0; i < basecount; i++)
      {
	if(m_breadthfirst)
	  m_bases.push_back(csymptr->getBaseClass(i)); //extends FIFO queue with next level of base UTIs
	else
	  m_bases.push_front(csymptr->getBaseClass(i)); //extends LIFO stack with next level of base UTIs
      }
  }


  bool BaseclassWalker::getNextBase(UTI& nextbase)
  {
    bool rtnb = false;
    UTI baseuti = Nouti;
    std::pair<std::set<UTI>::iterator,bool> ret;

    while(!rtnb && !m_bases.empty())
      {
	baseuti = m_bases.front(); //always from the front, regardless of breadth or depth-first
	m_bases.pop_front();
	ret = m_seenset.insert(baseuti);
	if(ret.second)
	  {
	    rtnb = true; //first-sighting
	    nextbase = baseuti;
	  } //else already-seen, try next one..
      } //end while

    if(!rtnb)
      nextbase = Nouti;
    return rtnb;
  } //getNextBase

} //MFM
