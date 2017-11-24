// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QDateTime>

#include "SortedBlockTransactionsModel.h"
#include "BlockTransactionsModel.h"

namespace WalletGui
{
	SortedBlockTransactionsModel& SortedBlockTransactionsModel::instance()
	{
		static SortedBlockTransactionsModel inst;
		return inst;
	}

	SortedBlockTransactionsModel& SortedBlockTransactionsModel::instance(uint32_t _height)
	{
		static SortedBlockTransactionsModel inst(_height);
#if 1
		inst.setSourceModel(&BlockTransactionsModel::instance(_height));
#else
		if (inst.m_height != _height)
		{
			inst.m_height = _height;
			inst.setSourceModel(&BlockTransactionsModel::instance(_height));
		}
#endif
		return inst;
	}

	SortedBlockTransactionsModel& SortedBlockTransactionsModel::instance(BlockDetails  *_blockdetail)
	{
		static SortedBlockTransactionsModel inst(_blockdetail);
#if 1
		inst.setSourceModel(&BlockTransactionsModel::instance(_blockdetail));
#else
		if (inst.m_blockdetail != _blockdetail)
		{
			inst.m_blockdetail = _blockdetail;
			inst.setSourceModel(&BlockTransactionsModel::instance(_blockdetail));
		}
#endif
		return inst;
	}

	SortedBlockTransactionsModel::SortedBlockTransactionsModel(uint32_t _height) : QSortFilterProxyModel()
	{
		setSourceModel(&BlockTransactionsModel::instance(_height));
		setDynamicSortFilter(true);
		sort(BlockTransactionsModel::COLUMN_NO, Qt::AscendingOrder);
	}

	SortedBlockTransactionsModel::SortedBlockTransactionsModel(BlockDetails  *_blockdetail) : QSortFilterProxyModel()
	{
//		setSourceModel(&BlockTransactionsModel::instance(_blockdetail));
		setDynamicSortFilter(true);
		sort(BlockTransactionsModel::COLUMN_NO, Qt::AscendingOrder);
	}

	SortedBlockTransactionsModel::SortedBlockTransactionsModel() : QSortFilterProxyModel()
	{
		setSourceModel(&BlockTransactionsModel::instance());
		setDynamicSortFilter(true);
		sort(BlockTransactionsModel::COLUMN_NO, Qt::AscendingOrder);
	}

	SortedBlockTransactionsModel::~SortedBlockTransactionsModel()
	{
	}

	bool SortedBlockTransactionsModel::lessThan(const QModelIndex& _left, const QModelIndex& _right) const
	{
		uint leftDate = _left.data(BlockTransactionsModel::ROLE_NO).toUInt();
		uint rightDate = _right.data(BlockTransactionsModel::ROLE_NO).toUInt();

		return leftDate < rightDate;
	}
}