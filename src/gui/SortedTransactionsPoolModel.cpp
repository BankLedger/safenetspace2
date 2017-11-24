// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QDateTime>

#include "SortedTransactionsPoolModel.h"
#include "TransactionsPoolModel.h"

namespace WalletGui
{
	SortedTransactionsPoolModel& SortedTransactionsPoolModel::instance()
	{
		static SortedTransactionsPoolModel inst;
		return inst;
	}

	SortedTransactionsPoolModel::SortedTransactionsPoolModel() : QSortFilterProxyModel()
	{
		setSourceModel(&TransactionsPoolModel::instance());
		setDynamicSortFilter(true);
		sort(TransactionsPoolModel::COLUMN_NO, Qt::AscendingOrder);
	}
	
	SortedTransactionsPoolModel::~SortedTransactionsPoolModel()
	{
	}

	bool SortedTransactionsPoolModel::lessThan(const QModelIndex& _left, const QModelIndex& _right) const
	{
		uint leftDate = _left.data(TransactionsPoolModel::ROLE_HEIGHT).toUInt();
		uint rightDate = _right.data(TransactionsPoolModel::ROLE_HEIGHT).toUInt();

		return leftDate < rightDate;
	}
}