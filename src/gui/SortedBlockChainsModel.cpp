// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QDateTime>

#include "SortedBlockChainsModel.h"
#include "BlockChainsModel.h"

namespace WalletGui
{
	SortedBlockChainsModel& SortedBlockChainsModel::instance()
	{
		static SortedBlockChainsModel inst;
		return inst;
	}

	SortedBlockChainsModel::SortedBlockChainsModel() : QSortFilterProxyModel()
	{
		setSourceModel(&BlockChainsModel::instance());
		setDynamicSortFilter(true);
		sort(BlockChainsModel::COLUMN_HEIGHT, Qt::DescendingOrder);
	}
	
	SortedBlockChainsModel::~SortedBlockChainsModel()
	{
	}

	bool SortedBlockChainsModel::lessThan(const QModelIndex& _left, const QModelIndex& _right) const
	{
		uint leftDate = _left.data(BlockChainsModel::ROLE_HEIGHT).toUInt();
		uint rightDate = _right.data(BlockChainsModel::ROLE_HEIGHT).toUInt();

		return leftDate < rightDate;
	}
}