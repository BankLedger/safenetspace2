
// Copyright (c) 2014-2016 XDN developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <vector>
#include <boost/variant.hpp>
#include "CryptoTypes.h"

namespace CryptoNote {

struct BaseInput {
  uint32_t blockIndex;
};

struct KeyInput {
  uint64_t amount;
  std::vector<uint32_t> outputIndexes;
  Crypto::KeyImage keyImage;
};

struct MultisignatureInput {
  uint64_t amount;
  uint8_t signatureCount;
  uint32_t outputIndex;
  uint32_t term;
};

struct KeyOutput {
  Crypto::PublicKey key;
};

struct MultisignatureOutput {
  std::vector<Crypto::PublicKey> keys;
  uint8_t requiredSignatureCount;
  uint32_t term;
};

typedef boost::variant<BaseInput, KeyInput, MultisignatureInput> TransactionInput;

typedef boost::variant<KeyOutput, MultisignatureOutput> TransactionOutputTarget;

struct TransactionOutput {
  uint64_t amount;
  TransactionOutputTarget target;
};

struct TransactionPrefix {
  uint8_t version;
  uint64_t unlockTime;
  std::vector<TransactionInput> inputs;
  std::vector<TransactionOutput> outputs;
  std::vector<uint8_t> extra;
};
struct Transaction : public TransactionPrefix {
  std::vector<std::vector<Crypto::Signature>> signatures;
};

struct BlockHeader {
  uint8_t majorVersion;
  uint8_t minorVersion;
  uint32_t nonce;
  uint64_t timestamp;
  Crypto::Hash previousBlockHash;
};

struct Block : public BlockHeader {
  Transaction baseTransaction;
  std::vector<Crypto::Hash> transactionHashes;
};

struct TransactionEntry {
	Transaction tx;
	std::vector<uint32_t> m_global_output_indexes;
};

struct BlockChain {
	uint64_t block_cumulative_size;
//	difficulty_type cumulative_difficulty;
	uint64_t already_generated_coins;
	//std::vector<TransactionEntry> m_transactions;

	uint32_t height;
	uint32_t size;
	uint32_t transactions;
	Crypto::Hash  hash;
	Block      block;
};

struct AccountPublicAddress {
  Crypto::PublicKey spendPublicKey;
  Crypto::PublicKey viewPublicKey;
};

struct AccountKeys {
  AccountPublicAddress address;
  Crypto::SecretKey spendSecretKey;
  Crypto::SecretKey viewSecretKey;

#if SQ_ADD_BM2
  std::string  m_name;
  uint8_t m_id;
#endif
};

struct KeyPair {
  Crypto::PublicKey publicKey;
  Crypto::SecretKey secretKey;
};

//#if SQ_ADD_BM_WALLET
struct AddressGroup
{
	std::string  m_address;
	std::string  m_name;
	uint8_t m_id;
	uint8_t m_type;
};
//#endif

using BinaryArray = std::vector<uint8_t>;

}
