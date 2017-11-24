
// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QDateTime>

#include "SortedMessagesModel.h"
#include "MessagesModel.h"

#if SQ_ADD_BM_WALLET
#include "GroupMessagesModel.h"
#endif

namespace WalletGui {

SortedMessagesModel& SortedMessagesModel::instance() {
  static SortedMessagesModel inst;
  return inst;
}

SortedMessagesModel::SortedMessagesModel() : QSortFilterProxyModel() {
#if SQ_ADD_BM_WALLET
	setSourceModel(&GroupMessagesModel::instance());
	sort(GroupMessagesModel::COLUMN_DATE, Qt::DescendingOrder);
#else
	setSourceModel(&MessagesModel::instance());
	sort(MessagesModel::COLUMN_DATE, Qt::DescendingOrder);
#endif
  setDynamicSortFilter(true);
}

SortedMessagesModel::~SortedMessagesModel() {
}

bool SortedMessagesModel::lessThan(const QModelIndex& _left, const QModelIndex& _right) const {
#if SQ_ADD_BM_WALLET
	QDateTime leftDate = _left.data(GroupMessagesModel::ROLE_DATE).toDateTime();
	QDateTime rightDate = _right.data(GroupMessagesModel::ROLE_DATE).toDateTime();
#else
  QDateTime leftDate = _left.data(MessagesModel::ROLE_DATE).toDateTime();
  QDateTime rightDate = _right.data(MessagesModel::ROLE_DATE).toDateTime();
#endif
  if((rightDate.isNull() || !rightDate.isValid()) && (leftDate.isNull() || !leftDate.isValid())) {
    return _left.row() < _right.row();
  }

  if(leftDate.isNull() || !leftDate.isValid()) {
    return false;
  }

  if(rightDate.isNull() || !rightDate.isValid()) {
    return true;
  }

  return leftDate < rightDate;
}

#if SQ_ADD_BM_WALLET
bool SortedMessagesModel::setGroupId(int id)
{
	m_groupId = id;
	Q_EMIT changeGroupId(id);
	return GroupMessagesModel::instance().setGroupId(id);
}
#endif
}
