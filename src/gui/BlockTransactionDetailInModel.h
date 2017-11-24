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
	
	class BlockTransactionDetailInModel : public QAbstractItemModel
	{
		Q_OBJECT
			Q_ENUMS(Columns)
			Q_ENUMS(Roles)
			
	public:
		enum Columns
		{
			COLUMN_NO = 0, COLUMN_AMOUNT, COLUMN_KEYIMAGE
		};
		
		enum Roles
		{
			ROLE_NO = Qt::UserRole, ROLE_AMOUNT, ROLE_KEYIMAGE, ROLE_COLUMN, ROLE_ROW
		};
		
		static BlockTransactionDetailInModel& instance();
		static BlockTransactionDetailInModel& instance(uint32_t _height);
		static BlockTransactionDetailInModel& instance(const TransactionDetails *_blockdetail);
		
		int columnCount(const QModelIndex& _parent = QModelIndex()) const Q_DECL_OVERRIDE;
		int rowCount(const QModelIndex& _parent = QModelIndex()) const Q_DECL_OVERRIDE;
		
		QVariant headerData(int _section, Qt::Orientation _orientation, int _role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
		QVariant data(const QModelIndex& _index, int _role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
		QModelIndex index(int _row, int _column, const QModelIndex& _parent = QModelIndex()) const Q_DECL_OVERRIDE;
		QModelIndex	parent(const QModelIndex& _index) const Q_DECL_OVERRIDE;
		
		bool showBlockChains(uint32_t _startheight = 0);
	
	private:
		uint32_t m_row;
		const TransactionDetails *m_transactiondetail;
		
		BlockTransactionDetailInModel();
		BlockTransactionDetailInModel(const TransactionDetails *_blockdetail);
		~BlockTransactionDetailInModel();
		
		QVariant getDisplayRole(const QModelIndex& _index) const;
		QVariant getDecorationRole(const QModelIndex& _index) const;
		QVariant getAlignmentRole(const QModelIndex& _index) const;
		QVariant getUserRoleDetails(const QModelIndex& _index, int _role, CryptoNote::BlockHeight _bh, const TransactionInputDetails* _inputdetails) const;
		
		void reloadBlockChains();
		void reset();
	};
}
