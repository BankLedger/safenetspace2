// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SortedBlockTransactionDetailOutModel.h"
#include "BlockTransactionDetailOutListModel.h"
#include "BlockTransactionDetailOutModel.h"

namespace WalletGui
{
	BlockTransactionDetailOutListModel::BlockTransactionDetailOutListModel() : QSortFilterProxyModel()
	{
		setSourceModel(&SortedBlockTransactionDetailOutModel::instance());
	}

	BlockTransactionDetailOutListModel::BlockTransactionDetailOutListModel(const TransactionDetails  *_transactiondetail) : QSortFilterProxyModel()
	{
		setSourceModel(&SortedBlockTransactionDetailOutModel::instance(_transactiondetail));
	}

	BlockTransactionDetailOutListModel::~BlockTransactionDetailOutListModel()
	{
	}

	bool BlockTransactionDetailOutListModel::filterAcceptsColumn(int _sourceColumn, const QModelIndex& _sourceParent) const
	{
		quint32 column = sourceModel()->headerData(_sourceColumn, Qt::Horizontal, BlockTransactionDetailOutModel::ROLE_COLUMN).toUInt();
		return column == BlockTransactionDetailOutModel::COLUMN_NO || column == BlockTransactionDetailOutModel::COLUMN_AMOUNT ||
			column == BlockTransactionDetailOutModel::COLUMN_INDEX || column == BlockTransactionDetailOutModel::COLUMN_OUTKEY;
	}
}
