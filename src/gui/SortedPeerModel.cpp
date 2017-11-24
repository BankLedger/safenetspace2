// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QDateTime>

#include "SortedPeerModel.h"
#include "PeerModel.h"

namespace WalletGui
{
	SortedPeerModel& SortedPeerModel::instance()
	{
		static SortedPeerModel inst;
		return inst;
	}

	SortedPeerModel::SortedPeerModel() : QSortFilterProxyModel()
	{
		setSourceModel(&PeerModel::instance());
		setDynamicSortFilter(true);
		sort(PeerModel::COLUMN_NO, Qt::AscendingOrder);
	}
	
	SortedPeerModel::~SortedPeerModel()
	{
	}

	bool SortedPeerModel::lessThan(const QModelIndex& _left, const QModelIndex& _right) const
	{
		uint leftDate = _left.data(PeerModel::ROLE_NO).toUInt();
		uint rightDate = _right.data(PeerModel::ROLE_NO).toUInt();

		return leftDate < rightDate;
	}
}