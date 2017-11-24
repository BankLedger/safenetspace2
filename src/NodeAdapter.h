
// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <QObject>
#include <QThread>

#include <INode.h>
#include <IWalletLegacy.h>

#include "CryptoNoteWrapper.h"

namespace CryptoNote {

class Currency;
struct BlockChain;
}

namespace Logging {
  class LoggerManager;
}

namespace WalletGui {

class InProcessNodeInitializer;

class NodeAdapter : public QObject, public INodeCallback {
  Q_OBJECT
  Q_DISABLE_COPY(NodeAdapter)

public:
  static NodeAdapter& instance();

  quintptr getPeerCount() const;
  std::string convertPaymentId(const QString& _payment_id_string) const;
  QString extractPaymentId(const std::string& _extra) const;
  CryptoNote::IWalletLegacy* createWallet() const;

  bool init();
  void deinit();
  bool getBlockByHeight(uint32_t& blockHeight, CryptoNote::BlockChain &bc);

#if SQ_DOGETBLOCK_TEST
  INode* getnode();
  bool doGetBlocks(const std::vector<uint32_t>& blockHeights, std::vector<std::vector<CryptoNote::BlockDetails>>& blocks);
  bool doGetBlocks(const std::vector<Crypto::Hash>& blockHashes, std::vector<CryptoNote::BlockDetails>& blocks);
  bool doGetBlocks(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t blocksNumberLimit, std::vector<CryptoNote::BlockDetails>& blocks, uint32_t& blocksNumberWithinTimestamps);
#endif

#if SQ_GET_POOL_TEST
  bool GetPoolTransactions(std::vector<TransactionDetails>& transactions);
#endif

#if SQ_GET_TRANSACTION
  bool doGetTransactions(const std::vector<Crypto::Hash>& transactionHashes, std::vector<TransactionDetails>& transactions);
#endif

#if SQ_ADD_PEER_FRAME
  bool GetPeerList(std::list<PeerlistEntry>& pl_wite, std::list<PeerlistEntry>& pl_gray);
#endif

  //bool getBlockByHeight(uint32_t& blockHeight, BlockChain &bc);
  quint64 getLastKnownBlockHeight() const;
  quint64 getLastLocalBlockHeight() const;
  QDateTime getLastLocalBlockTimestamp() const;
  QDateTime getLastBlockTimestamp() const;
  void peerCountUpdated(Node& _node, size_t _count) Q_DECL_OVERRIDE;
  void localBlockchainUpdated(Node& _node, uint64_t _height) Q_DECL_OVERRIDE;
  void lastKnownBlockHeightUpdated(Node& _node, uint64_t _height) Q_DECL_OVERRIDE;

private:
  Node* m_node;
  QThread m_nodeInitializerThread;
  InProcessNodeInitializer* m_nodeInitializer;

  NodeAdapter();
  ~NodeAdapter();

  bool initInProcessNode();
  CryptoNote::CoreConfig makeCoreConfig() const;
  CryptoNote::NetNodeConfig makeNetNodeConfig() const;

Q_SIGNALS:
  void localBlockchainUpdatedSignal(quint64 _height);
  void lastKnownBlockHeightUpdatedSignal(quint64 _height);
  void nodeInitCompletedSignal();
  void nodeInitCompletedSignal1();
  void peerCountUpdatedSignal(quintptr _count);
  void initNodeSignal(Node** _node, const CryptoNote::Currency* currency, INodeCallback* _callback, Logging::LoggerManager* _loggerManager,
    const CryptoNote::CoreConfig& _coreConfig, const CryptoNote::NetNodeConfig& _netNodeConfig);
  void deinitNodeSignal(Node** _node);
};

}
