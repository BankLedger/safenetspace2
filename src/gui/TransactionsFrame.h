
// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <QFrame>
#include <QStyledItemDelegate>
#include <QMenu>
#include <QAction>

namespace Ui {
class TransactionsFrame;
}

namespace WalletGui {

class TransactionsListModel;

class TransactionsFrame : public QFrame {
  Q_OBJECT
  Q_DISABLE_COPY(TransactionsFrame)

public:
  TransactionsFrame(QWidget* _parent);
  ~TransactionsFrame();

  void scrollToTransaction(const QModelIndex& _index);

private:
  QScopedPointer<Ui::TransactionsFrame> m_ui;
  QScopedPointer<TransactionsListModel> m_transactionsModel;

  QMenu *popMenu;
  QAction *action;
  QString m_stradd;

  Q_SLOT void exportToCsv();
  Q_SLOT void showTransactionDetails(const QModelIndex& _index);
  Q_SLOT void on_customContextMenuRequested(QPoint pos);
  Q_SLOT void rightClickedOperation();
};

}
