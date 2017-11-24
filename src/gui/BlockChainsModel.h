// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

#include "CurrencyAdapter.h"
#include "Message.h"
#include "NodeAdapter.h"
#include "WalletAdapter.h"

#if SQ_DOGETBLOCK_TEST
#include "BlockchainExplorer/BlockchainExplorer.h"
#include "BlockchainExplorerData.h"
#include "LoggerAdapter.h"
using namespace CryptoNote;
#endif

namespace WalletGui
{
	typedef QPair<CryptoNote::TransactionId, CryptoNote::TransferId> TransactionTransferId; 
	typedef QPair<CryptoNote::BlockHeight, CryptoNote::BlockHeight2> BlockHeight;
	
	class BlockChainsModel : public QAbstractItemModel
	{
		Q_OBJECT
			Q_ENUMS(Columns)
			Q_ENUMS(Roles)
			
	public:
		bool  m_b_lasted;
		enum Columns
		{
			COLUMN_HEIGHT = 0, COLUMN_TIMESTAMP, COLUMN_SIZE, COLUMN_TRANSACTIONS, COLUMN_HASH
		};
		
		enum Roles
		{
			ROLE_DATE = Qt::UserRole, ROLE_HASH, ROLE_SIZE, ROLE_TRANSACTIONS, ROLE_HEIGHT, ROLE_COLUMN, ROLE_ROW
		};
		
		static BlockChainsModel& instance();
		static BlockChainsModel& instance(uint32_t _height);
		
		int columnCount(const QModelIndex& _parent = QModelIndex()) const Q_DECL_OVERRIDE;
		int rowCount(const QModelIndex& _parent = QModelIndex()) const Q_DECL_OVERRIDE;
		
		QVariant headerData(int _section, Qt::Orientation _orientation, int _role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
		QVariant data(const QModelIndex& _index, int _role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
		QModelIndex index(int _row, int _column, const QModelIndex& _parent = QModelIndex()) const Q_DECL_OVERRIDE;
		QModelIndex	parent(const QModelIndex& _index) const Q_DECL_OVERRIDE;
		
		bool showBlockChains(uint32_t _startheight = 0);
		bool showBlockChainsEnd(uint32_t _endheight = 0);
		Q_SLOT void showLastestBlockChains();
		Q_SLOT void showNextBlockChains();
		Q_SLOT void showLastBlockChains();
		Q_SLOT void showFirstBlockChains();
		Q_SLOT void updateBlockChains();
		Q_SLOT bool changeBlocksPerPage(uint32_t _blockspage);
		Q_SLOT bool turnPage(uint32_t _page);
	
	private:
		uint32_t m_row;
		uint32_t m_start_height;
		QVector<BlockHeight> m_blockheights;
		QHash<CryptoNote::BlockHeight, QPair<quint32, quint32> > m_blockRow;
		std::vector<BlockDetails> m_blockdetails;
		
		BlockChainsModel();
		~BlockChainsModel();
		
		QVariant getDisplayRole(const QModelIndex& _index) const;
		QVariant getDecorationRole(const QModelIndex& _index) const;
		QVariant getAlignmentRole(const QModelIndex& _index) const;
		QVariant getUserRole(const QModelIndex& _index, int _role, CryptoNote::BlockHeight _bh, const CryptoNote::BlockChain& _blockchain) const;
		QVariant getUserRoleDetails(const QModelIndex& _index, int _role, CryptoNote::BlockHeight _bh, const BlockDetails* _blockdetails) const;
		
		void reloadBlockChains();
		void appendBlockChain(CryptoNote::BlockHeight _height, quint32& _row_count);
		void appendBlockChain(CryptoNote::BlockHeight _height);
		void updateBlockChain(CryptoNote::BlockHeight _height);
		void lastKnownHeightUpdated(quint64 _height);
		void reset();
	};
}
