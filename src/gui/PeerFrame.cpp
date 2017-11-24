// Copyright (c) 2015-2016 darknetspace developer
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QClipboard>

#include "MainWindow.h"
#include "SortedPeerModel.h"
#include "PeerFrame.h"
#include "PeerModel.h"
#include "PeerListModel.h"
#include "ui_peerframe.h"

#include "Common/StringTools.h"

namespace WalletGui
{
	PeerFrame::PeerFrame(QWidget* _parent) :
		QFrame(_parent),
		m_ui(new Ui::PeerFrame),
		m_peerlistModel(new PeerListModel)
	{
		m_ui->setupUi(this);
		m_ui->m_peerView->setModel(m_peerlistModel.data());
		m_ui->m_peerView->header()->setSectionResizeMode(PeerModel::COLUMN_NO, QHeaderView::Fixed);
		m_ui->m_peerView->header()->resizeSection(PeerModel::COLUMN_NO, 25);
		m_ui->m_peerView->header()->resizeSection(PeerModel::COLUMN_TYPE, 60);
		m_ui->m_peerView->header()->resizeSection(PeerModel::COLUMN_ID, 150);
		m_ui->m_peerView->header()->setSectionResizeMode(PeerModel::COLUMN_ADDRESS, QHeaderView::Stretch);
		m_ui->m_peerView->header()->resizeSection(PeerModel::COLUMN_ADDRESS, 200);
		m_ui->m_peerView->header()->resizeSection(PeerModel::COLUMN_SEEN, 200);

		m_ui->m_peerView->setContextMenuPolicy(Qt::CustomContextMenu);
		popMenu = new QMenu(this);
		action_hash = new QAction("Copy Hash", this);
		action_payid = new QAction("Copy Payment ID", this);
		connect(action_hash, SIGNAL(triggered()), this, SLOT(rightClickedOperation()));
		connect(action_payid, SIGNAL(triggered()), this, SLOT(rightClickedOperation_PayID()));
	}
	
	PeerFrame::~PeerFrame()
	{
	}
	
	void PeerFrame::scrollToTransaction(const QModelIndex& _index)
	{
		QModelIndex sortedModelIndex = SortedPeerModel::instance().mapFromSource(_index);
		QModelIndex index = static_cast<QSortFilterProxyModel*>(m_ui->m_peerView->model())->mapFromSource(sortedModelIndex);
		m_ui->m_peerView->scrollTo(index);
		m_ui->m_peerView->setFocus();
		m_ui->m_peerView->setCurrentIndex(index);
	}
	
	void PeerFrame::showBlockChainDetails(const QModelIndex& _index)
	{
		if (!_index.isValid())
		{
			return;
		}

		uint32_t height = uint32_t(_index.data(PeerModel::ROLE_NO).toUInt());

		return;
	}
	
	void PeerFrame::on_customContextMenuRequested(QPoint pos)
	{
		popMenu->clear();
		m_strhash = QString(m_ui->m_peerView->indexAt(pos).data(PeerModel::ROLE_ADDRESS).toByteArray().toHex().toUpper());
		if (!(m_strhash.isEmpty()))
			popMenu->addAction(action_hash);
		m_strpayid = QString(m_ui->m_peerView->indexAt(pos).data(PeerModel::ROLE_ID).toByteArray().toHex().toUpper());
		if (!(m_strpayid.isEmpty()))
			popMenu->addAction(action_payid);
		popMenu->exec(QCursor::pos());
	}

	void PeerFrame::rightClickedOperation()
	{
		QApplication::clipboard()->setText(m_strhash);
	}

	void PeerFrame::rightClickedOperation_PayID()
	{
		QApplication::clipboard()->setText(m_strpayid);
	}
}
