// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "BlockTransactionsListModel.h"

namespace WalletGui
{
	BlockTransactionsListModel::BlockTransactionsListModel() : QSortFilterProxyModel()
	{
		setSourceModel(&SortedBlockTransactionsModel::instance());
	}

	BlockTransactionsListModel::BlockTransactionsListModel(uint32_t _height) : QSortFilterProxyModel()
	{
		m_height = _height;
		setSourceModel(&SortedBlockTransactionsModel::instance(_height));
	}

	BlockTransactionsListModel::BlockTransactionsListModel(BlockDetails  *_blockdetail) : QSortFilterProxyModel()
	{
		m_blockdetail = _blockdetail;
		setSourceModel(&SortedBlockTransactionsModel::instance(_blockdetail));
	}

	BlockTransactionsListModel::~BlockTransactionsListModel()
	{
	}

	bool BlockTransactionsListModel::filterAcceptsColumn(int _sourceColumn, const QModelIndex& _sourceParent) const
	{
		quint32 column = sourceModel()->headerData(_sourceColumn, Qt::Horizontal, BlockTransactionsModel::ROLE_COLUMN).toUInt();
		return column == BlockTransactionsModel::COLUMN_NO || column == BlockTransactionsModel::COLUMN_HASH || column == BlockTransactionsModel::COLUMN_SIZE ||
			column == BlockTransactionsModel::COLUMN_FEE || column == BlockTransactionsModel::COLUMN_AMOUNT;
	}
}
