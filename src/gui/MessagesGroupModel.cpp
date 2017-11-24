
// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MessagesGroupModel.h"
#include "SortedMessagesGroupModel.h"
#include "GroupModel.h"

namespace WalletGui
{
	MessagesGroupModel::MessagesGroupModel() : QSortFilterProxyModel()
	{
		setSourceModel(&SortedMessagesGroupModel::instance());
	}
	MessagesGroupModel::~MessagesGroupModel()
	{
	}
	
	bool MessagesGroupModel::filterAcceptsColumn(int _sourceColumn, const QModelIndex& _sourceParent) const
	{
		return  _sourceColumn == GroupModel::COLUMN_ADDRESS;
	}
}
