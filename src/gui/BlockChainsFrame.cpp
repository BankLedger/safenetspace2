// Copyright (c) 2015-2016 darknetspace developer
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QClipboard>

#include "MainWindow.h"
#include "SortedBlockChainsModel.h"
#include "BlockChainsFrame.h"
#include "BlockChainsModel.h"
#include "BlockChainsListModel.h"
#include "ui_blockchainsframe.h"
#include "BlockTransactionsDialog.h"

#include "Common/StringTools.h"
#include <QTableView>

namespace WalletGui
{
	BlockChainsFrame::BlockChainsFrame(QWidget* _parent) : QFrame(_parent), m_ui(new Ui::BlockChainsFrame),  m_blockchainsModel(new BlockChainsListModel)
	{
		m_ui->setupUi(this);

		m_b_first = true;

		QValidator *validator = new QIntValidator(0, 9999, this);
		m_ui->lineEdit_blocks_page->setValidator(validator);

		QValidator *validator1 = new QIntValidator(1, 99999, this);
		m_ui->lineEdit_page->setValidator(validator1);

		m_ui->m_blockchainsView->setContextMenuPolicy(Qt::CustomContextMenu);
		popMenu = new QMenu(this);
		action = new QAction("Copy Hash", this);
		connect(action, SIGNAL(triggered()), this, SLOT(rightClickedOperation()));

		m_ui->m_blockchainsView->setModel(m_blockchainsModel.data());
		m_ui->m_blockchainsView->setStyleSheet("QTreeView{selection-background-color:blue;}");
		m_ui->m_blockchainsView->header()->setSectionResizeMode(BlockChainsModel::COLUMN_HEIGHT, QHeaderView::Fixed);
		m_ui->m_blockchainsView->header()->setSectionResizeMode(BlockChainsModel::COLUMN_TIMESTAMP, QHeaderView::Fixed);
		m_ui->m_blockchainsView->header()->resizeSection(BlockChainsModel::COLUMN_HEIGHT, 100);
		m_ui->m_blockchainsView->header()->resizeSection(BlockChainsModel::COLUMN_TIMESTAMP, 130);
		m_ui->m_blockchainsView->header()->resizeSection(BlockChainsModel::COLUMN_SIZE, 100);
		m_ui->m_blockchainsView->header()->resizeSection(BlockChainsModel::COLUMN_TRANSACTIONS, 100);
		m_ui->m_blockchainsView->header()->resizeSection(BlockChainsModel::COLUMN_HASH, 400);
#if 0
		QRegExp regx("[0-9]+$");
		QValidator *validator = new QRegExpValidator(regx, m_ui->lineEdit_height );
		m_ui->lineEdit_height->setValidator( validator ); 
#endif
	}
	
	BlockChainsFrame::~BlockChainsFrame()
	{
	}
	
	void BlockChainsFrame::scrollToTransaction(const QModelIndex& _index)
	{
		QModelIndex sortedModelIndex = SortedBlockChainsModel::instance().mapFromSource(_index);
		QModelIndex index = static_cast<QSortFilterProxyModel*>(m_ui->m_blockchainsView->model())->mapFromSource(sortedModelIndex);
		m_ui->m_blockchainsView->scrollTo(index);
		m_ui->m_blockchainsView->setFocus();
		m_ui->m_blockchainsView->setCurrentIndex(index);
	}
	
	void BlockChainsFrame::showBlockChainDetails(const QModelIndex& _index)
	{
		if (!_index.isValid())
		{
			return;
		}

		uint32_t height = uint32_t(_index.data(BlockChainsModel::ROLE_HEIGHT).toUInt());

		std::vector<std::vector<BlockDetails>> mblockdetails;
		std::vector<uint32_t> mheight;
		mheight.push_back(height);
		if (!(NodeAdapter::instance().doGetBlocks(mheight, mblockdetails)))
		{
			return;
		}
		if (mblockdetails.empty())
		{
			return;
		}
		BlockDetails *blockdetails;
		blockdetails = &(mblockdetails.at(0).at(0));
		BlockTransactionsDialog dlg(&MainWindow::instance(), &(mblockdetails.at(0).at(0)));
		dlg.exec();
	}

	void BlockChainsFrame::SearchBlockChains()
	{
		//QString str = m_ui->lineEdit_height->text();
		std::string pstr = m_ui->lineEdit_height->text().toStdString();
		if ((pstr.size() < 18))
		{
			for (int i = 0; i < pstr.size(); i++)
			{
				if ((pstr[i] < '0') || (pstr[i] > '9'))
				{
					QMessageBox::warning(nullptr, QObject::tr("Fail"), QString("Wrong argument!"));
					return;
				}
			}
#if 1
			std::vector<std::vector<BlockDetails>> mblockdetails;
			std::vector<uint32_t> mheight;
			CryptoNote::BlockHeight bh = uint32_t(m_ui->lineEdit_height->text().toUInt());

			mheight.push_back(bh);
			if( (!(NodeAdapter::instance().doGetBlocks(mheight, mblockdetails))) || (mblockdetails.empty()))
			{
				QMessageBox::warning(nullptr, QObject::tr("Warning"), QString("%1 can not be found!").arg(bh));
				return;
			}
			BlockChainsModel::instance().showBlockChainsEnd(bh);
			BlockTransactionsDialog dlg(&(MainWindow::instance()), &(mblockdetails.at(0).at(0)));
			dlg.exec();
#else
			uint32_t height = uint32_t(m_ui->lineEdit_height->text().toUInt());
			if (!(BlockChainsModel::instance().showBlockChains(height)))
			{
				QMessageBox::warning(nullptr, QObject::tr("Fail"), QString("%1 can not be found!").arg(height));
				return;
			}
#endif
		}
		else if (pstr.size() == (sizeof(Crypto::Hash)) * 2)
		{
			std::vector<Crypto::Hash> mhash;
			Crypto::Hash phash;
			size_t h_size;
			if (!(Common::fromHex(pstr, &phash, (sizeof(Crypto::Hash)), h_size)))
			{
				QMessageBox::warning(nullptr, QObject::tr("Warning"), QString("Wrong argument!"));
				return;
			}
			mhash.push_back(phash);
			std::vector<BlockDetails> mblockdetails;
			if ((NodeAdapter::instance().doGetBlocks(mhash, mblockdetails)) && (!(mblockdetails.empty())))
			{
				BlockChainsModel::instance().showBlockChainsEnd(mblockdetails.at(0).height);
				BlockTransactionsDialog dlg(&(MainWindow::instance()), &(mblockdetails.at(0)));
				dlg.exec();
			}
			else
			{
				std::vector<TransactionDetails> mtrandetails;
				if ((NodeAdapter::instance().doGetTransactions(mhash, mtrandetails)) && (!(mtrandetails.empty())))
				{
					BlockChainsModel::instance().showBlockChainsEnd(mtrandetails.at(0).blockHeight);
					BlockTransactionDetailDialog dlg(&(MainWindow::instance()), &(mtrandetails.at(0)));
					dlg.exec();
				}
				else
				{
					QMessageBox::warning(nullptr, QObject::tr("Warning"), QString("%1 can not be found!").arg(m_ui->lineEdit_height->text()));
					return;
				}
			}
		}
		else
		{
			QMessageBox::warning(nullptr, QObject::tr("Fail"), QString("Wrong argument!"));
			return;
		}
	}

	void BlockChainsFrame::showFirstBlockChains()
	{
		BlockChainsModel::instance().showFirstBlockChains();
	}

	void BlockChainsFrame::showLastestBlockChains()
	{
		BlockChainsModel::instance().showLastestBlockChains();
	}

	void BlockChainsFrame::showNextBlockChains()
	{
		BlockChainsModel::instance().showNextBlockChains();
	}

	void BlockChainsFrame::showLastBlockChains()
	{
		BlockChainsModel::instance().showLastBlockChains();
	}

	void BlockChainsFrame::changeBlocksPerPage()
	{
		BlockChainsModel::instance().changeBlocksPerPage(m_ui->lineEdit_blocks_page->text().toUInt());
	}

	void BlockChainsFrame::turnPage()
	{
		uint32_t _page = m_ui->lineEdit_page->text().toUInt();
		BlockChainsModel::instance().turnPage(_page);
	}

	void BlockChainsFrame::on_customContextMenuRequested(QPoint pos)
	{
		m_strhash = QString(m_ui->m_blockchainsView->indexAt(pos).data(BlockChainsModel::ROLE_HASH).toByteArray().toHex()/*.toUpper()*/);
		popMenu->addAction(action);
		popMenu->exec(QCursor::pos());
	}

	void BlockChainsFrame::rightClickedOperation()
	{
		QApplication::clipboard()->setText(m_strhash);
	}
}
