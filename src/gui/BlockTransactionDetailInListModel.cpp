// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SortedBlockTransactionDetailInModel.h"
#include "BlockTransactionDetailInListModel.h"
#include "BlockTransactionDetailInModel.h"

namespace WalletGui
{
	BlockTransactionDetailInListModel::BlockTransactionDetailInListModel() : QSortFilterProxyModel()
	{
		setSourceModel(&SortedBlockTransactionDetailInModel::instance());
	}

	BlockTransactionDetailInListModel::BlockTransactionDetailInListModel(const TransactionDetails *_blockdetail) : QSortFilterProxyModel()
	{
		setSourceModel(&SortedBlockTransactionDetailInModel::instance(_blockdetail));
	}

	BlockTransactionDetailInListModel::~BlockTransactionDetailInListModel()
	{
	}

	bool BlockTransactionDetailInListModel::filterAcceptsColumn(int _sourceColumn, const QModelIndex& _sourceParent) const
	{
		quint32 column = sourceModel()->headerData(_sourceColumn, Qt::Horizontal, BlockTransactionDetailInModel::ROLE_COLUMN).toUInt();
		return  column ==BlockTransactionDetailInModel::COLUMN_NO || column == BlockTransactionDetailInModel::COLUMN_AMOUNT||
			column == BlockTransactionDetailInModel::COLUMN_KEYIMAGE;
	}
}
