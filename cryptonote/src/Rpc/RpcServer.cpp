// Copyright (c) 2011-2016 The Cryptonote developers
// Copyright (c) 2014-2016 XDN developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "RpcServer.h"

#include <future>
#include <unordered_map>

// CryptoNote
#include "Common/StringTools.h"
#include "CryptoNoteCore/CryptoNoteTools.h"
#include "CryptoNoteCore/Core.h"
#include "CryptoNoteCore/IBlock.h"
#include "CryptoNoteCore/Miner.h"
#include "CryptoNoteCore/TransactionExtra.h"

#include "CryptoNoteProtocol/ICryptoNoteProtocolQuery.h"

#include "P2p/NetNode.h"

#include "CoreRpcServerErrorCodes.h"
#include "JsonRpc.h"


#undef ERROR
#define COMMAND_RPC_GET_BLOCKS_BY_HEIGHTS_MAX_COUNT     100

using namespace Logging;
using namespace Crypto;
using namespace Common;

namespace CryptoNote {

namespace {

template <typename Command>
RpcServer::HandlerFunction binMethod(bool (RpcServer::*handler)(typename Command::request const&, typename Command::response&)) {
  return [handler](RpcServer* obj, const HttpRequest& request, HttpResponse& response) {

    boost::value_initialized<typename Command::request> req;
    boost::value_initialized<typename Command::response> res;

    if (!loadFromBinaryKeyValue(static_cast<typename Command::request&>(req), request.getBody())) {
      return false;
    }

    bool result = (obj->*handler)(req, res);
    response.setBody(storeToBinaryKeyValue(res.data()));
    return result;
  };
}

template <typename Command>
RpcServer::HandlerFunction jsonMethod(bool (RpcServer::*handler)(typename Command::request const&, typename Command::response&)) {
  return [handler](RpcServer* obj, const HttpRequest& request, HttpResponse& response) {

    boost::value_initialized<typename Command::request> req;
    boost::value_initialized<typename Command::response> res;

    if (!loadFromJson(static_cast<typename Command::request&>(req), request.getBody())) {
      return false;
    }

    bool result = (obj->*handler)(req, res);
    response.setBody(storeToJson(res.data()));
    return result;
  };
}

}
  
std::unordered_map<std::string, RpcServer::RpcHandler<RpcServer::HandlerFunction>> RpcServer::s_handlers = {
  
  // binary handlers
  { "/getblocks.bin", { binMethod<COMMAND_RPC_GET_BLOCKS_FAST>(&RpcServer::on_get_blocks), false } },
  { "/queryblocks.bin", { binMethod<COMMAND_RPC_QUERY_BLOCKS>(&RpcServer::on_query_blocks), false } },
  { "/queryblockslite.bin", { binMethod<COMMAND_RPC_QUERY_BLOCKS_LITE>(&RpcServer::on_query_blocks_lite), false } },
  { "/get_o_indexes.bin", { binMethod<COMMAND_RPC_GET_TX_GLOBAL_OUTPUTS_INDEXES>(&RpcServer::on_get_indexes), false } },
  { "/getrandom_outs.bin", { binMethod<COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS>(&RpcServer::on_get_random_outs), false } },
  { "/get_pool_changes.bin", { binMethod<COMMAND_RPC_GET_POOL_CHANGES>(&RpcServer::onGetPoolChanges), false } },
  { "/get_pool_changes_lite.bin", { binMethod<COMMAND_RPC_GET_POOL_CHANGES_LITE>(&RpcServer::onGetPoolChangesLite), false } },

  //dnc1 handlers
  //{ "/set_maintainers_info.bin", { binMethod<COMMAND_RPC_SET_MAINTAINERS_INFO>(&RpcServer::on_set_maintainers_info), false } },
  //{ "/get_tx_pool.bin", { binMethod<COMMAND_RPC_GET_TX_POOL>(&RpcServer::on_get_tx_pool), false } },

  { "/get_transactions", { jsonMethod<COMMAND_RPC_GET_TRANSACTIONS_JSON>(&RpcServer::on_get_transactions_json), false } },
  { "/get_tx_pool", { jsonMethod<COMMAND_RPC_GET_TX_POOL_JSON>(&RpcServer::on_get_tx_pool_json), false } },
  { "/get_blocks_by_heights", { jsonMethod<COMMAND_RPC_GET_BLOCKS_BY_HEIGHTS>(&RpcServer::on_get_blocks_by_heights), false } },
  
  // json handlers
  { "/getinfo", { jsonMethod<COMMAND_RPC_GET_INFO>(&RpcServer::on_get_info), true } },
  { "/getheight", { jsonMethod<COMMAND_RPC_GET_HEIGHT>(&RpcServer::on_get_height), true } },
  { "/gettransactions", { jsonMethod<COMMAND_RPC_GET_TRANSACTIONS>(&RpcServer::on_get_transactions), false } },
  { "/sendrawtransaction", { jsonMethod<COMMAND_RPC_SEND_RAW_TX>(&RpcServer::on_send_raw_tx), false } },
  { "/start_mining", { jsonMethod<COMMAND_RPC_START_MINING>(&RpcServer::on_start_mining), false } },
  { "/stop_mining", { jsonMethod<COMMAND_RPC_STOP_MINING>(&RpcServer::on_stop_mining), false } },
  { "/stop_daemon", { jsonMethod<COMMAND_RPC_STOP_DAEMON>(&RpcServer::on_stop_daemon), true } },

  // json rpc
  { "/json_rpc", { std::bind(&RpcServer::processJsonRpcRequest, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), true } }
};

RpcServer::RpcServer(System::Dispatcher& dispatcher, Logging::ILogger& log, core& c, NodeServer& p2p, const ICryptoNoteProtocolQuery& protocolQuery) :
  HttpServer(dispatcher, log), logger(log, "RpcServer"), m_core(c), m_p2p(p2p), m_protocolQuery(protocolQuery) {
}

void RpcServer::processRequest(const HttpRequest& request, HttpResponse& response) {
  auto url = request.getUrl();

  auto it = s_handlers.find(url);
  if (it == s_handlers.end()) {
    response.setStatus(HttpResponse::STATUS_404);
    return;
  }

  if (!it->second.allowBusyCore && !isCoreReady()) {
    response.setStatus(HttpResponse::STATUS_500);
    response.setBody("Core is busy");
    return;
  }

  it->second.handler(this, request, response);
}

bool RpcServer::processJsonRpcRequest(const HttpRequest& request, HttpResponse& response) {

  using namespace JsonRpc;

  response.addHeader("Content-Type", "application/json");

  JsonRpcRequest jsonRequest;
  JsonRpcResponse jsonResponse;

  try {
    logger(TRACE) << "JSON-RPC request: " << request.getBody();
    jsonRequest.parseRequest(request.getBody());
    jsonResponse.setId(jsonRequest.getId()); // copy id

	static std::unordered_map<std::string, RpcServer::RpcHandler<JsonMemberMethod>> jsonRpcHandlers = {
#if SQ_ADD_ALIAS
	  { "getaliasdetails", { makeMemberMethod(&RpcServer::on_get_alias_details), false } },
	  { "getallaliases", { makeMemberMethod(&RpcServer::on_get_all_aliases), false } },
	  { "aliasbyaddress", { makeMemberMethod(&RpcServer::on_alias_by_address), false } },
#endif
      { "getblockcount", { makeMemberMethod(&RpcServer::on_getblockcount), true } },
      { "getblockhash", { makeMemberMethod(&RpcServer::on_getblockhash), false } },
      { "getblocktemplate", { makeMemberMethod(&RpcServer::on_getblocktemplate), false } },
      { "getcurrencyid", { makeMemberMethod(&RpcServer::on_get_currency_id), true } },
      { "submitblock", { makeMemberMethod(&RpcServer::on_submitblock), false } },
      { "getlastblockheader", { makeMemberMethod(&RpcServer::on_get_last_block_header), false } },
      { "getblockheaderbyhash", { makeMemberMethod(&RpcServer::on_get_block_header_by_hash), false } },
	  { "getblockheaderbyheight", { makeMemberMethod(&RpcServer::on_get_block_header_by_height), false } }
    };

	static std::unordered_map<std::string, RpcServer::RpcHandler<JsonMemberMethod>> jsonRpcHandlers_dnc1 = {
#if SQ_ADD_ALIAS
	  { "get_alias_details", { makeMemberMethod(&RpcServer::on_dnc1_get_alias_details), false } },
	  { "get_all_alias_details", { makeMemberMethod(&RpcServer::on_dnc1_get_all_aliases), false } },
	  { "get_alias_by_address", { makeMemberMethod(&RpcServer::on_dnc1_alias_by_address), false } },
	  { "get_addendums", { makeMemberMethod(&RpcServer::on_dnc1_get_addendums), false } },
#endif
	  { "getblockcount", { makeMemberMethod(&RpcServer::on_dnc1_getblockcount), true } },
	  { "getblockhash", { makeMemberMethod(&RpcServer::on_dnc1_getblockhash), false } },
	  { "getblocktemplate", { makeMemberMethod(&RpcServer::on_dnc1_getblocktemplate), false } },
	  { "submitblock", { makeMemberMethod(&RpcServer::on_dnc1_submitblock), false } },
	  { "getlastblockheader", { makeMemberMethod(&RpcServer::on_dnc1_get_last_block_header), false } },
	  { "getblockheaderbyhash", { makeMemberMethod(&RpcServer::on_dnc1_get_block_header_by_hash), false } },
	  { "getblockheaderbyheight", { makeMemberMethod(&RpcServer::on_dnc1_get_block_header_by_height), false } }
	};

	auto it = (jsonRequest.getDnc().compare("2.0")) ? jsonRpcHandlers_dnc1.find(jsonRequest.getMethod()) : jsonRpcHandlers.find(jsonRequest.getMethod());
	auto end = (jsonRequest.getDnc().compare("2.0")) ? jsonRpcHandlers_dnc1.end() : jsonRpcHandlers.end();

	if (it == end) {
		throw JsonRpcError(JsonRpc::errMethodNotFound);
	}

	if (!it->second.allowBusyCore && !isCoreReady()) {
		throw JsonRpcError(CORE_RPC_ERROR_CODE_CORE_BUSY, "Core is busy");
	}

	it->second.handler(this, jsonRequest, jsonResponse);
	
  } catch (const JsonRpcError& err) {
    jsonResponse.setError(err);
  } catch (const std::exception& e) {
    jsonResponse.setError(JsonRpcError(JsonRpc::errInternalError, e.what()));
  }

  response.setBody(jsonResponse.getBody());
  logger(TRACE) << "JSON-RPC response: " << jsonResponse.getBody();
  return true;
}

bool RpcServer::isCoreReady() {
  return m_core.currency().isTestnet() || m_p2p.get_payload_object().isSynchronized();
}

//
// Binary handlers
//

bool RpcServer::on_get_blocks(const COMMAND_RPC_GET_BLOCKS_FAST::request& req, COMMAND_RPC_GET_BLOCKS_FAST::response& res) {
  // TODO code duplication see InProcessNode::doGetNewBlocks()
  if (req.block_ids.empty()) {
    res.status = "Failed";
    return false;
  }

  if (req.block_ids.back() != m_core.getBlockIdByHeight(0)) {
    res.status = "Failed";
    return false;
  }

  uint32_t totalBlockCount;
  uint32_t startBlockIndex;
  std::vector<Crypto::Hash> supplement = m_core.findBlockchainSupplement(req.block_ids, COMMAND_RPC_GET_BLOCKS_FAST_MAX_COUNT, totalBlockCount, startBlockIndex);

  res.current_height = totalBlockCount;
  res.start_height = startBlockIndex;

  for (const auto& blockId : supplement) {
    assert(m_core.have_block(blockId));
    auto completeBlock = m_core.getBlock(blockId);
    assert(completeBlock != nullptr);

    res.blocks.resize(res.blocks.size() + 1);
    res.blocks.back().block = asString(toBinaryArray(completeBlock->getBlock()));

    res.blocks.back().txs.reserve(completeBlock->getTransactionCount());
    for (size_t i = 0; i < completeBlock->getTransactionCount(); ++i) {
      res.blocks.back().txs.push_back(asString(toBinaryArray(completeBlock->getTransaction(i))));
    }
  }

  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_query_blocks(const COMMAND_RPC_QUERY_BLOCKS::request& req, COMMAND_RPC_QUERY_BLOCKS::response& res) {
  uint32_t startHeight;
  uint32_t currentHeight;
  uint32_t fullOffset;

  if (!m_core.queryBlocks(req.block_ids, req.timestamp, startHeight, currentHeight, fullOffset, res.items)) {
    res.status = "Failed to perform query";
    return false;
  }

  res.start_height = startHeight;
  res.current_height = currentHeight;
  res.full_offset = fullOffset;
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_query_blocks_lite(const COMMAND_RPC_QUERY_BLOCKS_LITE::request& req, COMMAND_RPC_QUERY_BLOCKS_LITE::response& res) {
  uint32_t startHeight;
  uint32_t currentHeight;
  uint32_t fullOffset;
  if (!m_core.queryBlocksLite(req.blockIds, req.timestamp, startHeight, currentHeight, fullOffset, res.items)) {
    res.status = "Failed to perform query";
    return false;
  }

  res.startHeight = startHeight;
  res.currentHeight = currentHeight;
  res.fullOffset = fullOffset;
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_get_indexes(const COMMAND_RPC_GET_TX_GLOBAL_OUTPUTS_INDEXES::request& req, COMMAND_RPC_GET_TX_GLOBAL_OUTPUTS_INDEXES::response& res) {
  std::vector<uint32_t> outputIndexes;
  if (!m_core.get_tx_outputs_gindexs(req.txid, outputIndexes)) {
    res.status = "Failed";
    return true;
  }

  res.o_indexes.assign(outputIndexes.begin(), outputIndexes.end());
  res.status = CORE_RPC_STATUS_OK;
  logger(TRACE) << "COMMAND_RPC_GET_TX_GLOBAL_OUTPUTS_INDEXES: [" << res.o_indexes.size() << "]";
  return true;
}

bool RpcServer::on_get_random_outs(const COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::request& req, COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::response& res) {
  res.status = "Failed";
  if (!m_core.get_random_outs_for_amounts(req, res)) {
    return true;
  }

  res.status = CORE_RPC_STATUS_OK;

  std::stringstream ss;
  typedef COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::outs_for_amount outs_for_amount;
  typedef COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::out_entry out_entry;

  std::for_each(res.outs.begin(), res.outs.end(), [&](outs_for_amount& ofa)  {
    ss << "[" << ofa.amount << "]:";

    assert(ofa.outs.size() && "internal error: ofa.outs.size() is empty");

    std::for_each(ofa.outs.begin(), ofa.outs.end(), [&](out_entry& oe)
    {
      ss << oe.global_amount_index << " ";
    });
    ss << ENDL;
  });
  std::string s = ss.str();
  logger(TRACE) << "COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS: " << ENDL << s;
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::onGetPoolChanges(const COMMAND_RPC_GET_POOL_CHANGES::request& req, COMMAND_RPC_GET_POOL_CHANGES::response& rsp) {
  rsp.status = CORE_RPC_STATUS_OK;
  std::vector<CryptoNote::Transaction> addedTransactions;
  rsp.isTailBlockActual = m_core.getPoolChanges(req.tailBlockId, req.knownTxsIds, addedTransactions, rsp.deletedTxsIds);
  for (auto& tx : addedTransactions) {
    BinaryArray txBlob;
    if (!toBinaryArray(tx, txBlob)) {
      rsp.status = "Internal error";
      break;;
    }

    rsp.addedTxs.emplace_back(std::move(txBlob));
  }
  return true;
}


bool RpcServer::onGetPoolChangesLite(const COMMAND_RPC_GET_POOL_CHANGES_LITE::request& req, COMMAND_RPC_GET_POOL_CHANGES_LITE::response& res) {
  res.status = CORE_RPC_STATUS_OK;
  res.isTailBlockActual = m_core.getPoolChangesLite(req.tailBlockId, req.knownTxsIds, res.addedTxs, res.deletedTxsIds);

  return true;
}
//dnc1 handlers
//bool RpcServer::on_set_maintainers_info(const COMMAND_RPC_SET_MAINTAINERS_INFO::request& req, COMMAND_RPC_SET_MAINTAINERS_INFO::response& rsp){
	//if (!m_p2p.handle_maintainers_entry(req))
	//{
	//	res.status = "Failed to get call handle_maintainers_entry()";
	//	return true;
	//}

	//res.status = CORE_RPC_STATUS_OK;
	//return true;
//}
//bool RpcServer::on_get_tx_pool(const COMMAND_RPC_GET_TX_POOL::request& req, COMMAND_RPC_GET_TX_POOL::response& rsp){
	//std::list<transaction> txs;
	//if (!m_core.get_pool_transactions(txs))
	//{
	//	res.status = "Failed to call get_pool_transactions()";
	//	return true;
	//}

	//for (auto& tx : txs)
	//{
	//	res.txs.push_back(t_serializable_object_to_blob(tx));
	//}
	//res.status = CORE_RPC_STATUS_OK;
	//LOG_PRINT_L2("COMMAND_RPC_GET_TX_POOL: [" << res.txs.size() << "]");
	//return true;
//}

bool RpcServer::on_get_transactions_json(const COMMAND_RPC_GET_TRANSACTIONS_JSON::request& req, COMMAND_RPC_GET_TRANSACTIONS_JSON::response& res){
	std::vector<Crypto::Hash> vh;
	BOOST_FOREACH(const auto& tx_hex_str, req.txs_hashes)
	{
		//--blobdata b;
		//--if (!string_tools::parse_hexstr_to_binbuff(tx_hex_str, b))
		std::string b;
		if (!parse_hexstr_to_binbuff(tx_hex_str, b))
		{
			res.status = "Failed to parse hex representation of transaction hash";
			return true;
		}
		//--if (b.size() != sizeof(Crypto::Hash))
		if (b.size() != sizeof(Crypto::Hash))
		{
			res.status = "Failed, size of data mismatch";
		}
		vh.push_back(*reinterpret_cast<const Crypto::Hash*>(b.c_str()));
	}

	//--bool r = m_core.get_transactions(vh, txs, missed_txs);
	std::list<Crypto::Hash> missed_txs;
	std::list<CryptoNote::Transaction> txs;
	m_core.getTransactions(vh, txs, missed_txs,true);
	BOOST_FOREACH(auto& tx, txs)
	{
		//--blobdata json_str = currency::obj_to_json_str(tx);
		//--blobdata blob = t_serializable_object_to_blob(tx);
		auto blob		= toBinaryArray(tx);
		auto anoStr		= storeToJson(tx);
		//toString(tx, anoStr);
		res.txs_as_hex.push_back(anoStr);
		res.sizes.push_back(blob.size());
	}

	BOOST_FOREACH(const auto& miss_tx, missed_txs)
	{
		//--res.missed_tx.push_back(string_tools::pod_to_hex(miss_tx));
		res.missed_tx.push_back(Common::podToHex(miss_tx));
	}
	res.status = CORE_RPC_STATUS_OK;
	//LOG_PRINT_L2("COMMAND_RPC_GET_TRANSACTIONS: [ txs_as_hex = " << res.txs_as_hex.size() << " | missed_tx = " << res.missed_tx.size() << "]");
	return true;
}
bool RpcServer::on_get_tx_pool_json(const COMMAND_RPC_GET_TX_POOL_JSON::request& req, COMMAND_RPC_GET_TX_POOL_JSON::response& res){
	auto txs = m_core.getPoolTransactions();
	for (auto& tx : txs)
	{
		auto auStr = storeToJson(tx);
		res.txs.push_back(auStr);
	
		//tx size
		auto blob = toBinaryArray(tx);
		res.sizes.push_back(blob.size());

		//tx hash
		auto hash = getObjectHash(tx);
		res.hashs.push_back(Common::podToHex(hash));
	}
	res.status = CORE_RPC_STATUS_OK;
	return true;
}
bool RpcServer::on_get_blocks_by_heights(const COMMAND_RPC_GET_BLOCKS_BY_HEIGHTS::request& req, COMMAND_RPC_GET_BLOCKS_BY_HEIGHTS::response& res){
	uint64_t count = 0;
	//--uint64_t maxheight = m_core.get_blockchain_storage().get_current_blockchain_height();
	uint64_t maxheight = m_core.get_current_blockchain_height();
	//--std::vector<std::pair<CryptoNote::Block, std::vector<CryptoNote::Transaction>>> bs;
	std::vector<Crypto::Hash> block_ids;

	if (maxheight < req.start_height)
	{
		res.status = "Failed, no such height: " + std::to_string(req.start_height);
		return false;
	}
	count = req.blocks_count >(uint64_t)COMMAND_RPC_GET_BLOCKS_BY_HEIGHTS_MAX_COUNT ? (uint64_t)COMMAND_RPC_GET_BLOCKS_BY_HEIGHTS_MAX_COUNT : req.blocks_count;

	if (maxheight < req.start_height + count && maxheight > req.start_height)
		count = maxheight - req.start_height;

	Crypto::Hash hash;
	for (uint64_t i = 0; i < count; i++)
	{
		//--hash = m_core.get_blockchain_storage().get_block_id_by_height(req.start_height + i);
		hash = m_core.getBlockIdByHeight(req.start_height + i);
		block_ids.push_back(hash);
	}
	std::string buff;
	//--if (!string_tools::parse_hexstr_to_binbuff(req.genesis_hash, buff))
	if (!parse_hexstr_to_binbuff(req.genesis_hash, buff))
	{
		res.status = "Failed to parse hex representation of genesis transaction hash";
		return false;
	}
	hash = *reinterpret_cast<const Crypto::Hash*>(buff.c_str());
	block_ids.push_back(hash);
	//--uint64_t current, start;
	//--if (!m_core.find_blockchain_supplement(block_ids, bs, current, start, count))
	uint32_t current, start;
	auto bs = m_core.findBlockchainSupplement(block_ids, count, current, start);
	if (bs.empty())
	{
		res.status = "Failed";
		return false;
	}

	//BOOST_FOREACH(auto& b, bs)
	//{
	//	res.blocks.resize(res.blocks.size() + 1);
	//	res.blocks.back().block = obj_to_json_str(b.first);

	//	//miner_tx hash
	//	blobdata blob = t_serializable_object_to_blob(b.first.miner_tx);
	//	res.miner_tx_sizes.push_back(blob.size());

	//	//miner_tx size
	//	crypto::hash hash = currency::get_transaction_hash(b.first.miner_tx);
	//	res.miner_tx_hashs.push_back(string_tools::pod_to_hex(hash));

	//	//block head size
	//	blob = t_serializable_object_to_blob(b.first);
	//	uint64_t size = blob.size();

	//	BOOST_FOREACH(auto& t, b.second)
	//	{
	//		res.blocks.back().txs.push_back(obj_to_json_str(t));

	//		//block head size + every tx size
	//		blob = t_serializable_object_to_blob(t);
	//		size += blob.size();

	//		//tx_size
	//		res.tx_sizes.push_back(blob.size());
	//	}
	//	res.block_sizes.push_back(size);
	//}

	for (auto& b : bs)
	{
		CryptoNote::Block blk;
	    m_core.getBlockByHash(b, blk);
				
		res.blocks.resize(res.blocks.size() + 1);
		res.blocks.back().block = Common::podToHex(b);		

		for(auto& tx : blk.transactionHashes)
		{			
			auto hs = Common::podToHex(tx.data);
			res.blocks.back().txs.push_back(hs);
		}

		size_t blksize = 0;
		m_core.getBlockSize(b, blksize);
		size_t tsize = getObjectBinarySize(blk) + blksize - getObjectBinarySize(blk.baseTransaction);
		res.block_sizes.push_back(tsize);
	}

	res.blocks_count = count;
	res.status = CORE_RPC_STATUS_OK;
	return true;
}
//
// JSON handlers
//

bool RpcServer::on_get_info(const COMMAND_RPC_GET_INFO::request& req, COMMAND_RPC_GET_INFO::response& res) {
  res.height = m_core.get_current_blockchain_height();
  res.difficulty = m_core.getNextBlockDifficulty();
  res.tx_count = m_core.get_blockchain_total_transactions() - res.height; //without coinbase
  res.tx_pool_size = m_core.get_pool_transactions_count();
  res.alt_blocks_count = m_core.get_alternative_blocks_count();
  uint64_t total_conn = m_p2p.get_connections_count();
  res.outgoing_connections_count = m_p2p.get_outgoing_connections_count();
  res.incoming_connections_count = total_conn - res.outgoing_connections_count;
  res.white_peerlist_size = m_p2p.getPeerlistManager().get_white_peers_count();
  res.grey_peerlist_size = m_p2p.getPeerlistManager().get_gray_peers_count();
  res.last_known_block_index = std::max(static_cast<uint32_t>(1), m_protocolQuery.getObservedHeight()) - 1;
  res.full_deposit_amount = m_core.fullDepositAmount();
  res.full_deposit_interest = m_core.fullDepositInterest();
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_get_height(const COMMAND_RPC_GET_HEIGHT::request& req, COMMAND_RPC_GET_HEIGHT::response& res) {
  res.height = m_core.get_current_blockchain_height();
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_get_transactions(const COMMAND_RPC_GET_TRANSACTIONS::request& req, COMMAND_RPC_GET_TRANSACTIONS::response& res) {
  std::vector<Hash> vh;
  for (const auto& tx_hex_str : req.txs_hashes) {
    BinaryArray b;
    if (!fromHex(tx_hex_str, b))
    {
      res.status = "Failed to parse hex representation of transaction hash";
      return true;
    }
    if (b.size() != sizeof(Hash))
    {
      res.status = "Failed, size of data mismatch";
    }
    vh.push_back(*reinterpret_cast<const Hash*>(b.data()));
  }
  std::list<Hash> missed_txs;
  std::list<Transaction> txs;
  m_core.getTransactions(vh, txs, missed_txs,true);

  for (auto& tx : txs) {
    res.txs_as_hex.push_back(toHex(toBinaryArray(tx)));
  }

  for (const auto& miss_tx : missed_txs) {
    res.missed_tx.push_back(Common::podToHex(miss_tx));
  }

  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_send_raw_tx(const COMMAND_RPC_SEND_RAW_TX::request& req, COMMAND_RPC_SEND_RAW_TX::response& res) {
  BinaryArray tx_blob;
  if (!fromHex(req.tx_as_hex, tx_blob))
  {
    logger(INFO) << "[on_send_raw_tx]: Failed to parse tx from hexbuff: " << req.tx_as_hex;
    res.status = "Failed";
    return true;
  }

  tx_verification_context tvc = boost::value_initialized<tx_verification_context>();
  if (!m_core.handle_incoming_tx(tx_blob, tvc, false))
  {
    logger(INFO) << "[on_send_raw_tx]: Failed to process tx";
    res.status = "Failed";
    return true;
  }

  if (tvc.m_verifivation_failed)
  {
    logger(INFO) << "[on_send_raw_tx]: tx verification failed";
    res.status = "Failed";
    return true;
  }

  if (!tvc.m_should_be_relayed)
  {
    logger(INFO) << "[on_send_raw_tx]: tx accepted, but not relayed";
    res.status = "Not relayed";
    return true;
  }


  NOTIFY_NEW_TRANSACTIONS::request r;
  r.txs.push_back(asString(tx_blob));
  m_core.get_protocol()->relay_transactions(r);
  //TODO: make sure that tx has reached other nodes here, probably wait to receive reflections from other nodes
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_start_mining(const COMMAND_RPC_START_MINING::request& req, COMMAND_RPC_START_MINING::response& res) {
  AccountPublicAddress adr;
  if (!m_core.currency().parseAccountAddressString(req.miner_address, adr)) {
    res.status = "Failed, wrong address";
    return true;
  }

  if (!m_core.get_miner().start(adr, static_cast<size_t>(req.threads_count))) {
    res.status = "Failed, mining not started";
    return true;
  }

  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_stop_mining(const COMMAND_RPC_STOP_MINING::request& req, COMMAND_RPC_STOP_MINING::response& res) {
  if (!m_core.get_miner().stop()) {
    res.status = "Failed, mining not stopped";
    return true;
  }
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_stop_daemon(const COMMAND_RPC_STOP_DAEMON::request& req, COMMAND_RPC_STOP_DAEMON::response& res) {
  if (m_core.currency().isTestnet()) {
    m_p2p.sendStopSignal();
    res.status = CORE_RPC_STATUS_OK;
  } else {
    res.status = CORE_RPC_ERROR_CODE_INTERNAL_ERROR;
    return false;
  }
  return true;
}

//------------------------------------------------------------------------------------------------------------------------------
// JSON RPC methods
//------------------------------------------------------------------------------------------------------------------------------
bool RpcServer::on_getblockcount(const COMMAND_RPC_GETBLOCKCOUNT::request& req, COMMAND_RPC_GETBLOCKCOUNT::response& res) {
  res.count = m_core.get_current_blockchain_height();
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_getblockhash(const COMMAND_RPC_GETBLOCKHASH::request& req, COMMAND_RPC_GETBLOCKHASH::response& res) {
  if (req.size() != 1) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_WRONG_PARAM, "Wrong parameters, expected height" };
  }

  uint32_t h = static_cast<uint32_t>(req[0]);
  Crypto::Hash blockId = m_core.getBlockIdByHeight(h);
  if (blockId == NULL_HASH) {
    throw JsonRpc::JsonRpcError{ 
      CORE_RPC_ERROR_CODE_TOO_BIG_HEIGHT,
      std::string("To big height: ") + std::to_string(h) + ", current blockchain height = " + std::to_string(m_core.get_current_blockchain_height())
    };
  }

  res = Common::podToHex(blockId);
  return true;
}

namespace {
  uint64_t slow_memmem(void* start_buff, size_t buflen, void* pat, size_t patlen)
  {
    void* buf = start_buff;
    void* end = (char*)buf + buflen - patlen;
    while ((buf = memchr(buf, ((char*)pat)[0], buflen)))
    {
      if (buf>end)
        return 0;
      if (memcmp(buf, pat, patlen) == 0)
        return (char*)buf - (char*)start_buff;
      buf = (char*)buf + 1;
    }
    return 0;
  }
}

bool RpcServer::on_getblocktemplate(const COMMAND_RPC_GETBLOCKTEMPLATE::request& req, COMMAND_RPC_GETBLOCKTEMPLATE::response& res) {
  if (req.reserve_size > TX_EXTRA_NONCE_MAX_COUNT) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_TOO_BIG_RESERVE_SIZE, "To big reserved size, maximum 255" };
  }

  AccountPublicAddress acc = boost::value_initialized<AccountPublicAddress>();

  if (!req.wallet_address.size() || !m_core.currency().parseAccountAddressString(req.wallet_address, acc)) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_WRONG_WALLET_ADDRESS, "Failed to parse wallet address" };
  }

  Block b = boost::value_initialized<Block>();
  CryptoNote::BinaryArray blob_reserve;
  blob_reserve.resize(req.reserve_size, 0);
  if (!m_core.get_block_template(b, acc, res.difficulty, res.height, blob_reserve)) {
    logger(ERROR) << "Failed to create block template";
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Internal error: failed to create block template" };
  }

  BinaryArray block_blob = toBinaryArray(b);
  PublicKey tx_pub_key = CryptoNote::getTransactionPublicKeyFromExtra(b.baseTransaction.extra);
  if (tx_pub_key == NULL_PUBLIC_KEY) {
    logger(ERROR) << "Failed to find tx pub key in coinbase extra";
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Internal error: failed to find tx pub key in coinbase extra" };
  }

  if (0 < req.reserve_size) {
    res.reserved_offset = slow_memmem((void*)block_blob.data(), block_blob.size(), &tx_pub_key, sizeof(tx_pub_key));
    if (!res.reserved_offset) {
      logger(ERROR) << "Failed to find tx pub key in blockblob";
      throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Internal error: failed to create block template" };
    }
    res.reserved_offset += sizeof(tx_pub_key) + 3; //3 bytes: tag for TX_EXTRA_TAG_PUBKEY(1 byte), tag for TX_EXTRA_NONCE(1 byte), counter in TX_EXTRA_NONCE(1 byte)
    if (res.reserved_offset + req.reserve_size > block_blob.size()) {
      logger(ERROR) << "Failed to calculate offset for reserved bytes";
      throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Internal error: failed to create block template" };
    }
  } else {
    res.reserved_offset = 0;
  }

  res.blocktemplate_blob = toHex(block_blob);
  res.status = CORE_RPC_STATUS_OK;

  return true;
}

bool RpcServer::on_get_currency_id(const COMMAND_RPC_GET_CURRENCY_ID::request& /*req*/, COMMAND_RPC_GET_CURRENCY_ID::response& res) {
  Hash currencyId = m_core.currency().genesisBlockHash();
  res.currency_id_blob = Common::podToHex(currencyId);
  return true;
}

bool RpcServer::on_submitblock(const COMMAND_RPC_SUBMITBLOCK::request& req, COMMAND_RPC_SUBMITBLOCK::response& res) {
  if (req.size() != 1) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_WRONG_PARAM, "Wrong param" };
  }

  BinaryArray blockblob;
  if (!fromHex(req[0], blockblob)) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_WRONG_BLOCKBLOB, "Wrong block blob" };
  }

  block_verification_context bvc = boost::value_initialized<block_verification_context>();

  m_core.handle_incoming_block_blob(blockblob, bvc, true, true);

  if (!bvc.m_added_to_main_chain) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_BLOCK_NOT_ACCEPTED, "Block not accepted" };
  }

  res.status = CORE_RPC_STATUS_OK;
  return true;
}


namespace {
  uint64_t get_block_reward(const Block& blk) {
    uint64_t reward = 0;
    for (const TransactionOutput& out : blk.baseTransaction.outputs) {
      reward += out.amount;
    }
    return reward;
  }
}

void RpcServer::fill_block_header_response(const Block& blk, bool orphan_status, uint64_t height, const Hash& hash, block_header_response& responce) {
  responce.major_version = blk.majorVersion;
  responce.minor_version = blk.minorVersion;
  responce.timestamp = blk.timestamp;
  responce.prev_hash = Common::podToHex(blk.previousBlockHash);
  responce.nonce = blk.nonce;
  responce.orphan_status = orphan_status;
  responce.height = height;
  responce.depth = m_core.get_current_blockchain_height() - height - 1;
  responce.hash = Common::podToHex(hash);
  m_core.getBlockDifficulty(static_cast<uint32_t>(height), responce.difficulty);
  responce.reward = get_block_reward(blk);
}

bool RpcServer::on_get_last_block_header(const COMMAND_RPC_GET_LAST_BLOCK_HEADER::request& req, COMMAND_RPC_GET_LAST_BLOCK_HEADER::response& res) {
  uint32_t last_block_height;
  Hash last_block_hash;
  
  m_core.get_blockchain_top(last_block_height, last_block_hash);

  Block last_block;
  if (!m_core.getBlockByHash(last_block_hash, last_block)) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Internal error: can't get last block hash." };
  }
  
  fill_block_header_response(last_block, false, last_block_height, last_block_hash, res.block_header);
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_get_block_header_by_hash(const COMMAND_RPC_GET_BLOCK_HEADER_BY_HASH::request& req, COMMAND_RPC_GET_BLOCK_HEADER_BY_HASH::response& res) {
  Hash block_hash;

  if (!parse_hash256(req.hash, block_hash)) {
    throw JsonRpc::JsonRpcError{
      CORE_RPC_ERROR_CODE_WRONG_PARAM,
      "Failed to parse hex representation of block hash. Hex = " + req.hash + '.' };
  }

  Block blk;
  if (!m_core.getBlockByHash(block_hash, blk)) {
    throw JsonRpc::JsonRpcError{
      CORE_RPC_ERROR_CODE_INTERNAL_ERROR,
      "Internal error: can't get block by hash. Hash = " + req.hash + '.' };
  }

  if (blk.baseTransaction.inputs.front().type() != typeid(BaseInput)) {
    throw JsonRpc::JsonRpcError{
      CORE_RPC_ERROR_CODE_INTERNAL_ERROR,
      "Internal error: coinbase transaction in the block has the wrong type" };
  }

  uint64_t block_height = boost::get<BaseInput>(blk.baseTransaction.inputs.front()).blockIndex;
  fill_block_header_response(blk, false, block_height, block_hash, res.block_header);
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_get_block_header_by_height(const COMMAND_RPC_GET_BLOCK_HEADER_BY_HEIGHT::request& req, COMMAND_RPC_GET_BLOCK_HEADER_BY_HEIGHT::response& res) {
  if (m_core.get_current_blockchain_height() <= req.height) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_TOO_BIG_HEIGHT,
      std::string("To big height: ") + std::to_string(req.height) + ", current blockchain height = " + std::to_string(m_core.get_current_blockchain_height()) };
  }

  Hash block_hash = m_core.getBlockIdByHeight(static_cast<uint32_t>(req.height));
  Block blk;
  if (!m_core.getBlockByHash(block_hash, blk)) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR,
      "Internal error: can't get block by height. Height = " + std::to_string(req.height) + '.' };
  }

  fill_block_header_response(blk, false, req.height, block_hash, res.block_header);
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

#if SQ_ADD_ALIAS
bool RpcServer::on_get_alias_details(const COMMAND_RPC_GET_ALIAS_DETAILS::request& req, COMMAND_RPC_GET_ALIAS_DETAILS::response& res)
{
	alias_info_base aib = boost::value_initialized<alias_info_base>();
	std::string err;
	if (!validate_alias_name(req.alias, err))
	{
		res.status = "Alias have wrong name";
		return false;
	}
	if (!m_core.get_alias_info(req.alias, aib))
	{
		res.status = "Alias not found";
		return true;
	}
	res.alias_details.address = getAccountAddressAsStr(parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX, aib.m_address);
	res.alias_details.comment = aib.m_text_comment;
	if (aib.m_view_key != NULL_SECRET_KEY)
		res.alias_details.tracking_key = Common::podToHex(aib.m_view_key);

	res.status = CORE_RPC_STATUS_OK;
	return true;
}
//------------------------------------------------------------------------------------------------------------------------------
bool RpcServer::on_get_all_aliases(const COMMAND_RPC_GET_ALL_ALIASES::request& req, COMMAND_RPC_GET_ALL_ALIASES::response& res)
{
	std::list<CryptoNote::alias_info> aliases;
	m_core.get_all_aliases(aliases);
	for (auto a : aliases)
	{
		res.aliases.push_back(alias_rpc_details());
		res.aliases.back().alias = a.m_alias;
		res.aliases.back().details.address = getAccountAddressAsStr(parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX, a.m_address);
		res.aliases.back().details.comment = a.m_text_comment;
		if (a.m_view_key != NULL_SECRET_KEY)
			res.aliases.back().details.tracking_key = Common::podToHex(a.m_view_key);
	}
	res.status = CORE_RPC_STATUS_OK;
	return true;
}

bool RpcServer::on_alias_by_address(const COMMAND_RPC_GET_ALIASES_BY_ADDRESS::request& req, COMMAND_RPC_GET_ALIASES_BY_ADDRESS::response& res)
{
	AccountPublicAddress addr = boost::value_initialized<AccountPublicAddress>();
	uint64_t prefix;
	if (!CryptoNote::parseAccountAddressString(prefix, addr, req.addr))
	{
		res.status = CORE_RPC_STATUS_FAILED;
		return false;
	}

	if (prefix != parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX) {
		logger(DEBUGGING) << "Wrong address prefix: " << prefix << ", expected " << parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX;
		res.status = CORE_RPC_STATUS_FAILED;
		return false;
	}
	res.alias = m_core.get_alias_by_address(addr);
	if (res.alias.size())
		res.status = CORE_RPC_STATUS_OK;
	else
		res.status = CORE_RPC_STATUS_NOT_FOUND;
	return true;
}

bool RpcServer::on_dnc1_get_alias_details(const COMMAND_RPC_GET_ALIAS_DETAILS::request& req, COMMAND_RPC_GET_ALIAS_DETAILS::response& res) {
	return on_get_alias_details(req,res);
}
bool RpcServer::on_dnc1_get_all_aliases(const COMMAND_RPC_GET_ALL_ALIASES::request& req, COMMAND_RPC_GET_ALL_ALIASES::response& res) {
	return on_get_all_aliases(req,res);
}
bool RpcServer::on_dnc1_alias_by_address(const COMMAND_RPC_GET_ALIASES_BY_ADDRESS::request& req, COMMAND_RPC_GET_ALIASES_BY_ADDRESS::response& res) {
	return on_alias_by_address(req,res);
}
bool get_block_scratchpad_addendum(const CryptoNote::Block& b, std::vector<Crypto::Hash>& res)
{
	//if (get_block_height(b))
	//	res.push_back(b.prev_id);

	//Crypto::PublicKey tx_pub;
	//parseTransactionExtra();
	//bool r = parse_and_validate_tx_extra(b.miner_tx, tx_pub);
	//CHECK_AND_ASSERT_MES(r, false, "wrong miner tx in put_block_scratchpad_data: no one-time tx pubkey");
	//res.push_back(*reinterpret_cast<crypto::hash*>(&tx_pub));
	//res.push_back(get_tx_tree_hash(b));
	//for (const auto& out : b.miner_tx.vout)
	//{
	//	CHECK_AND_ASSERT_MES(out.target.type() == typeid(txout_to_key), false, "wrong tx out type in coinbase!!!");
	//	/*
	//	tx outs possible to fill with nonrandom data, let's hash it with prev_tx to avoid nonrandom data in scratchpad
	//	*/
	//	res.push_back(hash_together(b.prev_id, boost::get<txout_to_key>(out.target).key));
	//}
	return true;
}
bool RpcServer::get_addendum_for_hi(const height_info& hi, std::list<addendum>& res)
{
	if (!hi.height || hi.height + 1 == m_core.get_current_blockchain_height())
		return true;//do not make addendum for whole blockchain

	if (hi.height > m_core.get_current_blockchain_height())	{
		throw JsonRpc::JsonRpcError{
			CORE_RPC_ERROR_CODE_WRONG_PARAM,
			"wrong height parameter passed:" };
	}

	Crypto::Hash h;
	if (!podFromHex(hi.block_id, h)) {
		throw JsonRpc::JsonRpcError{
			CORE_RPC_ERROR_CODE_WRONG_PARAM,
			"wrong block_id parameter passed: " };
	}

	auto block_chain_id = m_core.getBlockIdByHeight(hi.height);
	if (block_chain_id == NULL_HASH) {
		throw JsonRpc::JsonRpcError{
			CORE_RPC_ERROR_CODE_WRONG_PARAM,
			"internal error: can't get block id by height: " };
	}

	uint64_t height = hi.height;
	if (block_chain_id != h)
	{
		//probably split
		if (hi.height <= 0) {
			throw JsonRpc::JsonRpcError{
				CORE_RPC_ERROR_CODE_WRONG_PARAM,
				"wrong height passed" };
		}
		--height;
	}

	std::list<CryptoNote::Block> blocks;
	if (!m_core.get_blocks(height + 1, m_core.get_current_blockchain_height() - (height + 1), blocks))	{
		throw JsonRpc::JsonRpcError{
			CORE_RPC_ERROR_CODE_WRONG_PARAM,
			"failed to get blocks" };
	}

	for (auto it = blocks.begin(); it != blocks.end(); it++)
	{
		res.push_back(addendum());
		res.back().hi.height = get_block_height(*it);
		res.back().hi.block_id = Common::podToHex(get_block_hash(*it));
		//res.back().prev_id = Common::podToHex(it->prev_id);
		//std::vector<Crypto::Hash> ad;
		//r = get_block_scratchpad_addendum(*it, ad);
		//CHECK_AND_ASSERT_MES(r, false, "Failed to add block addendum");
		//addendum_to_hexstr(ad, res.back().addm);

	}
	return true;
}
bool RpcServer::on_dnc1_get_addendums(const COMMAND_RPC_GET_ADDENDUMS::request& req, COMMAND_RPC_GET_ADDENDUMS::response& res) {
	
	if (!get_addendum_for_hi(req, res.addms))
	{
		res.status = "Fail at get_addendum_for_hi, check daemon logs for details";
		return true;
	}
	res.status = CORE_RPC_STATUS_OK;
	return true;
}
#endif

bool RpcServer::on_dnc1_getblockcount(const COMMAND_RPC_GETBLOCKCOUNT::request& req, COMMAND_RPC_GETBLOCKCOUNT::response& res) {
	return on_getblockcount(req,res);
}
bool RpcServer::on_dnc1_getblockhash(const COMMAND_RPC_GETBLOCKHASH::request& req, COMMAND_RPC_GETBLOCKHASH::response& res) {
	return on_getblockhash(req,res);
}
bool RpcServer::on_dnc1_getblocktemplate(const COMMAND_RPC_GETBLOCKTEMPLATE_DNC1::request& req, COMMAND_RPC_GETBLOCKTEMPLATE_DNC1::response& res)
{
	if (req.reserve_size > TX_EXTRA_NONCE_MAX_COUNT) {
		throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_TOO_BIG_RESERVE_SIZE, "To big reserved size, maximum 255" };
	}

	AccountPublicAddress acc = boost::value_initialized<AccountPublicAddress>();

	if (!req.wallet_address.size() || !m_core.currency().parseAccountAddressString(req.wallet_address, acc)) {
		throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_WRONG_WALLET_ADDRESS, "Failed to parse wallet address" };
	}

	Block b = boost::value_initialized<Block>();
	CryptoNote::BinaryArray blob_reserve;
	blob_reserve.resize(req.reserve_size, 0);
	if (!m_core.get_block_template(b, acc, res.difficulty, res.height, blob_reserve)) {
		logger(ERROR) << "Failed to create block template";
		throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Internal error: failed to create block template" };
	}

	BinaryArray block_blob = toBinaryArray(b);
	PublicKey tx_pub_key = CryptoNote::getTransactionPublicKeyFromExtra(b.baseTransaction.extra);
	if (tx_pub_key == NULL_PUBLIC_KEY) {
		logger(ERROR) << "Failed to find tx pub key in coinbase extra";
		throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Internal error: failed to find tx pub key in coinbase extra" };
	}

	if (0 < req.reserve_size) {
		res.reserved_offset = slow_memmem((void*)block_blob.data(), block_blob.size(), &tx_pub_key, sizeof(tx_pub_key));
		if (!res.reserved_offset) {
			logger(ERROR) << "Failed to find tx pub key in blockblob";
			throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Internal error: failed to create block template" };
		}
		res.reserved_offset += sizeof(tx_pub_key) + 3; //3 bytes: tag for TX_EXTRA_TAG_PUBKEY(1 byte), tag for TX_EXTRA_NONCE(1 byte), counter in TX_EXTRA_NONCE(1 byte)
		if (res.reserved_offset + req.reserve_size > block_blob.size()) {
			logger(ERROR) << "Failed to calculate offset for reserved bytes";
			throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Internal error: failed to create block template" };
		}
	}
	else {
		res.reserved_offset = 0;
	}

	res.blocktemplate_blob = toHex(block_blob);
	res.status = CORE_RPC_STATUS_OK;

	return true;
}
bool RpcServer::on_dnc1_submitblock(const COMMAND_RPC_SUBMITBLOCK::request& req, COMMAND_RPC_SUBMITBLOCK::response& res) {
	return on_submitblock(req,res);
}
bool RpcServer::on_dnc1_get_last_block_header(const COMMAND_RPC_GET_LAST_BLOCK_HEADER_DNC1::request& req, COMMAND_RPC_GET_LAST_BLOCK_HEADER_DNC1::response& res)
{
	uint32_t last_block_height;
	Hash last_block_hash;

	m_core.get_blockchain_top(last_block_height, last_block_hash);

	Block last_block;
	if (!m_core.getBlockByHash(last_block_hash, last_block)) {
		throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Internal error: can't get last block hash." };
	}

	fill_block_header_response(last_block, false, last_block_height, last_block_hash, res.block_header);
	res.status = CORE_RPC_STATUS_OK;
	return true;
}
bool RpcServer::on_dnc1_get_block_header_by_hash(const COMMAND_RPC_GET_BLOCK_HEADER_BY_HASH::request& req, COMMAND_RPC_GET_BLOCK_HEADER_BY_HASH::response& res) {
	return on_get_block_header_by_hash(req,res);
}
bool RpcServer::on_dnc1_get_block_header_by_height(const COMMAND_RPC_GET_BLOCK_HEADER_BY_HEIGHT::request& req, COMMAND_RPC_GET_BLOCK_HEADER_BY_HEIGHT::response& res) {
	return on_get_block_header_by_height(req,res);
}

}