// Copyright (c) 2015-2016 darknetspace developer
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QClipboard>

#include "MainWindow.h"
#include "BlockTransactionDetailDialog.h"
#include "ui_blocktransactiondetaildialog.h"

namespace WalletGui
{
	//using namespace CryptoNote;
	BlockTransactionDetailDialog::BlockTransactionDetailDialog(QWidget* _parent) :
		QDialog(_parent), m_ui(new Ui::BlockTransactionDetailDialog)
	{
		m_ui->setupUi(this);
		m_ui->m_blocktransactioninputView->hide();
	}

	BlockTransactionDetailDialog::BlockTransactionDetailDialog(QWidget* _parent, const TransactionDetails *_blockdetail) :
		QDialog(_parent), m_ui(new Ui::BlockTransactionDetailDialog), m_blocktransactiondetailinlistModel(new BlockTransactionDetailInListModel(_blockdetail)),
		m_blocktransactiondetailoutlistModel(new BlockTransactionDetailOutListModel(_blockdetail))
	{
		m_ui->setupUi(this);

		func_initmenu();
		func_initview(_blockdetail);
		func_inittable(_blockdetail);
		//func_inittopinfo(_blockdetail);
		m_ui->groupBox->hide();
	}

	BlockTransactionDetailDialog::~BlockTransactionDetailDialog()
	{
	}

	void BlockTransactionDetailDialog::func_inittopinfo(const TransactionDetails *_blockdetail)
	{
		m_ui->groupBox->setStyleSheet("QGroupBox{border:none;}");
		QString strbuf;
		strbuf = "<span style=\" font-weight:600;\">Block Hash: </span>";
		for (int i = 0; i < sizeof(Crypto::Hash); i++)
		{
			strbuf += QString::number(_blockdetail->blockHash.data[i]).sprintf("%02x", _blockdetail->blockHash.data[i])/*.toUpper()*/;
		}
		m_ui->label_blockhash->setText(strbuf);
		m_ui->label_blockheight->setText("<span style=\" font-weight:600;\">Block Height: </span>" + QString::number(_blockdetail->blockHeight));
		m_ui->label_blocktime->setText("<span style=\" font-weight:600;\">Block Date: </span>" + QDateTime::fromTime_t(_blockdetail->timestamp).toString("yyyy-MM-dd HH:mm"));
		strbuf = "<span style=\" font-weight:600;\">Hash: </span>";
		for (int i = 0; i < sizeof(Crypto::Hash); i++)
		{
			strbuf += QString::number(_blockdetail->hash.data[i]).sprintf("%02x", _blockdetail->hash.data[i])/*.toUpper()*/;
		}
		m_ui->label_hash->setText(strbuf);
		m_ui->label_amount->setText("<span style=\" font-weight:600;\">Amount: </span>" + QString::number(_blockdetail->totalOutputsAmount / 100000000.0));
		m_ui->label_size->setText("<span style=\" font-weight:600;\">Size: </span>" + QString::number(_blockdetail->size));
		m_ui->label_fee->setText("<span style=\" font-weight:600;\">Fee: </span>" + QString::number(_blockdetail->fee / 100000000.0));
	}

	void BlockTransactionDetailDialog::func_initmenu()
	{
		m_ui->m_blocktransactioninputView->setContextMenuPolicy(Qt::CustomContextMenu);
		popMenuIn = new QMenu(this);
		actionIn = new QAction("Copy Key", this);
		connect(m_ui->m_blocktransactioninputView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(on_customContextMenuRequestedIn(QPoint)));
		connect(actionIn, SIGNAL(triggered()), this, SLOT(rightClickedOperationIn()));

		m_ui->m_blocktransactionoutputView->setContextMenuPolicy(Qt::CustomContextMenu);
		popMenuOut = new QMenu(this);
		actionOut = new QAction("Copy Key", this);
		connect(m_ui->m_blocktransactionoutputView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(on_customContextMenuRequestedOut(QPoint)));
		connect(actionOut, SIGNAL(triggered()), this, SLOT(rightClickedOperationOut()));

		m_ui->tableWidget_detail->setContextMenuPolicy(Qt::CustomContextMenu);
		popMenuTable = new QMenu(this);
		actionTable = new QAction("Copy Hash", this);
		connect(m_ui->tableWidget_detail, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(on_customContextMenuRequestedTable(QPoint)));
		connect(actionTable, SIGNAL(triggered()), this, SLOT(rightClickedOperationTable()));
	}

	void BlockTransactionDetailDialog::func_initview(const TransactionDetails *_blockdetail)
	{
		if (_blockdetail->inputs.size() > 0)
		{
			m_ui->label_input->setText("<span style = \" font-weight:600;\">Input</span>");
			m_ui->label_input->setVisible(true);
			m_ui->m_blocktransactioninputView->setVisible(true);
			m_ui->m_blocktransactioninputView->setModel(m_blocktransactiondetailinlistModel.data());
			m_ui->m_blocktransactioninputView->header()->setSectionResizeMode(BlockTransactionDetailInModel::COLUMN_NO, QHeaderView::Fixed);
			m_ui->m_blocktransactioninputView->header()->resizeSection(BlockTransactionDetailInModel::COLUMN_NO, 50);
			m_ui->m_blocktransactioninputView->header()->resizeSection(BlockTransactionDetailInModel::COLUMN_AMOUNT, 100);
			m_ui->m_blocktransactioninputView->header()->resizeSection(BlockTransactionDetailInModel::COLUMN_KEYIMAGE, 350);
		}
		else
		{
			m_ui->label_input->hide();
			m_ui->m_blocktransactioninputView->hide();
		}

		if (_blockdetail->outputs.size() > 0)
		{
			m_ui->label_output->setText("<span style = \" font-weight:600;\">Output</span>");
			m_ui->label_output->setVisible(true);
			m_ui->m_blocktransactionoutputView->setVisible(true);
			m_ui->m_blocktransactionoutputView->setModel(m_blocktransactiondetailoutlistModel.data());
			m_ui->m_blocktransactionoutputView->header()->setSectionResizeMode(BlockTransactionDetailOutModel::COLUMN_NO, QHeaderView::Fixed);
			m_ui->m_blocktransactionoutputView->header()->resizeSection(BlockTransactionDetailOutModel::COLUMN_NO, 50);
			m_ui->m_blocktransactionoutputView->header()->resizeSection(BlockTransactionDetailOutModel::COLUMN_AMOUNT, 100);
			m_ui->m_blocktransactionoutputView->header()->resizeSection(BlockTransactionDetailOutModel::COLUMN_INDEX, 100);
			m_ui->m_blocktransactionoutputView->header()->resizeSection(BlockTransactionDetailOutModel::COLUMN_OUTKEY, 350);
		}
		else
		{
			m_ui->label_output->hide();
			m_ui->m_blocktransactionoutputView->hide();
		}
	}

	void BlockTransactionDetailDialog::func_inittable(const TransactionDetails *_blockdetail)
	{
		m_ui->tableWidget_detail->horizontalHeader()->resizeSection(0, 120);
		m_ui->tableWidget_detail->horizontalHeader()->resizeSection(1, 300);
		m_ui->tableWidget_detail->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
		m_ui->tableWidget_detail->horizontalHeader()->resizeSection(2, 120);
		m_ui->tableWidget_detail->horizontalHeader()->resizeSection(3, 120);

		if (!_blockdetail)
			return;

		QString strbuf;
		if (m_ui->tableWidget_detail->item(0, 1) == NULL)
		{
			strbuf = "";
			for (int i = 0; i < sizeof(Crypto::Hash); i++)
			{
				strbuf += QString::number(_blockdetail->blockHash.data[i]).sprintf("%02x", _blockdetail->blockHash.data[i])/*.toUpper()*/;
			}
			QTableWidgetItem *item_buf = new QTableWidgetItem(strbuf);
			m_ui->tableWidget_detail->setItem(0, 1, item_buf);
			item_buf = new QTableWidgetItem(QDateTime::fromTime_t(_blockdetail->timestamp).toString("yyyy-MM-dd HH:mm"));
			m_ui->tableWidget_detail->setItem(1, 1, item_buf);
			item_buf = new QTableWidgetItem(QString::number(_blockdetail->blockHeight));
			m_ui->tableWidget_detail->setItem(1, 3, item_buf);

			strbuf = "";
			for (int i = 0; i < sizeof(Crypto::Hash); i++)
			{
				strbuf += QString::number(_blockdetail->hash.data[i]).sprintf("%02x", _blockdetail->hash.data[i])/*.toUpper()*/;
			}
			item_buf = new QTableWidgetItem(strbuf);
			m_ui->tableWidget_detail->setItem(2, 1, item_buf);
			item_buf = new QTableWidgetItem(QString::number(_blockdetail->size));
			m_ui->tableWidget_detail->setItem(2, 3, item_buf);
			QString amountStr = CurrencyAdapter::instance().formatAmount(qAbs(_blockdetail->totalOutputsAmount));
			item_buf = new QTableWidgetItem(((_blockdetail->totalOutputsAmount) < 0) ? ("-" + amountStr) : (amountStr));
			m_ui->tableWidget_detail->setItem(3, 1, item_buf);
			item_buf = new QTableWidgetItem(CurrencyAdapter::instance().formatAmount(qAbs(_blockdetail->fee)));
			m_ui->tableWidget_detail->setItem(3, 3, item_buf);
		}
		else
		{
			strbuf = "";
			for (int i = 0; i < sizeof(Crypto::Hash); i++)
			{
				strbuf += QString::number(_blockdetail->blockHash.data[i]).sprintf("%02x", _blockdetail->blockHash.data[i])/*.toUpper()*/;
			}
			m_ui->tableWidget_detail->item(0, 1)->setText(strbuf);
			m_ui->tableWidget_detail->item(1, 1)->setText(QDateTime::fromTime_t(_blockdetail->timestamp).toString("yyyy-MM-dd HH:mm"));
			m_ui->tableWidget_detail->item(1, 3)->setText(QString::number(_blockdetail->blockHeight));
			strbuf = "";
			for (int i = 0; i < sizeof(Crypto::Hash); i++)
			{
				strbuf += QString::number(_blockdetail->hash.data[i]).sprintf("%02x", _blockdetail->hash.data[i])/*.toUpper()*/;
			}
			m_ui->tableWidget_detail->item(2, 1)->setText(strbuf);
			m_ui->tableWidget_detail->item(2, 3)->setText(QString::number(_blockdetail->size));
			QString amountStr = CurrencyAdapter::instance().formatAmount(qAbs(_blockdetail->totalOutputsAmount));
			m_ui->tableWidget_detail->item(3, 1)->setText(((_blockdetail->totalOutputsAmount) < 0) ? ("-" + amountStr) : (amountStr));
			m_ui->tableWidget_detail->item(3, 3)->setText(CurrencyAdapter::instance().formatAmount(qAbs(_blockdetail->fee)));
		}
	}

	void BlockTransactionDetailDialog::scrollToTransaction(const QModelIndex& _index)
	{
		QModelIndex sortedModelIndex = SortedBlockTransactionDetailInModel::instance().mapFromSource(_index);
		QModelIndex index = static_cast<QSortFilterProxyModel*>(m_ui->m_blocktransactioninputView->model())->mapFromSource(sortedModelIndex);
		m_ui->m_blocktransactioninputView->scrollTo(index);
		m_ui->m_blocktransactioninputView->setFocus();
		m_ui->m_blocktransactioninputView->setCurrentIndex(index);
	}

	void BlockTransactionDetailDialog::on_customContextMenuRequestedIn(QPoint pos)
	{
		m_strIn = QString(m_ui->m_blocktransactioninputView->indexAt(pos).data(BlockTransactionDetailInModel::ROLE_KEYIMAGE).toByteArray().toHex().toUpper());
		popMenuIn->addAction(actionIn);
		popMenuIn->exec(QCursor::pos());
	}

	void BlockTransactionDetailDialog::rightClickedOperationIn()
	{
		QApplication::clipboard()->setText(m_strIn);
	}

	void BlockTransactionDetailDialog::on_customContextMenuRequestedOut(QPoint pos)
	{
		m_strOut = QString(m_ui->m_blocktransactionoutputView->indexAt(pos).data(BlockTransactionDetailOutModel::ROLE_OUTKEY).toByteArray().toHex().toUpper());
		popMenuOut->addAction(actionOut);
		popMenuOut->exec(QCursor::pos());
	}

	void BlockTransactionDetailDialog::rightClickedOperationOut()
	{
		QApplication::clipboard()->setText(m_strOut);
	}

	void BlockTransactionDetailDialog::on_customContextMenuRequestedTable(QPoint pos)
	{
		int row, col;
		row = m_ui->tableWidget_detail->indexAt(pos).row();
		col  = m_ui->tableWidget_detail->indexAt(pos).column();
		if ((col == 1) && ((row == 0) || (row == 2)))
		{
			m_strTable = m_ui->tableWidget_detail->item(row, col)->text();
			popMenuTable->addAction(actionTable);
			popMenuTable->exec(QCursor::pos());
		}
	}

	void BlockTransactionDetailDialog::rightClickedOperationTable()
	{
		QApplication::clipboard()->setText(m_strTable);
	}
}
