
// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <QSortFilterProxyModel>

namespace WalletGui
{
	class VisibleGroupMessagesModel : public QSortFilterProxyModel
	{
		Q_OBJECT
	public:
		VisibleGroupMessagesModel();
		~VisibleGroupMessagesModel();
		void setGroupId(int id);
		static VisibleGroupMessagesModel& instance();

	protected:
		bool filterAcceptsColumn(int _sourceColumn, const QModelIndex& _sourceParent) const Q_DECL_OVERRIDE;
		int m_groupId;
		void deleteMessageGroup();

	Q_SIGNALS:
		void changeGroupId(const int& _groupId);
	};
}
