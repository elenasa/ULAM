#include <sstream>
#include "Tokenizer.h"
#include "Token.h"

namespace MFM {

  Tokenizer::Tokenizer(): m_haveUnreadToken(false) {}

  Tokenizer::~Tokenizer(){}

  void Tokenizer::unreadToken()
  {
    m_haveUnreadToken = true;
  }

} //end MFM


