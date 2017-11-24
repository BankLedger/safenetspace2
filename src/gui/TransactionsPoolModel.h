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
	
	class TransactionsPoolModel : public QAbstractItemModel
	{
		Q_OBJECT
			Q_ENUMS(Columns)
			Q_ENUMS(Roles)
			
	public:
		//enum Columns
		//{
		//	COLUMN_NO = 0, COLUMN_TIMESTAMP, COLUMN_HASH, COLUMN_SIZE, COLUMN_TRANSACTIONS
		//};
		//
		//enum Roles
		//{
		//	ROLE_NO = Qt::UserRole, ROLE_TIMESTAMP, ROLE_HASH, ROLE_SIZE, ROLE_TRANSACTIONS, ROLE_HEIGHT, ROLE_COLUMN, ROLE_ROW
		//};
		enum Columns {
			COLUMN_NO = 0, COLUMN_HASH, COLUMN_FEE, COLUMN_PAYMENT_ID, COLUMN_AMOUNT, COLUMN_AMOUNTOUT, COLUMN_STATE, COLUMN_DATE,
			COLUMN_ADDRESS, COLUMN_MESSAGE, COLUMN_HEIGHT, COLUMN_TYPE
		};

		enum Roles {
			ROLE_NO = Qt::UserRole, ROLE_DATE, ROLE_TYPE, ROLE_HASH, ROLE_ADDRESS, ROLE_AMOUNT, ROLE_AMOUNTOUT, ROLE_PAYMENT_ID, ROLE_ICON,
			ROLE_TRANSACTION_ID, ROLE_HEIGHT, ROLE_FEE, ROLE_NUMBER_OF_CONFIRMATIONS, ROLE_COLUMN, ROLE_ROW, ROLE_MESSAGE,
			ROLE_MESSAGES, ROLE_DEPOSIT_ID, ROLE_DEPOSIT_COUNT
		};
		static TransactionsPoolModel& instance();
		static TransactionsPoolModel& instance(uint32_t _height);
		
		int columnCount(const QModelIndex& _parent = QModelIndex()) const Q_DECL_OVERRIDE;
		int rowCount(const QModelIndex& _parent = QModelIndex()) const Q_DECL_OVERRIDE;
		
		QVariant headerData(int _section, Qt::Orientation _orientation, int _role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
		QVariant data(const QModelIndex& _index, int _role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
		QModelIndex index(int _row, int _column, const QModelIndex& _parent = QModelIndex()) const Q_DECL_OVERRIDE;
		QModelIndex	parent(const QModelIndex& _index) const Q_DECL_OVERRIDE;

		Q_SLOT void ShowPoolTransactions();
	
	private:
		uint32_t m_row;
		std::vector<TransactionDetails> m_transactions;
		
		TransactionsPoolModel();
		~TransactionsPoolModel();
		
		QVariant getDisplayRole(const QModelIndex& _index) const;
		QVariant getDecorationRole(const QModelIndex& _index) const;
		QVariant getAlignmentRole(const QModelIndex& _index) const;
		QVariant getUserRoleDetails(const QModelIndex& _index, int _role, CryptoNote::BlockHeight _bh, const TransactionDetails* _trandetails) const;
		
		void reset();
	};
}
