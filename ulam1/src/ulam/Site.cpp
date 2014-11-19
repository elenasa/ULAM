#include <assert.h>
#include "Site.h"
#include "CompilerState.h"

namespace MFM {

  Site::Site(): m_live(false)
  { 
    init(); 
  }


  Site::~Site()
  {}


  void Site::init()
  {
    m_site = UlamValue::makeAtom(); //type set 'Atom'; clears to zeros
  }


  UTI Site::getElementTypeNumber()
  {
    return m_site.getAtomElementTypeIdx();
  }


  void Site::setElementTypeNumber(UTI type)
  {
    m_site.setAtomElementTypeIdx(type);
  }


  bool Site::isSiteLive()
  {
    return m_live;
  }

  void Site::setSiteLive(bool b)
  {
    m_live = b;
  }


  UlamValue Site::getSiteUlamValue()
  {
    return m_site;
  }

  void Site::setSiteUlamValue(UlamValue uv)
  {
    m_site = uv;
  }

} //MFM
