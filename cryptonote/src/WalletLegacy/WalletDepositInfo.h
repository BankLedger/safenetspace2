
// Copyright (c) 2014-2016 XDN developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "IWallet.h"

namespace CryptoNote {

struct DepositInfo {
  Deposit deposit;
  uint32_t outputInTransaction;
#if SQ_ADD_BM2
  uint8_t m_wallet_version;
  uint8_t m_id;
#endif
};

}
