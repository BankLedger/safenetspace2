
// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <system_error>
#include "InProcessNode/InProcessNode.h"
#include "BlockchainExplorerData.h"

#if SQ_DOGETBLOCK_TEST
#include "INode.h"
using namespace CryptoNote;
#endif

namespace CryptoNote {
class INode;
class IWalletLegacy;
class Currency;
class CoreConfig;
class NetNodeConfig;
struct BlockChain;
struct BlockDetails;
}

namespace Logging {
  class LoggerManager;
}

namespace WalletGui {

class Node {
public:
  virtual ~Node() = 0;
  virtual void init(const std::function<void(std::error_code)>& callback) = 0;
  virtual void deinit() = 0;
  
  virtual bool getBlockByHeight(uint32_t& blockHeight, CryptoNote::BlockChain &bc) = 0;

#if SQ_DOGETBLOCK_TEST
  virtual INode* getnode() = 0;
  virtual bool doGetBlocks(const std::vector<uint32_t>& blockHeights, std::vector<std::vector<CryptoNote::BlockDetails>>& blocks) = 0;
  virtual bool doGetBlocks(const std::vector<Crypto::Hash>& blockHashes, std::vector<CryptoNote::BlockDetails>& blocks) = 0;
  virtual bool doGetBlocks(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t blocksNumberLimit, std::vector<CryptoNote::BlockDetails>& blocks, uint32_t& blocksNumberWithinTimestamps) = 0;
#endif

#if SQ_GET_POOL_TEST
  virtual bool GetPoolTransactions(std::vector<TransactionDetails>& transactions) = 0;
#endif

#if SQ_GET_TRANSACTION
  virtual bool doGetTransactions(const std::vector<Crypto::Hash>& transactionHashes, std::vector<TransactionDetails>& transactions) = 0;
#endif

#if SQ_ADD_PEER_FRAME
  virtual bool GetPeerList(std::list<PeerlistEntry>& pl_wite, std::list<PeerlistEntry>& pl_gray) = 0;
#endif

  virtual std::string convertPaymentId(const std::string& paymentIdString) = 0;
  virtual std::string extractPaymentId(const std::string& extra) = 0;
  virtual uint64_t getLastKnownBlockHeight() const = 0;
  virtual uint64_t getLastLocalBlockHeight() const = 0;
  virtual uint64_t getLastLocalBlockTimestamp() const = 0;
  virtual uint64_t getPeerCount() const = 0;

  virtual CryptoNote::IWalletLegacy* createWallet() = 0;
};

class INodeCallback {
public:
  virtual void peerCountUpdated(Node& node, size_t count) = 0;
  virtual void localBlockchainUpdated(Node& node, uint64_t height) = 0;
  virtual void lastKnownBlockHeightUpdated(Node& node, uint64_t height) = 0;
};

Node* createRpcNode(const CryptoNote::Currency& currency, INodeCallback& callback, const std::string& nodeHost, unsigned short nodePort);
Node* createInprocessNode(const CryptoNote::Currency& currency, Logging::LoggerManager& logManager,
  const CryptoNote::CoreConfig& coreConfig, const CryptoNote::NetNodeConfig& netNodeConfig, INodeCallback& callback);

}
