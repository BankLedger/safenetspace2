// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SortedTransactionsPoolModel.h"
#include "TransactionsPoolListModel.h"
#include "TransactionsPoolModel.h"

namespace WalletGui
{
	TransactionsPoolListModel::TransactionsPoolListModel() : QSortFilterProxyModel()
	{
		setSourceModel(&SortedTransactionsPoolModel::instance());
	}
	
	TransactionsPoolListModel::~TransactionsPoolListModel()
	{
	}

	bool TransactionsPoolListModel::filterAcceptsColumn(int _sourceColumn, const QModelIndex& _sourceParent) const
	{
		quint32 column = sourceModel()->headerData(_sourceColumn, Qt::Horizontal, TransactionsPoolModel::ROLE_COLUMN).toUInt();
		return column == TransactionsPoolModel::COLUMN_NO || /*column == TransactionsPoolModel::COLUMN_DATE ||*/
			column == TransactionsPoolModel::COLUMN_HASH || /*column == TransactionsPoolModel::COLUMN_MESSAGE ||*/
			/*column == TransactionsPoolModel::COLUMN_AMOUNT || column == TransactionsPoolModel::COLUMN_AMOUNTOUT ||*/
			column == TransactionsPoolModel::COLUMN_FEE || column == TransactionsPoolModel::COLUMN_PAYMENT_ID;
	}
}
