
// Copyright (c) 2014-2016 XDN developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "crypto/crypto.h"

#include <stdint.h>

namespace CryptoNote {

class ISerializer;

//This is DTO structure. Do not change it.
struct KeysStorage {
  uint64_t creationTimestamp;
  uint8_t  m_wallet_version = 2;

  Crypto::TimeString TimeString;
  Crypto::PublicKey m_buf;
  Crypto::BufSpendPKey BufSPKey;
  Crypto::BufViewPKey  BufVPKey;
  Crypto::BufSpendSKey BufSVKey;
  Crypto::BufViewSKey  BufVSKey;

  Crypto::PublicKey spendPublicKey;
  Crypto::SecretKey spendSecretKey;

  Crypto::PublicKey viewPublicKey;
  Crypto::SecretKey viewSecretKey;

#if SQ_ADD_BM2
  std::string  m_name;
  uint8_t m_id;
#endif

  void serialize(ISerializer& serializer, const std::string& name);
};

} //namespace CryptoNote
