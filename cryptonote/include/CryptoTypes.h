
// Copyright (c) 2014-2016 XDN developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <cstdint>
#include "CryptoNoteConfig.h"
#if SQ_ADD_BM
#include "Common/StringTools.h"
#endif

namespace Crypto {

struct Hash {
  uint8_t data[32];
};

struct PublicKey {
  uint8_t data[32];
};

struct SecretKey {
  uint8_t data[32];
};

struct KeyDerivation {
  uint8_t data[32];
};

struct KeyImage {
  uint8_t data[32];
};

struct Signature {
  uint8_t data[64];
};

struct TimeString {
	uint8_t data[8];
};

struct BufSpendPKey {
	uint8_t data[50];
};

struct BufViewPKey {
	uint8_t data[20];
};

struct BufSpendSKey {
	uint8_t data[21];
};

struct BufViewSKey {
	uint8_t data[20];
};

#if SQ_ADD_BM2
struct SecretKeyBM
{
	uint8_t m_id;
	SecretKey m_viewSecretkey;
	SecretKey m_spendSecretKey;
	PublicKey m_spendPubKey;
};
#endif
}
