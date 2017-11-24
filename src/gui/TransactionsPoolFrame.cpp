// Copyright (c) 2015-2016 darknetspace developer
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QClipboard>

#include "MainWindow.h"
#include "SortedTransactionsPoolModel.h"
#include "TransactionsPoolFrame.h"
#include "TransactionsPoolModel.h"
#include "TransactionsPoolListModel.h"
#include "ui_transactionspoolframe.h"

#include "Common/StringTools.h"

namespace WalletGui
{
	TransactionsPoolFrame::TransactionsPoolFrame(QWidget* _parent) : 
		QFrame(_parent),
		m_ui(new Ui::TransactionsPoolFrame),
		m_transactionspoollistModel(new TransactionsPoolListModel)
	{
		m_ui->setupUi(this);
		m_ui->m_transactionspoolView->setModel(m_transactionspoollistModel.data());
		m_ui->m_transactionspoolView->header()->setSectionResizeMode(TransactionsPoolModel::COLUMN_NO, QHeaderView::Fixed);
		m_ui->m_transactionspoolView->header()->resizeSection(TransactionsPoolModel::COLUMN_NO, 25);
		m_ui->m_transactionspoolView->header()->resizeSection(TransactionsPoolModel::COLUMN_HASH, 460);
		//m_ui->m_transactionspoolView->header()->resizeSection(TransactionsPoolModel::COLUMN_AMOUNT, 100);
		//m_ui->m_transactionspoolView->header()->resizeSection(TransactionsPoolModel::COLUMN_AMOUNTOUT, 100);
		m_ui->m_transactionspoolView->header()->resizeSection(TransactionsPoolModel::COLUMN_FEE, 100);
		m_ui->m_transactionspoolView->header()->setSectionResizeMode(TransactionsPoolModel::COLUMN_PAYMENT_ID, QHeaderView::Stretch);
		m_ui->m_transactionspoolView->header()->resizeSection(TransactionsPoolModel::COLUMN_PAYMENT_ID, 400);

		m_ui->m_transactionspoolView->setContextMenuPolicy(Qt::CustomContextMenu);
		popMenu = new QMenu(this);
		action_hash = new QAction("Copy Hash", this);
		action_payid = new QAction("Copy Payment ID", this);
		connect(action_hash, SIGNAL(triggered()), this, SLOT(rightClickedOperation()));
		connect(action_payid, SIGNAL(triggered()), this, SLOT(rightClickedOperation_PayID()));
	}
	
	TransactionsPoolFrame::~TransactionsPoolFrame()
	{
	}
	
	void TransactionsPoolFrame::scrollToTransaction(const QModelIndex& _index)
	{
		QModelIndex sortedModelIndex = SortedTransactionsPoolModel::instance().mapFromSource(_index);
		QModelIndex index = static_cast<QSortFilterProxyModel*>(m_ui->m_transactionspoolView->model())->mapFromSource(sortedModelIndex);
		m_ui->m_transactionspoolView->scrollTo(index);
		m_ui->m_transactionspoolView->setFocus();
		m_ui->m_transactionspoolView->setCurrentIndex(index);
	}
	
	void TransactionsPoolFrame::showBlockChainDetails(const QModelIndex& _index)
	{
		if (!_index.isValid())
		{
			return;
		}

		uint32_t height = uint32_t(_index.data(TransactionsPoolModel::ROLE_HEIGHT).toUInt());

		return;
	}
	
	void TransactionsPoolFrame::on_customContextMenuRequested(QPoint pos)
	{
		popMenu->clear();
		m_strhash = QString(m_ui->m_transactionspoolView->indexAt(pos).data(TransactionsPoolModel::ROLE_HASH).toByteArray().toHex()/*.toUpper()*/);
		if (!(m_strhash.isEmpty()))
			popMenu->addAction(action_hash);
		m_strpayid = QString(m_ui->m_transactionspoolView->indexAt(pos).data(TransactionsPoolModel::ROLE_PAYMENT_ID).toByteArray().toHex().toUpper());
		if (!(m_strpayid.isEmpty()))
			popMenu->addAction(action_payid);
		popMenu->exec(QCursor::pos());
	}

	void TransactionsPoolFrame::rightClickedOperation()
	{
		QApplication::clipboard()->setText(m_strhash);
	}

	void TransactionsPoolFrame::rightClickedOperation_PayID()
	{
		QApplication::clipboard()->setText(m_strpayid);
	}
}
