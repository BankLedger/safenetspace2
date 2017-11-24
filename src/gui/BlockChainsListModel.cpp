// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SortedBlockChainsModel.h"
#include "BlockChainsListModel.h"
#include "BlockChainsModel.h"

namespace WalletGui
{
	BlockChainsListModel::BlockChainsListModel() : QSortFilterProxyModel()
	{
		setSourceModel(&SortedBlockChainsModel::instance());
	}
	
	BlockChainsListModel::~BlockChainsListModel()
	{
	}

	bool BlockChainsListModel::filterAcceptsColumn(int _sourceColumn, const QModelIndex& _sourceParent) const
	{
		quint32 column = sourceModel()->headerData(_sourceColumn, Qt::Horizontal, BlockChainsModel::ROLE_COLUMN).toUInt();
		return column == BlockChainsModel::COLUMN_HEIGHT || column == BlockChainsModel::COLUMN_TIMESTAMP ||
			column == BlockChainsModel::COLUMN_SIZE || column == BlockChainsModel::COLUMN_TRANSACTIONS ||
			column == BlockChainsModel::COLUMN_HASH;
	}
}
