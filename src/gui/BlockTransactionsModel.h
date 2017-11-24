// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

#include <IWalletLegacy.h>

#include "BlockchainExplorer/BlockchainExplorer.h"
#include "BlockchainExplorerData.h"
#include "LoggerAdapter.h"
using namespace CryptoNote;

namespace WalletGui
{
	typedef QPair<CryptoNote::TransactionNo, CryptoNote::TransactionNo2> TransactionNo12;
	
	class BlockTransactionsModel : public QAbstractItemModel
	{
		Q_OBJECT
			Q_ENUMS(Columns)
			Q_ENUMS(Roles)
			
	public:
		enum Columns
		{
			COLUMN_NO = 0, COLUMN_HASH, COLUMN_SIZE, COLUMN_FEE, COLUMN_AMOUNT
		};
		
		enum Roles
		{
			ROLE_NO = Qt::UserRole, ROLE_HASH, ROLE_SIZE, ROLE_FEE, ROLE_AMOUNT, ROLE_COLUMN, ROLE_ROW
		};
		
		static BlockTransactionsModel& instance();
		static BlockTransactionsModel& instance(uint32_t _height);
		static BlockTransactionsModel& instance(BlockDetails  *_blockdetail);
		
		int columnCount(const QModelIndex& _parent = QModelIndex()) const Q_DECL_OVERRIDE;
		int rowCount(const QModelIndex& _parent = QModelIndex()) const Q_DECL_OVERRIDE;
		
		QVariant headerData(int _section, Qt::Orientation _orientation, int _role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
		QVariant data(const QModelIndex& _index, int _role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
		QModelIndex index(int _row, int _column, const QModelIndex& _parent = QModelIndex()) const Q_DECL_OVERRIDE;
		QModelIndex	parent(const QModelIndex& _index) const Q_DECL_OVERRIDE;
		
		bool showBlockTransactions(uint32_t _height = 0);
		bool showBlockTransactions(BlockDetails  *_blockdetail);
	
	private:
		uint32_t m_row;
		uint32_t m_height;
		std::vector<std::vector<BlockDetails>> m_blockdetails;
		BlockDetails m_blockdetail;
		BlockDetails  *mm_blockdetail;
		QVector<TransactionNo12> m_blockheights;
		QHash<CryptoNote::TransactionNo, QPair<quint32, quint32> > m_blockRow;
		
		BlockTransactionsModel();
		BlockTransactionsModel(uint32_t _height);
		BlockTransactionsModel(BlockDetails  *_blockdetail);
		~BlockTransactionsModel();
		
		QVariant getDisplayRole(const QModelIndex& _index) const;
		QVariant getDecorationRole(const QModelIndex& _index) const;
		QVariant getAlignmentRole(const QModelIndex& _index) const;
		QVariant getUserRole(const QModelIndex& _index, int _role, CryptoNote::BlockHeight _tno) const;
		
		void reloadBlockTransactions();
		void appendBlockTransaction(CryptoNote::TransactionNo _no, quint32& _row_count);
		void appendBlockTransaction(CryptoNote::TransactionNo _no);
		void updateBlockTransaction(CryptoNote::TransactionNo _no);
		void lastKnownHeightUpdated(quint64 _height);
		void reset();
	};
}
