
// Copyright (c) 2014-2016 XDN developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CryptoNoteFormatUtils.h"

#include <set>
#include <Logging/LoggerRef.h>
#include <Common/int-util.h>
#include <Common/Varint.h>

#include "Serialization/BinaryOutputStreamSerializer.h"
#include "Serialization/BinaryInputStreamSerializer.h"

#include "Account.h"
#include "CryptoNoteBasicImpl.h"
#include "CryptoNoteSerialization.h"
#include "TransactionExtra.h"
#include "CryptoNoteTools.h"

#include "CryptoNoteConfig.h"

using namespace Logging;
using namespace Crypto;
using namespace Common;

namespace CryptoNote {

bool parseAndValidateTransactionFromBinaryArray(const BinaryArray& tx_blob, Transaction& tx, Hash& tx_hash, Hash& tx_prefix_hash) {
  if (!fromBinaryArray(tx, tx_blob)) {
    return false;
  }

  //TODO: validate tx
  cn_fast_hash(tx_blob.data(), tx_blob.size(), tx_hash);
  getObjectHash(*static_cast<TransactionPrefix*>(&tx), tx_prefix_hash);
  return true;
}

bool generate_key_image_helper(const AccountKeys& ack, const PublicKey& tx_public_key, size_t real_output_index, KeyPair& in_ephemeral, KeyImage& ki) {
  KeyDerivation recv_derivation;
  bool r = generate_key_derivation(tx_public_key, ack.viewSecretKey, recv_derivation);

  assert(r && "key image helper: failed to generate_key_derivation");

  if (!r) {
    return false;
  }

  r = derive_public_key(recv_derivation, real_output_index, ack.address.spendPublicKey, in_ephemeral.publicKey);

  assert(r && "key image helper: failed to derive_public_key");

  if (!r) {
    return false;
  }

  derive_secret_key(recv_derivation, real_output_index, ack.spendSecretKey, in_ephemeral.secretKey);
  generate_key_image(in_ephemeral.publicKey, in_ephemeral.secretKey, ki);
  return true;
}

uint64_t power_integral(uint64_t a, uint64_t b) {
  if (b == 0)
    return 1;
  uint64_t total = a;
  for (uint64_t i = 1; i != b; i++)
    total *= a;
  return total;
}

bool constructTransaction(
  const AccountKeys& sender_account_keys,
  const std::vector<TransactionSourceEntry>& sources,
  const std::vector<TransactionDestinationEntry>& destinations,
  const std::vector<tx_message_entry>& messages,
  uint64_t ttl,
  std::vector<uint8_t> extra,
  Transaction& tx,
  uint64_t unlock_time,
  Logging::ILogger& log) {
  LoggerRef logger(log, "construct_tx");

  tx.inputs.clear();
  tx.outputs.clear();
  tx.signatures.clear();

  tx.version = TRANSACTION_VERSION_1;
  tx.unlockTime = unlock_time;

  tx.extra = extra;
  KeyPair txkey = generateKeyPair();
  addTransactionPublicKeyToExtra(tx.extra, txkey.publicKey);

  struct input_generation_context_data {
    KeyPair in_ephemeral;
  };

  std::vector<input_generation_context_data> in_contexts;
  uint64_t summary_inputs_money = 0;
  //fill inputs
  for (const TransactionSourceEntry& src_entr : sources) {
    if (src_entr.realOutput >= src_entr.outputs.size()) {
      logger(ERROR) << "real_output index (" << src_entr.realOutput << ")bigger than output_keys.size()=" << src_entr.outputs.size();
      return false;
    }
    summary_inputs_money += src_entr.amount;

    //KeyDerivation recv_derivation;
    in_contexts.push_back(input_generation_context_data());
    KeyPair& in_ephemeral = in_contexts.back().in_ephemeral;
    KeyImage img;
    if (!generate_key_image_helper(sender_account_keys, src_entr.realTransactionPublicKey, src_entr.realOutputIndexInTransaction, in_ephemeral, img))
      return false;

    //check that derived key is equal with real output key
    if (!(in_ephemeral.publicKey == src_entr.outputs[src_entr.realOutput].second)) {
      logger(ERROR) << "derived public key mismatch with output public key! " << ENDL << "derived_key:"
        << Common::podToHex(in_ephemeral.publicKey) << ENDL << "real output_public_key:"
        << Common::podToHex(src_entr.outputs[src_entr.realOutput].second);
      return false;
    }

    //put key image into tx input
    KeyInput input_to_key;
    input_to_key.amount = src_entr.amount;
    input_to_key.keyImage = img;

    //fill outputs array and use relative offsets
    for (const TransactionSourceEntry::OutputEntry& out_entry : src_entr.outputs) {
      input_to_key.outputIndexes.push_back(out_entry.first);
    }

    input_to_key.outputIndexes = absolute_output_offsets_to_relative(input_to_key.outputIndexes);
    tx.inputs.push_back(input_to_key);
  }

  // "Shuffle" outs
  std::vector<TransactionDestinationEntry> shuffled_dsts(destinations);
  std::sort(shuffled_dsts.begin(), shuffled_dsts.end(), [](const TransactionDestinationEntry& de1, const TransactionDestinationEntry& de2) { return de1.amount < de2.amount; });

  uint64_t summary_outs_money = 0;
  //fill outputs
  size_t output_index = 0;
  for (const TransactionDestinationEntry& dst_entr : shuffled_dsts) {
    if (!(dst_entr.amount > 0)) {
      logger(ERROR, BRIGHT_RED) << "Destination with wrong amount: " << dst_entr.amount;
      return false;
    }
    KeyDerivation derivation;
    PublicKey out_eph_public_key;
    bool r = generate_key_derivation(dst_entr.addr.viewPublicKey, txkey.secretKey, derivation);

    if (!(r)) {
      logger(ERROR, BRIGHT_RED)
        << "at creation outs: failed to generate_key_derivation("
        << dst_entr.addr.viewPublicKey << ", " << txkey.secretKey << ")";
      return false;
    }

    r = derive_public_key(derivation, output_index,
      dst_entr.addr.spendPublicKey,
      out_eph_public_key);
    if (!(r)) {
      logger(ERROR, BRIGHT_RED)
        << "at creation outs: failed to derive_public_key(" << derivation
        << ", " << output_index << ", " << dst_entr.addr.spendPublicKey
        << ")";
      return false;
    }

    TransactionOutput out;
    out.amount = dst_entr.amount;
    KeyOutput tk;
    tk.key = out_eph_public_key;
    out.target = tk;
    tx.outputs.push_back(out);
    output_index++;
    summary_outs_money += dst_entr.amount;
  }

  //check money
  if (summary_outs_money > summary_inputs_money) {
    logger(ERROR) << "Transaction inputs money (" << summary_inputs_money << ") less than outputs money (" << summary_outs_money << ")";
    return false;
  }

  for (size_t i = 0; i < messages.size(); i++) {
    const tx_message_entry &msg = messages[i];
    tx_extra_message tag;
    if (!tag.encrypt(i, msg.message, msg.encrypt ? &msg.addr : NULL, txkey)) {
      return false;
    }

    if (!append_message_to_extra(tx.extra, tag)) {
      return false;
    }
  }

  if (ttl != 0) {
    appendTTLToExtra(tx.extra, ttl);
  }

  //generate ring signatures
  Hash tx_prefix_hash;
  getObjectHash(*static_cast<TransactionPrefix*>(&tx), tx_prefix_hash);

  size_t i = 0;
  for (const TransactionSourceEntry& src_entr : sources) {
    std::vector<const PublicKey*> keys_ptrs;
    for (const TransactionSourceEntry::OutputEntry& o : src_entr.outputs) {
      keys_ptrs.push_back(&o.second);
    }

    tx.signatures.push_back(std::vector<Signature>());
    std::vector<Signature>& sigs = tx.signatures.back();
    sigs.resize(src_entr.outputs.size());
    generate_ring_signature(tx_prefix_hash, boost::get<KeyInput>(tx.inputs[i]).keyImage, keys_ptrs,
      in_contexts[i].in_ephemeral.secretKey, src_entr.realOutput, sigs.data());
    i++;
  }

  return true;
}

uint32_t get_block_height(const Block& b) {
  if (b.baseTransaction.inputs.size() != 1) {
    return 0;
  }
  const auto& in = b.baseTransaction.inputs[0];
  if (in.type() != typeid(BaseInput)) {
    return 0;
  }
  return boost::get<BaseInput>(in).blockIndex;
}

bool check_inputs_types_supported(const TransactionPrefix& tx) {
  for (const auto& in : tx.inputs) {
    const auto& inputType = in.type();
    if (inputType == typeid(MultisignatureInput)) {
      if (tx.version < TRANSACTION_VERSION_2) {
        return false;
      }
    } else if (in.type() != typeid(KeyInput) && in.type() != typeid(MultisignatureInput)) {
      return false;
    }
  }

  return true;
}

bool check_outs_valid(const TransactionPrefix& tx, std::string* error) {
  for (const TransactionOutput& out : tx.outputs) {
    if (out.target.type() == typeid(KeyOutput)) {
      if (out.amount == 0) {
        if (error) {
          *error = "Zero amount ouput";
        }
        return false;
      }

      if (!check_key(boost::get<KeyOutput>(out.target).key)) {
        if (error) {
          *error = "Output with invalid key";
        }
        return false;
      }
    } else if (out.target.type() == typeid(MultisignatureOutput)) {
      if (tx.version < TRANSACTION_VERSION_2) {
        *error = "Transaction contains multisignature output but its version is less than 2";
        return false;
      }

      const MultisignatureOutput& multisignatureOutput = ::boost::get<MultisignatureOutput>(out.target);
      if (multisignatureOutput.requiredSignatureCount > multisignatureOutput.keys.size()) {
        if (error) {
          *error = "Multisignature output with invalid required signature count";
        }
        return false;
      }
      for (const PublicKey& key : multisignatureOutput.keys) {
        if (!check_key(key)) {
          if (error) {
            *error = "Multisignature output with invalid public key";
          }
          return false;
        }
      }
    } else {
      if (error) {
        *error = "Output with invalid type";
      }
      return false;
    }
  }

  return true;
}

bool checkMultisignatureInputsDiff(const TransactionPrefix& tx) {
  std::set<std::pair<uint64_t, uint32_t>> inputsUsage;
  for (const auto& inv : tx.inputs) {
    if (inv.type() == typeid(MultisignatureInput)) {
      const MultisignatureInput& in = ::boost::get<MultisignatureInput>(inv);
      if (!inputsUsage.insert(std::make_pair(in.amount, in.outputIndex)).second) {
        return false;
      }
    }
  }
  return true;
}

bool check_money_overflow(const TransactionPrefix &tx) {
  return check_inputs_overflow(tx) && check_outs_overflow(tx);
}

bool check_inputs_overflow(const TransactionPrefix &tx) {
  uint64_t money = 0;

  for (const auto &in : tx.inputs) {
    uint64_t amount = 0;

    if (in.type() == typeid(KeyInput)) {
      amount = boost::get<KeyInput>(in).amount;
    } else if (in.type() == typeid(MultisignatureInput)) {
      amount = boost::get<MultisignatureInput>(in).amount;
      if (boost::get<MultisignatureInput>(in).term != 0) {
        uint64_t hi;
        uint64_t lo = mul128(amount, CryptoNote::parameters::DEPOSIT_MAX_TOTAL_RATE, &hi);
        uint64_t maxInterestHi;
        uint64_t maxInterestLo;
        div128_32(hi, lo, 100, &maxInterestHi, &maxInterestLo);
        if (maxInterestHi > 0) {
          return false;
        }

        if (amount > std::numeric_limits<uint64_t>::max() - maxInterestLo) {
          return false;
        }

        amount += maxInterestLo;
      }
    }

    if (money > amount + money)
      return false;

    money += amount;
  }
  return true;
}

bool check_outs_overflow(const TransactionPrefix& tx) {
  uint64_t money = 0;
  for (const auto& o : tx.outputs) {
    if (money > o.amount + money)
      return false;
    money += o.amount;
  }
  return true;
}

uint64_t get_outs_money_amount(const Transaction& tx) {
  uint64_t outputs_amount = 0;
  for (const auto& o : tx.outputs) {
    outputs_amount += o.amount;
  }
  return outputs_amount;
}

std::string short_hash_str(const Hash& h) {
  std::string res = Common::podToHex(h);

  if (res.size() == 64) {
    auto erased_pos = res.erase(8, 48);
    res.insert(8, "....");
  }

  return res;
}

bool is_out_to_acc(const AccountKeys& acc, const KeyOutput& out_key, const KeyDerivation& derivation, size_t keyIndex) {
  PublicKey pk;
  derive_public_key(derivation, keyIndex, acc.address.spendPublicKey, pk);
  return pk == out_key.key;
}

bool is_out_to_acc(const AccountKeys& acc, const KeyOutput& out_key, const PublicKey& tx_pub_key, size_t keyIndex) {
  KeyDerivation derivation;
  generate_key_derivation(tx_pub_key, acc.viewSecretKey, derivation);
  return is_out_to_acc(acc, out_key, derivation, keyIndex);
}

bool lookup_acc_outs(const AccountKeys& acc, const Transaction& tx, std::vector<size_t>& outs, uint64_t& money_transfered) {
  PublicKey transactionPublicKey = getTransactionPublicKeyFromExtra(tx.extra);
  if (transactionPublicKey == NULL_PUBLIC_KEY)
    return false;
  return lookup_acc_outs(acc, tx, transactionPublicKey, outs, money_transfered);
}

bool lookup_acc_outs(const AccountKeys& acc, const Transaction& tx, const PublicKey& tx_pub_key, std::vector<size_t>& outs, uint64_t& money_transfered) {
  money_transfered = 0;
  size_t keyIndex = 0;
  size_t outputIndex = 0;

  KeyDerivation derivation;
  generate_key_derivation(tx_pub_key, acc.viewSecretKey, derivation);

  for (const TransactionOutput& o : tx.outputs) {
    assert(o.target.type() == typeid(KeyOutput) || o.target.type() == typeid(MultisignatureOutput));
    if (o.target.type() == typeid(KeyOutput)) {
      if (is_out_to_acc(acc, boost::get<KeyOutput>(o.target), derivation, keyIndex)) {
        outs.push_back(outputIndex);
        money_transfered += o.amount;
      }

      ++keyIndex;
    } else if (o.target.type() == typeid(MultisignatureOutput)) {
      keyIndex += boost::get<MultisignatureOutput>(o.target).keys.size();
    }

    ++outputIndex;
  }
  return true;
}

bool get_block_hashing_blob(const Block& b, BinaryArray& ba) {
  if (!toBinaryArray(static_cast<const BlockHeader&>(b), ba)) {
    return false;
  }

  Hash treeRootHash = get_tx_tree_hash(b);
  ba.insert(ba.end(), treeRootHash.data, treeRootHash.data + 32);
  auto transactionCount = asBinaryArray(Tools::get_varint_data(b.transactionHashes.size() + 1));
  ba.insert(ba.end(), transactionCount.begin(), transactionCount.end());
  return true;
}

bool get_block_hash(const Block& b, Hash& res) {
  BinaryArray ba;
  if (!get_block_hashing_blob(b, ba)) {
    return false;
  }

  return getObjectHash(ba, res);
}

Hash get_block_hash(const Block& b) {
  Hash p = NULL_HASH;
  get_block_hash(b, p);
  return p;
}

bool get_aux_block_header_hash(const Block& b, Hash& res) {
  BinaryArray blob;
  if (!get_block_hashing_blob(b, blob)) {
    return false;
  }

  return getObjectHash(blob, res);
}

bool get_block_longhash(cn_context &context, const Block& b, Hash& res) {
  BinaryArray bd;
  if (!get_block_hashing_blob(b, bd)) {
    return false;
  }

  cn_slow_hash(context, bd.data(), bd.size(), res);
  return true;
}

std::vector<uint32_t> relative_output_offsets_to_absolute(const std::vector<uint32_t>& off) {
  std::vector<uint32_t> res = off;
  for (size_t i = 1; i < res.size(); i++)
    res[i] += res[i - 1];
  return res;
}

std::vector<uint32_t> absolute_output_offsets_to_relative(const std::vector<uint32_t>& off) {
  std::vector<uint32_t> res = off;
  if (!off.size())
    return res;
  std::sort(res.begin(), res.end());//just to be sure, actually it is already should be sorted
  for (size_t i = res.size() - 1; i != 0; i--)
    res[i] -= res[i - 1];

  return res;
}

void get_tx_tree_hash(const std::vector<Hash>& tx_hashes, Hash& h) {
  tree_hash(tx_hashes.data(), tx_hashes.size(), h);
}

Hash get_tx_tree_hash(const std::vector<Hash>& tx_hashes) {
  Hash h = NULL_HASH;
  get_tx_tree_hash(tx_hashes, h);
  return h;
}

Hash get_tx_tree_hash(const Block& b) {
  std::vector<Hash> txs_ids;
  Hash h = NULL_HASH;
  getObjectHash(b.baseTransaction, h);
  txs_ids.push_back(h);
  for (auto& th : b.transactionHashes) {
    txs_ids.push_back(th);
  }
  return get_tx_tree_hash(txs_ids);
}

#if SQ_ADD_ALIAS
bool validate_alias_name(const std::string& al, std::string& err)
{
	char echar[50];
	if (al.size() > MAX_ALIAS_LEN)
	{
		err = "Too long alias name, please use name no longer than 255.";
		return false;
	}
	/*allowed symbols "0-9","a-z", "-", "." */
	static bool alphabet[256] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	for (const auto ch : al)
	{
		if (!(alphabet[static_cast<unsigned char>(ch)]))
		{
			sprintf(echar, "Wrong character in alias: %c.", ch);
			err = echar;
			return false;
		}
	}
	return true;
}
//---------------------------------------------------------------
Crypto::Hash get_blob_hash(const std::string& blob)
{
	Crypto::Hash h = NULL_HASH;
	cn_fast_hash(blob.data(), blob.size(), h);
	return h;
}
//---------------------------------------------------------------
bool sign_update_alias(alias_info& ai, const Crypto::PublicKey& pkey, const Crypto::SecretKey& skey, std::string& err)
{
	std::string buf;
	err.clear();
	bool r = make_tx_extra_alias_entry(buf, ai, err, true);
	if (!r)
	{
		err += ",failed to make_tx_extra_alias_entry";
	}
	Crypto::generate_signature(get_blob_hash(buf), pkey, skey, ai.m_sign);
	return true;
}
//---------------------------------------------------------------
bool make_tx_extra_alias_entry(std::string& buff, const alias_info& alinfo, std::string& err, bool make_buff_to_sign)
{
	err.clear();
	if (alinfo.m_alias.size() == 0)
	{
		err = "alias cant be empty";
		return false;
	}
	if (alinfo.m_alias.size() <= std::numeric_limits<uint8_t>::max())
	{
		err = "alias is too big size";
		return false;
	}
	if (alinfo.m_text_comment.size() <= std::numeric_limits<uint8_t>::max())
	{
		err = "alias comment is too big size";
		return false;
	}
	buff.resize(3);
	buff[0] = TX_EXTRA_TAG_ALIAS;
	buff[1] = 0;
	buff[2] = static_cast<uint8_t>(alinfo.m_alias.size());
	buff += alinfo.m_alias;
	Common::apped_pod_to_strbuff(buff, alinfo.m_address.spendPublicKey);
	Common::apped_pod_to_strbuff(buff, alinfo.m_address.viewPublicKey);
	if (alinfo.m_view_key != NULL_SECRET_KEY)
	{
		buff[1] |= TX_EXTRA_TAG_ALIAS_FLAGS_ADDR_WITH_TRACK;
		Common::apped_pod_to_strbuff(buff, alinfo.m_view_key);
	}
	buff.resize(buff.size() + 1);
	buff.back() = static_cast<uint8_t>(alinfo.m_text_comment.size());
	buff += alinfo.m_text_comment;
	if (!make_buff_to_sign && alinfo.m_sign != NULL_SIG)
	{
		buff[1] |= TX_EXTRA_TAG_ALIAS_FLAGS_OP_UPDATE;
		Common::apped_pod_to_strbuff(buff, alinfo.m_sign);
	}
	return true;
}
//---------------------------------------------------------------
bool add_tx_extra_alias(Transaction& tx, const alias_info& alinfo, std::string& err)
{
	std::string buff;
	bool r = make_tx_extra_alias_entry(buff, alinfo, err);
	if (!r)
		return false;
	tx.extra.resize(tx.extra.size() + buff.size());
	memcpy(&tx.extra[tx.extra.size() - buff.size()], buff.data(), buff.size());
	return true;
}
//---------------------------------------------------------------
#if 0
bool parse_tx_extra_alias_entry(const Transaction& tx, size_t start, alias_info& alinfo, size_t& whole_entry_len, std::string& err)
{
	whole_entry_len = 0;
	size_t i = start;
	/************************************************************************************************************************************************************
	first byte counter+                                                                           first byte counter+
	1 byte          bytes[]             sizeof(crypto::public_key)*2           sizeof(crypto::secret_key)         bytes[]                 sizeof(crypto::signature)
	|--flags--|c---alias name----|--------- account public address --------|[----account tracking key----]|[c--- text comment ---][----- signature(poof of alias owning) ------]

	************************************************************************************************************************************************************/
	CHECK_AND_ASSERT_MES(tx.extra.size() - 1 - i >= sizeof(crypto::public_key) * 2 + 1, false, "Failed to parse transaction extra (TX_EXTRA_TAG_ALIAS have not enough bytes) in tx " << get_transaction_hash(tx));
	++i;
	uint8_t alias_flags = tx.extra[i];
	++i;
	uint8_t alias_name_len = tx.extra[i];
	CHECK_AND_ASSERT_MES(tx.extra.size() - 1 - i >= tx.extra[i] + sizeof(crypto::public_key) * 2, false, "Failed to parse transaction extra (TX_EXTRA_TAG_ALIAS have wrong name bytes counter) in tx " << get_transaction_hash(tx));

	alinfo.m_alias.assign((const char*)&tx.extra[i + 1], static_cast<size_t>(alias_name_len));
	i += tx.extra[i] + 1;
	alinfo.m_address.m_spend_public_key = *reinterpret_cast<const crypto::public_key*>(&tx.extra[i]);
	i += sizeof(const crypto::public_key);
	alinfo.m_address.m_view_public_key = *reinterpret_cast<const crypto::public_key*>(&tx.extra[i]);
	i += sizeof(const crypto::public_key);
	if (alias_flags&TX_EXTRA_TAG_ALIAS_FLAGS_ADDR_WITH_TRACK)
	{//address aliased with tracking key
		CHECK_AND_ASSERT_MES(tx.extra.size() - i >= sizeof(crypto::secret_key), false, "Failed to parse transaction extra (TX_EXTRA_TAG_ALIAS have not enough bytes) in tx " << get_transaction_hash(tx));
		alinfo.m_view_key = *reinterpret_cast<const crypto::secret_key*>(&tx.extra[i]);
		i += sizeof(const crypto::secret_key);
	}
	uint8_t comment_len = tx.extra[i];
	if (comment_len)
	{
		CHECK_AND_ASSERT_MES(tx.extra.size() - i >= tx.extra[i], false, "Failed to parse transaction extra (TX_EXTRA_TAG_ALIAS have not enough bytes) in tx " << get_transaction_hash(tx));
		alinfo.m_text_comment.assign((const char*)&tx.extra[i + 1], static_cast<size_t>(tx.extra[i]));
		i += tx.extra[i] + 1;
	}
	else
	{
		i++;
	}
	if (alias_flags&TX_EXTRA_TAG_ALIAS_FLAGS_OP_UPDATE)
	{
		CHECK_AND_ASSERT_MES(tx.extra.size() - i >= sizeof(crypto::secret_key), false, "Failed to parse transaction extra (TX_EXTRA_TAG_ALIAS have not enough bytes) in tx " << get_transaction_hash(tx));
		alinfo.m_sign = *reinterpret_cast<const crypto::signature*>(&tx.extra[i]);
		i += sizeof(const crypto::signature);
	}
	whole_entry_len = i - start;
	return true;
}

//---------------------------------------------------------------
//check make_alias_tx is valid or not
//make sure !is_coinbase(tx) && e.m_alias.m_alias.size() before called
//return true: tx is not make_alias_tx, or make_alias_tx is ok, should be accepted.
//return false: make_alias_tx is invalid, should be rejected
//---------------------------------------------------------------
bool check_make_alias_tx(const Transaction& tx, tx_extra_info &e, std::string& err)
{
	//not make_alias_tx, we ignore
	if (is_coinbase(tx) || e.m_alias.m_alias.size() == 0) return true;

	//1. at least 100 DNC fee for miner
	CHECK_AND_ASSERT_MES(get_tx_fee(tx) >= MAKE_ALIAS_MINIMUM_FEE, false, "make_alias_tx: not enough fee to make alias: " << e.m_alias.m_alias << ", at least " << MAKE_ALIAS_MINIMUM_FEE << " DNC, tx: " << get_transaction_hash(tx));

	CryptoNote::AccountKeys acc = boost::value_initialized<CryptoNote::AccountKeys>();
	std::vector<size_t> outs;
	uint64_t money_transfered = 0;

	//2. target address is CURRENCY_DONATIONS_ADDRESS
	bool r = CryptoNote::get_account_address_from_str(acc.address, CURRENCY_DONATIONS_ADDRESS);
	CHECK_AND_ASSERT_MES(r, false, "make_alias_tx: failed to get_account_address_from_str from CURRENCY_DONATIONS_ADDRESS");

	r = string_tools::parse_tpod_from_hex_string(CURRENCY_DONATIONS_ADDRESS_TRACKING_KEY, acc.m_view_secret_key);
	CHECK_AND_ASSERT_MES(r, false, "make_alias_tx:failed to parse_tpod_from_hex_string from CURRENCY_DONATIONS_ADDRESS_TRACKING_KEY");

	r = (e.m_tx_pub_key != CryptoNote::null_pkey);
	CHECK_AND_ASSERT_MES(r, false, "make_alias_tx: transaction extra no output address found");

	for (int i = 0; i < tx.vout.size(); i++)
	{
		if (tx.vout[i].target.type() != typeid(txout_to_key)) continue;
		if (!is_out_to_acc(acc, boost::get<txout_to_key>(tx.vout[i].target), e.m_tx_pub_key, i)) continue;
		if (tx.vout[i].amount < COIN) continue;
		money_transfered = tx.vout[i].amount;
		break;
	}

	//3. at least 1 DNC sent to CURRENCY_DONATIONS_ADDRESS 
	r = money_transfered >= COIN;
	CHECK_AND_ASSERT_MES(r, false, "make_alias_tx: can't find any ouput to CURRENCY_DONATIONS_ADDRESS, invalid make alias tx, alias: " << e.m_alias.m_alias << ", tx: " << get_transaction_hash(tx));

	return r;
}
//---------------------------------------------------------------
bool parse_and_validate_tx_extra(const Transaction& tx, Crypto::PublicKey& tx_pub_key, std::string& err)
{
	tx_extra_info e = AUTO_VAL_INIT(e);
	bool r = parse_and_validate_tx_extra(tx, e);
	tx_pub_key = e.m_tx_pub_key;
	return r;
}
//---------------------------------------------------------------
bool parse_and_validate_tx_extra(const Transaction& tx, tx_extra_info& extra, std::string& err)
{
	extra.m_tx_pub_key = null_pkey;
	bool padding_started = false; //let the padding goes only at the end
	bool tx_extra_tag_pubkey_found = false;
	bool tx_extra_user_data_found = false;
	bool tx_alias_found = false;
	for (size_t i = 0; i != tx.extra.size();)
	{
		if (padding_started)
		{
			CHECK_AND_ASSERT_MES(!tx.extra[i], false, "Failed to parse transaction extra (not 0 after padding) in tx " << get_transaction_hash(tx));
		}
		else if (tx.extra[i] == TX_EXTRA_TAG_PUBKEY)
		{
			CHECK_AND_ASSERT_MES(sizeof(crypto::public_key) <= tx.extra.size() - 1 - i, false, "Failed to parse transaction extra (TX_EXTRA_TAG_PUBKEY have not enough bytes) in tx " << get_transaction_hash(tx));
			CHECK_AND_ASSERT_MES(!tx_extra_tag_pubkey_found, false, "Failed to parse transaction extra (duplicate TX_EXTRA_TAG_PUBKEY entry) in tx " << get_transaction_hash(tx));
			extra.m_tx_pub_key = *reinterpret_cast<const crypto::public_key*>(&tx.extra[i + 1]);
			i += 1 + sizeof(crypto::public_key);
			tx_extra_tag_pubkey_found = true;
			continue;
		}
		else if (tx.extra[i] == TX_EXTRA_TAG_USER_DATA)
		{
			//CHECK_AND_ASSERT_MES(is_coinbase(tx), false, "Failed to parse transaction extra (TX_EXTRA_NONCE can be only in coinbase) in tx " << get_transaction_hash(tx));
			CHECK_AND_ASSERT_MES(!tx_extra_user_data_found, false, "Failed to parse transaction extra (duplicate TX_EXTRA_NONCE entry) in tx " << get_transaction_hash(tx));
			CHECK_AND_ASSERT_MES(tx.extra.size() - 1 - i >= 1, false, "Failed to parse transaction extra (TX_EXTRA_NONCE have not enough bytes) in tx " << get_transaction_hash(tx));
			++i;
			CHECK_AND_ASSERT_MES(tx.extra.size() - 1 - i >= tx.extra[i], false, "Failed to parse transaction extra (TX_EXTRA_NONCE have wrong bytes counter) in tx " << get_transaction_hash(tx));
			tx_extra_user_data_found = true;
			if (tx.extra[i])
				extra.m_user_data_blob.assign(reinterpret_cast<const char*>(&tx.extra[i + 1]), static_cast<size_t>(tx.extra[i]));
			i += tx.extra[i];//actually don't need to extract it now, just skip
		}
		else if (tx.extra[i] == TX_EXTRA_TAG_ALIAS)
		{
			//CHECK_AND_ASSERT_MES(is_coinbase(tx), false, "Failed to parse transaction extra (TX_EXTRA_TAG_ALIAS can be only in coinbase) in tx " << get_transaction_hash(tx));
			CHECK_AND_ASSERT_MES(!tx_alias_found, false, "Failed to parse transaction extra (duplicate TX_EXTRA_TAG_ALIAS entry) in tx " << get_transaction_hash(tx));
			size_t aliac_entry_len = 0;
			if (!parse_tx_extra_alias_entry(tx, i, extra.m_alias, aliac_entry_len))
				return false;

			tx_alias_found = true;
			i += aliac_entry_len - 1;
		}
		else if (!tx.extra[i])
		{
			padding_started = true;
			continue;
		}
		++i;
	}

	CHECK_AND_ASSERT_MES(check_make_alias_tx(tx, extra), false, "make_alias_tx error in tx " << get_transaction_hash(tx));
	return true;
}
#endif
#endif
}
