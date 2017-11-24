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

#include "BlockchainExplorer/BlockchainExplorer.h"
#include "BlockchainExplorerData.h"
#include "LoggerAdapter.h"
using namespace CryptoNote;

namespace WalletGui
{
	typedef QPair<CryptoNote::TransactionId, CryptoNote::TransferId> TransactionTransferId; 
	typedef QPair<CryptoNote::BlockHeight, CryptoNote::BlockHeight2> BlockHeight;
	
	class BlockTransactionDetailOutModel : public QAbstractItemModel
	{
		Q_OBJECT
			Q_ENUMS(Columns)
			Q_ENUMS(Roles)
			
	public:
		enum Columns
		{
			COLUMN_NO = 0, COLUMN_AMOUNT, COLUMN_INDEX, COLUMN_OUTKEY
		};
		
		enum Roles
		{
			ROLE_NO = Qt::UserRole, ROLE_AMOUNT, ROLE_INDEX, ROLE_OUTKEY, ROLE_COLUMN, ROLE_ROW
		};
		
		static BlockTransactionDetailOutModel& instance();
		static BlockTransactionDetailOutModel& instance(uint32_t _height);
		static BlockTransactionDetailOutModel& instance(const TransactionDetails  *_transactiondetail);
		
		int columnCount(const QModelIndex& _parent = QModelIndex()) const Q_DECL_OVERRIDE;
		int rowCount(const QModelIndex& _parent = QModelIndex()) const Q_DECL_OVERRIDE;
		
		QVariant headerData(int _section, Qt::Orientation _orientation, int _role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
		QVariant data(const QModelIndex& _index, int _role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
		QModelIndex index(int _row, int _column, const QModelIndex& _parent = QModelIndex()) const Q_DECL_OVERRIDE;
		QModelIndex	parent(const QModelIndex& _index) const Q_DECL_OVERRIDE;
		
		bool showBlockChains(uint32_t _startheight = 0);
	
	private:
		uint32_t m_row;
		const TransactionDetails  *m_transactiondetail;
		
		BlockTransactionDetailOutModel();
		BlockTransactionDetailOutModel(const TransactionDetails  *_transactiondetail);
		~BlockTransactionDetailOutModel();
		
		QVariant getDisplayRole(const QModelIndex& _index) const;
		QVariant getDecorationRole(const QModelIndex& _index) const;
		QVariant getAlignmentRole(const QModelIndex& _index) const;
		QVariant getUserRoleDetails(const QModelIndex& _index, int _role, CryptoNote::BlockHeight _bh, const TransactionOutputDetails* _outputdetails) const;
		
		void reloadBlockChains();
		void reset();
	};
}
