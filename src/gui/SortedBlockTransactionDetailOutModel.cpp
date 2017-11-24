// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QDateTime>

#include "SortedBlockTransactionDetailOutModel.h"
#include "BlockTransactionDetailOutModel.h"

namespace WalletGui
{
	SortedBlockTransactionDetailOutModel& SortedBlockTransactionDetailOutModel::instance()
	{
		static SortedBlockTransactionDetailOutModel inst;
		return inst;
	}

	SortedBlockTransactionDetailOutModel& SortedBlockTransactionDetailOutModel::instance(const TransactionDetails  *_transactiondetail)
	{
		static SortedBlockTransactionDetailOutModel inst(_transactiondetail);
		inst.setSourceModel(&BlockTransactionDetailOutModel::instance(_transactiondetail));
		return inst;
	}

	SortedBlockTransactionDetailOutModel::SortedBlockTransactionDetailOutModel() : QSortFilterProxyModel()
	{
		setSourceModel(&(BlockTransactionDetailOutModel::instance()));
		setDynamicSortFilter(true);
		sort(BlockTransactionDetailOutModel::COLUMN_NO, Qt::AscendingOrder);
	}

	SortedBlockTransactionDetailOutModel::SortedBlockTransactionDetailOutModel(const TransactionDetails  *_transactiondetail) : QSortFilterProxyModel()
	{
		setDynamicSortFilter(true);
		sort(BlockTransactionDetailOutModel::COLUMN_NO, Qt::AscendingOrder);
	}

	SortedBlockTransactionDetailOutModel::~SortedBlockTransactionDetailOutModel()
	{
	}

	bool SortedBlockTransactionDetailOutModel::lessThan(const QModelIndex& _left, const QModelIndex& _right) const
	{
		uint leftDate = _left.data(BlockTransactionDetailOutModel::COLUMN_NO).toUInt();
		uint rightDate = _right.data(BlockTransactionDetailOutModel::COLUMN_NO).toUInt();

		return leftDate < rightDate;
	}
}