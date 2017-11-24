// Copyright (c) 2015-2016 darknetspace developer
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QDateTime>
#include <QTextStream>
#include <QClipboard>

#include "MainWindow.h"
#include "BlockTransactionsDialog.h"
#include "ui_blocktransactionsdialog.h"
#include "BlockchainExplorerData.h"

namespace WalletGui
{
	//using namespace CryptoNote;
	BlockTransactionsDialog::BlockTransactionsDialog(QWidget* _parent, uint32_t  _height) :
		QDialog(_parent), m_ui(new Ui::BlockTransactionsDialog), m_blocktransactionsModel(new BlockTransactionsListModel(_height)), m_height(_height)
	{
		m_ui->setupUi(this);
		m_ui->m_blocktransactionsView->setModel(m_blocktransactionsModel.data());
		m_ui->m_blocktransactionsView->setStyleSheet("QTreeView{selection-background-color:blue;}");
		m_ui->m_blocktransactionsView->header()->setSectionResizeMode(BlockTransactionsModel::COLUMN_NO, QHeaderView::Fixed);
		m_ui->m_blocktransactionsView->header()->resizeSection(BlockTransactionsModel::COLUMN_NO, 50);
		m_ui->m_blocktransactionsView->header()->resizeSection(BlockTransactionsModel::COLUMN_HASH, 500);
		m_ui->m_blocktransactionsView->header()->resizeSection(BlockTransactionsModel::COLUMN_SIZE, 50);
		m_ui->m_blocktransactionsView->header()->resizeSection(BlockTransactionsModel::COLUMN_FEE, 100);
		m_ui->m_blocktransactionsView->header()->resizeSection(BlockTransactionsModel::COLUMN_AMOUNT, 100);
	}

	BlockTransactionsDialog::BlockTransactionsDialog(QWidget* _parent, BlockDetails *_blockdetail) :
		QDialog(_parent), m_ui(new Ui::BlockTransactionsDialog), m_blocktransactionsModel(new BlockTransactionsListModel(_blockdetail)), m_blockdetail(_blockdetail)
	{
		m_ui->setupUi(this); 

		m_ui->m_blocktransactionsView->setContextMenuPolicy(Qt::CustomContextMenu);
		popMenu = new QMenu(this);
		action = new QAction("Copy Hash", this);
		connect(m_ui->m_blocktransactionsView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(on_customContextMenuRequested(QPoint)));
		connect(action, SIGNAL(triggered()), this, SLOT(rightClickedOperation()));

		QString strbuf = "";
		for (int i = 0; i < sizeof(Crypto::Hash); i++)
		{
			strbuf += QString::number(_blockdetail->hash.data[i]).sprintf("%02x", _blockdetail->hash.data[i])/*.toUpper()*/;
		}
		m_ui->lineEdit_hash->setStyleSheet("QLineEdit{border-width:0;border-style:outset}");
		m_ui->lineEdit_hash->setText(strbuf);
		m_ui->m_label_hash->hide();
		if (m_ui->m_tableWidget_blockdetail->item(0, 1) == NULL)
		{
			QTableWidgetItem *item_buf = new QTableWidgetItem(QString::number(_blockdetail->height));
			m_ui->m_tableWidget_blockdetail->setItem(0, 1, item_buf);
			item_buf = new QTableWidgetItem(QDateTime::fromTime_t(_blockdetail->timestamp).toString("yyyy-MM-dd HH:mm"));
			m_ui->m_tableWidget_blockdetail->setItem(1, 1, item_buf);
			item_buf = new QTableWidgetItem(QString::number(_blockdetail->difficulty));
			m_ui->m_tableWidget_blockdetail->setItem(2, 1, item_buf);
			item_buf = new QTableWidgetItem(CurrencyAdapter::instance().formatAmount(qAbs(_blockdetail->reward)));
			m_ui->m_tableWidget_blockdetail->setItem(3, 1, item_buf);
			item_buf = new QTableWidgetItem((_blockdetail->isOrphaned) ? ("yes") : ("no"));
			m_ui->m_tableWidget_blockdetail->setItem(0, 3, item_buf);
			item_buf = new QTableWidgetItem(QString::number(_blockdetail->blockSize));
			m_ui->m_tableWidget_blockdetail->setItem(1, 3, item_buf);
			item_buf = new QTableWidgetItem(CurrencyAdapter::instance().formatAmount(qAbs(_blockdetail->totalFeeAmount)));
			m_ui->m_tableWidget_blockdetail->setItem(2, 3, item_buf);
			item_buf = new QTableWidgetItem(QString::number(_blockdetail->transactions.size()));
			m_ui->m_tableWidget_blockdetail->setItem(3, 3, item_buf);
		}
		else
		{
			m_ui->m_tableWidget_blockdetail->item(0, 1)->setText(QString::number(_blockdetail->height));
			m_ui->m_tableWidget_blockdetail->item(1, 1)->setText(QDateTime::fromTime_t(_blockdetail->timestamp).toString("yyyy-MM-dd HH:mm"));
			m_ui->m_tableWidget_blockdetail->item(2, 1)->setText(QString::number(_blockdetail->difficulty));
			m_ui->m_tableWidget_blockdetail->item(3, 1)->setText(CurrencyAdapter::instance().formatAmount(qAbs(_blockdetail->reward)));
			m_ui->m_tableWidget_blockdetail->item(0, 3)->setText((_blockdetail->isOrphaned) ? ("yes") : ("no"));
			m_ui->m_tableWidget_blockdetail->item(1, 3)->setText(QString::number(_blockdetail->blockSize));
			m_ui->m_tableWidget_blockdetail->item(2, 3)->setText(CurrencyAdapter::instance().formatAmount(qAbs(_blockdetail->totalFeeAmount)));
			m_ui->m_tableWidget_blockdetail->item(3, 3)->setText(QString::number(_blockdetail->transactions.size()));
		}

		m_ui->m_blocktransactionsView->setModel(m_blocktransactionsModel.data());
		m_ui->m_blocktransactionsView->setStyleSheet("QTreeView{selection-background-color:blue;}");
		m_ui->m_blocktransactionsView->header()->setSectionResizeMode(BlockTransactionsModel::COLUMN_NO, QHeaderView::Fixed);
		m_ui->m_blocktransactionsView->header()->resizeSection(BlockTransactionsModel::COLUMN_NO, 50);
		m_ui->m_blocktransactionsView->header()->resizeSection(BlockTransactionsModel::COLUMN_HASH, 500);
		m_ui->m_blocktransactionsView->header()->resizeSection(BlockTransactionsModel::COLUMN_SIZE, 50);
		m_ui->m_blocktransactionsView->header()->resizeSection(BlockTransactionsModel::COLUMN_FEE, 100);
		m_ui->m_blocktransactionsView->header()->resizeSection(BlockTransactionsModel::COLUMN_AMOUNT, 100);
	}

	BlockTransactionsDialog::~BlockTransactionsDialog()
	{
	}
	
	void BlockTransactionsDialog::scrollToTransaction(const QModelIndex& _index)
	{
		QModelIndex sortedModelIndex = SortedBlockTransactionsModel::instance().mapFromSource(_index);
		QModelIndex index = static_cast<QSortFilterProxyModel*>(m_ui->m_blocktransactionsView->model())->mapFromSource(sortedModelIndex);
		m_ui->m_blocktransactionsView->scrollTo(index);
		m_ui->m_blocktransactionsView->setFocus();
		m_ui->m_blocktransactionsView->setCurrentIndex(index);
	}
	
	void BlockTransactionsDialog::showBlockTransactionDetail(const QModelIndex& _index)
	{
		if (!_index.isValid())
		{
			return;
		}

		uint32_t height = uint32_t(_index.data(BlockTransactionsModel::ROLE_NO).toUInt());

		m_trandetail = &(m_blockdetail->transactions.at(height));
		BlockTransactionDetailDialog dlg(&MainWindow::instance(), &(m_blockdetail->transactions.at(height)));
		dlg.exec();
	}

	void BlockTransactionsDialog::on_customContextMenuRequested(QPoint pos)
	{
		m_strhash = QString(m_ui->m_blocktransactionsView->indexAt(pos).data(BlockTransactionsModel::ROLE_HASH).toByteArray().toHex()/*.toUpper()*/);
		popMenu->addAction(action);
		popMenu->exec(QCursor::pos());
	}

	void BlockTransactionsDialog::rightClickedOperation()
	{
		QApplication::clipboard()->setText(m_strhash);
	}
}
