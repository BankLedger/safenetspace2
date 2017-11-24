
// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <QDialog>
#include "BlockchainExplorer/BlockchainExplorer.h"
#include "BlockchainExplorerData.h"
using namespace CryptoNote;

namespace Ui {
class TransactionDetailsDialog;
}

namespace WalletGui {
	using namespace CryptoNote;
class TransactionDetailsDialog : public QDialog {
  Q_OBJECT
  Q_DISABLE_COPY(TransactionDetailsDialog)

public:
	TransactionDetailsDialog(const QModelIndex &_index, QWidget* _parent);
	TransactionDetailsDialog(const TransactionDetails &_trdetail, QWidget* _parent);
  ~TransactionDetailsDialog();

private:
  QScopedPointer<Ui::TransactionDetailsDialog> m_ui;
  const QString m_detailsTemplate;
};

}
