
// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QDateTime>

#include "SortedMessagesGroupModel.h"
#include "GroupModel.h"

namespace WalletGui
{
	SortedMessagesGroupModel& SortedMessagesGroupModel::instance()
	{
		static SortedMessagesGroupModel inst;
		return inst;
	}

	SortedMessagesGroupModel::SortedMessagesGroupModel() : QSortFilterProxyModel()
	{
		setSourceModel(&GroupModel::instance());
		setDynamicSortFilter(true);
		sort(GroupModel::COLUMN_NO, Qt::DescendingOrder);
	}

	SortedMessagesGroupModel::~SortedMessagesGroupModel()
	{
	}

	bool SortedMessagesGroupModel::lessThan(const QModelIndex& _left, const QModelIndex& _right) const
	{
		uint leftDate = _left.data(GroupModel::ROLE_NO).toUInt();
		uint rightDate = _right.data(GroupModel::ROLE_NO).toUInt();

		return leftDate > rightDate;
	}
}
