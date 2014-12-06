#include <assert.h>
#include "UEventWindow.h"
#include "CompilerState.h"

namespace MFM {

  UEventWindow::UEventWindow(CompilerState & state): m_state(state)
  {
    init();
  }


  UEventWindow::~UEventWindow()
  {
    //clear();
  }


  void UEventWindow::clear()
  {
    m_diamondOfSites.clear();
  }


  void UEventWindow::init()
  {
    u32 count = 0;
    for(s32 i = 0; i <= MAXMANHATTANDIST; i++)
      for(s32 j = 0; j <= MAXMANHATTANDIST - i; j++)
	{
	  Coord c(i,j);
	  Site s;
	  m_diamondOfSites.insert(std::pair<Coord,Site>(c,s));
	  count++;
	}

    for(s32 i = 1; i <= MAXMANHATTANDIST; i++)
      for(s32 j = 0; j <= MAXMANHATTANDIST - i; j++)
	{
	  Coord c(-i,j);
	  Site s;
	  m_diamondOfSites.insert(std::pair<Coord,Site>(c,s));
	  count++;
	}

    for(s32 i = 0; i <= MAXMANHATTANDIST; i++)
      for(s32 j = 1; j <= MAXMANHATTANDIST - i; j++)
	{
	  Coord c(i,-j);
	  Site s;
	  m_diamondOfSites.insert(std::pair<Coord,Site>(c,s));
	  count++;
	}

    for(s32 i = 1; i <= MAXMANHATTANDIST; i++)
      for(s32 j = 1; j <= MAXMANHATTANDIST - i; j++)
	{
	  Coord c(-i,-j);
	  Site s;
	  m_diamondOfSites.insert(std::pair<Coord,Site>(c,s));
	  count++;
	}

    //sanity check number of entries in the map:
    u32 shouldTotal = 1;
    s32 k = MAXMANHATTANDIST;
    while (k > 0)
      {
	shouldTotal += MAXMANHATTANDIST * k;
	k--;
      }
    assert(count == shouldTotal);
  }

  //tests and converts index to valid coords, preferred way to do conversion
  bool UEventWindow::isValidSite(u32 index, Coord& c)
  {
    bool rtnB = false;
    if(index >= 0 && index < MAXWIDTH * MAXWIDTH)
      {
	c = Coord::convertIndexToCoord(index);
	rtnB = isValidSite(c);
      }
    return rtnB;
  }


  bool UEventWindow::isValidSite(Coord c)
  {
    return (((c.x + c.y) >= -MAXMANHATTANDIST) && ((c.x + c.y) <= MAXMANHATTANDIST));
  }


  bool UEventWindow::getSite(Coord c, Site* & sref)
  {
    bool rtnB = false;
    std::map<Coord, Site, less_than_coord>::iterator it = m_diamondOfSites.find(c);
    if(it != m_diamondOfSites.end())
      {
	sref = &(it->second);
	rtnB = true;
      }
    return rtnB;
  }


  UTI UEventWindow::getSiteElementType(u32 index) //passes through to AtomValue at site
  {
    Coord c;
    assert(isValidSite(index, c));
    UTI uti = Nav;  //init
    Site * s;
    if(getSite(c,s))
      uti = s->getElementTypeNumber();

    return uti;
  }


  void UEventWindow::setSiteElementType(u32 index, UTI type)
  {
    Coord c;
    assert(isValidSite(index, c));
    setSiteElementType(c,type);
  }


  void UEventWindow::setSiteElementType(Coord c, UTI type)
  {
    Site * s;
    if(getSite(c,s))
      s->setElementTypeNumber(type);
  }


  bool UEventWindow::isSiteLive(Coord c)
  {
    Site * s;
    if(getSite(c,s))
      return s->isSiteLive();

    //error site doesn't exist
    return false;
  }


  void UEventWindow::setSiteLive(Coord c, bool b)
  {
    Site * s;
    if(getSite(c,s))
      s->setSiteLive(b);

    //error site doesn't exist
    assert(0);
  }


  UlamValue UEventWindow::loadAtomFromSite(u32 index) //converts index into 2D coord, returns Atom
  {
    Coord c;
    assert(isValidSite(index, c));
    Site * s;
    if(getSite(c,s))
      return s->getSiteUlamValue();

    //error!
    return UlamValue();  //Nav
  }


  bool UEventWindow::storeAtomIntoSite(u32 index, UlamValue uv)
  {
    Coord c;
    assert(isValidSite(index, c));
    return storeAtomIntoSite(c,uv);
  }


  bool UEventWindow::storeAtomIntoSite(Coord c, UlamValue v)
  {
    bool rtnB = false;
    Site * s;
    if(getSite(c,s))
      {
	s->setSiteUlamValue(v);
	rtnB = true;
      }
    //error!
    return rtnB;
  }


  void UEventWindow::assignUlamValue(UlamValue pluv, UlamValue ruv)
  {
    assert(pluv.getUlamValueTypeIdx() == Ptr);
    assert(ruv.getUlamValueTypeIdx()  != Ptr);      // not a Ptr

    u32 leftindex = pluv.getPtrSlotIndex();    //even for scalars
    bool stored = false;
    if(pluv.isTargetPacked() == UNPACKED)
      {
	stored = storeAtomIntoSite(leftindex, ruv);
      }
    else
      {
	//target is packed, use pos & len in ptr, unless there's no type
	UlamValue lvalAtIdx = loadAtomFromSite(leftindex);

	// if uninitialized, copy the entire ruv and set type to Atom
	if(lvalAtIdx.getUlamValueTypeIdx() == Nav)
	  {
	    stored = storeAtomIntoSite(leftindex, ruv);
	    setSiteElementType(leftindex, UAtom);
	  }
	else
	  {
	    lvalAtIdx.putDataIntoAtom(pluv, ruv, m_state);
	    stored = storeAtomIntoSite(leftindex, lvalAtIdx);
	  }
      }
    assert(stored);
  }


  void UEventWindow::assignUlamValuePtr(UlamValue pluv, UlamValue puv)
  {
    assert(0); //invalid assignment
  }


  UlamValue UEventWindow::makePtrToCenter()
  {
    return makePtrToSite(Coord(0,0));
  }


  UlamValue UEventWindow::makePtrToSite(Coord c)
  {
    u32 cidx = c.convertCoordToIndex();
    UlamValue ptr = UlamValue::makePtr(cidx, EVENTWINDOW, getSiteElementType(cidx), UNPACKED, m_state);
    return ptr;
  }


} //MFM
