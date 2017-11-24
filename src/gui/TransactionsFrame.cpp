
// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QFileDialog>
#include <QLabel>
#include <QClipboard>

#include "MainWindow.h"
#include "SortedTransactionsModel.h"
#include "TransactionsFrame.h"
#include "TransactionDetailsDialog.h"
#include "TransactionsListModel.h"
#include "TransactionsModel.h"

#include "ui_transactionsframe.h"

namespace WalletGui
{
TransactionsFrame::TransactionsFrame(QWidget* _parent) : QFrame(_parent), m_ui(new Ui::TransactionsFrame),  m_transactionsModel(new TransactionsListModel)
{
  m_ui->setupUi(this);
  m_ui->m_transactionsView->setModel(m_transactionsModel.data());
  m_ui->m_transactionsView->setStyleSheet("QTreeView{selection-background-color:blue;}");
  m_ui->m_transactionsView->header()->setSectionResizeMode(TransactionsModel::COLUMN_STATE, QHeaderView::Fixed);
  m_ui->m_transactionsView->header()->resizeSection(TransactionsModel::COLUMN_STATE, 25);
  m_ui->m_transactionsView->header()->resizeSection(TransactionsModel::COLUMN_DATE, 140);
  m_ui->m_transactionsView->header()->resizeSection(TransactionsModel::COLUMN_ADDRESS, 400);

  m_ui->m_transactionsView->setContextMenuPolicy(Qt::CustomContextMenu);
  popMenu = new QMenu(this);
  action = new QAction("Copy Address", this);
  connect(m_ui->m_transactionsView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(on_customContextMenuRequested(QPoint)));
  connect(action, SIGNAL(triggered()), this, SLOT(rightClickedOperation()));
}

TransactionsFrame::~TransactionsFrame() {
}

void TransactionsFrame::scrollToTransaction(const QModelIndex& _index) {
  QModelIndex sortedModelIndex = SortedTransactionsModel::instance().mapFromSource(_index);
  QModelIndex index = static_cast<QSortFilterProxyModel*>(m_ui->m_transactionsView->model())->mapFromSource(sortedModelIndex);
  m_ui->m_transactionsView->scrollTo(index);
  m_ui->m_transactionsView->setFocus();
  m_ui->m_transactionsView->setCurrentIndex(index);
}

void TransactionsFrame::exportToCsv() {
  QString file = QFileDialog::getSaveFileName(&MainWindow::instance(), tr("Select CSV file"), QDir::homePath(), "CSV (*.csv)");
  if (!file.isEmpty()) {
    QByteArray csv = TransactionsModel::instance().toCsv();
    QFile f(file);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
      f.write(csv);
      f.close();
    }
  }
}

void TransactionsFrame::showTransactionDetails(const QModelIndex& _index) {
  if (!_index.isValid()) {
    return;
  }

  TransactionDetailsDialog dlg(_index, &MainWindow::instance());
  dlg.exec();
}

void TransactionsFrame::on_customContextMenuRequested(QPoint pos)
{
	m_stradd = m_ui->m_transactionsView->indexAt(pos).sibling(m_ui->m_transactionsView->indexAt(pos).row(), TransactionsModel::COLUMN_ADDRESS).data().toString();
	popMenu->addAction(action);
	popMenu->exec(QCursor::pos());
}

void TransactionsFrame::rightClickedOperation()
{
	QApplication::clipboard()->setText(m_stradd);
}
}
