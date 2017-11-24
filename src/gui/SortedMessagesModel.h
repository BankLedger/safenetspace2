
// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <QSortFilterProxyModel>
#include "CryptoNoteConfig.h"

namespace WalletGui {

class SortedMessagesModel : public QSortFilterProxyModel {
  Q_OBJECT
  Q_DISABLE_COPY(SortedMessagesModel)

public:
	static SortedMessagesModel& instance();
#if SQ_ADD_BM_WALLET
	bool setGroupId(int id);
#endif

protected:
  bool lessThan(const QModelIndex& _left, const QModelIndex& _right) const Q_DECL_OVERRIDE;

private:
  SortedMessagesModel();
  ~SortedMessagesModel();
#if SQ_ADD_BM_WALLET
  int m_groupId;
Q_SIGNALS:
  void changeGroupId(const int& _groupId);
#endif
};



}
