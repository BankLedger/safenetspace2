
// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "VisibleGroupMessagesModel.h"
#include "GroupMessagesModel.h"
#include "SortedMessagesModel.h"
#include "WalletAdapter.h"

namespace WalletGui
{
	VisibleGroupMessagesModel::VisibleGroupMessagesModel() : QSortFilterProxyModel()
	{
		setSourceModel(&SortedMessagesModel::instance());
		m_groupId = 0;
		connect(&WalletAdapter::instance(), &WalletAdapter::deleteMessageGroupSignal, this, &VisibleGroupMessagesModel::deleteMessageGroup, Qt::QueuedConnection);
	}

	VisibleGroupMessagesModel::~VisibleGroupMessagesModel()
	{
	}
	
	bool VisibleGroupMessagesModel::filterAcceptsColumn(int _sourceColumn, const QModelIndex& _sourceParent) const
	{
		return _sourceColumn == GroupMessagesModel::COLUMN_TYPE ||  _sourceColumn == GroupMessagesModel::COLUMN_MESSAGE;
	}

	void VisibleGroupMessagesModel::setGroupId(int id)
	{
		if (m_groupId == id)
			return;
		m_groupId = id;
		Q_EMIT changeGroupId(id);
		SortedMessagesModel::instance().setGroupId(id);
	}

	VisibleGroupMessagesModel& VisibleGroupMessagesModel::instance()
	{
		static VisibleGroupMessagesModel inst;
		return inst;
	}

	void VisibleGroupMessagesModel::deleteMessageGroup()
	{
		setGroupId(0);
	}
}
