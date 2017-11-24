// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SortedPeerModel.h"
#include "PeerListModel.h"
#include "PeerModel.h"

namespace WalletGui
{
	PeerListModel::PeerListModel() : QSortFilterProxyModel()
	{
		setSourceModel(&SortedPeerModel::instance());
	}
	
	PeerListModel::~PeerListModel()
	{
	}

	bool PeerListModel::filterAcceptsColumn(int _sourceColumn, const QModelIndex& _sourceParent) const
	{
		quint32 column = sourceModel()->headerData(_sourceColumn, Qt::Horizontal, PeerModel::ROLE_COLUMN).toUInt();
		return column == PeerModel::COLUMN_NO || column == PeerModel::COLUMN_TYPE ||
			column == PeerModel::COLUMN_ID || column == PeerModel::COLUMN_ADDRESS || column == PeerModel::COLUMN_SEEN;
	}
}
