
// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <QFrame>
#include "CryptoNoteConfig.h"

namespace Ui {
class AddressBookFrame;
}

namespace WalletGui {

class AddressBookFrame : public QFrame {
  Q_OBJECT
  Q_DISABLE_COPY(AddressBookFrame)

public:
  AddressBookFrame(QWidget* _parent);
  ~AddressBookFrame();

private:
  QScopedPointer<Ui::AddressBookFrame> m_ui;

  Q_SLOT void addClicked();
  Q_SLOT void copyClicked();
  Q_SLOT void deleteClicked();
  Q_SLOT void currentAddressChanged(const QModelIndex& _index);
  Q_SLOT void addressDoubleClicked(const QModelIndex& _index);

#if SQ_ADD_BM_WALLET
  std::vector<CryptoNote::AddressGroup> m_groupList;
  Q_SLOT void addGroupClicked();
  Q_SLOT void deleteGroupClicked();
  Q_SLOT void createGroupClicked();
  void walletInitCompleted(int _error, const QString& _error_text);
#endif

Q_SIGNALS:
  void payToSignal(const QModelIndex& _index);
};

}
