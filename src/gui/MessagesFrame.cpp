
// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MessagesFrame.h"
#include "MainWindow.h"
#include "MessageDetailsDialog.h"
#include "MessagesModel.h"
#include "SortedMessagesModel.h"
#include "VisibleMessagesModel.h"

#include "ui_messagesframe.h"

#if SQ_ADD_BM_WALLET
#include "GroupModel.h"
#include "MessagesGroupModel.h"
#include "GroupMessagesModel.h"
#include "SortedMessagesGroupModel.h"
#include "VisibleGroupMessagesModel.h"
#include "WalletAdapter.h"
#endif

namespace WalletGui {
#if SQ_ADD_BM_WALLETUI
	MessagesFrame::MessagesFrame(QWidget* _parent) : QFrame(_parent), m_ui(new Ui::MessagesFrame),
		m_visibleMessagesModel(new VisibleGroupMessagesModel), m_MessagesGroupModel(new MessagesGroupModel) {
		m_ui->setupUi(this);
		m_ui->m_messagesView->setModel(m_visibleMessagesModel.data());
		m_ui->m_messagesView->setStyleSheet("QTreeView{selection-background-color:blue;}");
		m_ui->m_messagesView->header()->resizeSection(GroupMessagesModel::COLUMN_DATE, 140);
		m_ui->m_groupView->setModel(m_MessagesGroupModel.data());
		m_ui->m_groupView->setStyleSheet("QTreeView{selection-background-color:blue;}");
		m_ui->m_groupView->header()->resizeSection(GroupModel::COLUMN_ADDRESS, 140);
		m_ui->m_replyButton->setEnabled(false);

		connect(m_ui->m_messagesView->selectionModel(), &QItemSelectionModel::currentChanged, this, &MessagesFrame::currentMessageChanged);
		connect(m_ui->m_groupView->selectionModel(), &QItemSelectionModel::currentChanged, this, &MessagesFrame::currentGroupChanged);
	}

	void MessagesFrame::currentMessageChanged(const QModelIndex& _currentIndex)
	{
		m_ui->m_replyButton->setEnabled(_currentIndex.isValid() && (!_currentIndex.data(MessagesModel::ROLE_HEADER_REPLY_TO).toString().isEmpty())
			&& ((_currentIndex.data(MessagesModel::ROLE_HEADER_REPLY_TO).toString()) != (WalletAdapter::instance().getAddress())));
	}

#else
MessagesFrame::MessagesFrame(QWidget* _parent) : QFrame(_parent), m_ui(new Ui::MessagesFrame),
m_visibleMessagesModel(new VisibleMessagesModel) {
	m_ui->setupUi(this);
	m_ui->m_messagesView->setModel(m_visibleMessagesModel.data());
	m_ui->m_messagesView->setStyleSheet("QTreeView{selection-background-color:blue;}");
	m_ui->m_messagesView->header()->resizeSection(MessagesModel::COLUMN_DATE, 140);
	m_ui->m_replyButton->setEnabled(false);

	connect(m_ui->m_messagesView->selectionModel(), &QItemSelectionModel::currentChanged, this, &MessagesFrame::currentMessageChanged);
}

void MessagesFrame::currentMessageChanged(const QModelIndex& _currentIndex) {
	m_ui->m_replyButton->setEnabled(_currentIndex.isValid() && !_currentIndex.data(MessagesModel::ROLE_HEADER_REPLY_TO).toString().isEmpty());
}

#endif

MessagesFrame::~MessagesFrame() {
}

void MessagesFrame::messageDoubleClicked(const QModelIndex& _index) {
  if (!_index.isValid()) {
    return;
  }

  MessageDetailsDialog dlg(_index, &MainWindow::instance());
  if (dlg.exec() == QDialog::Accepted) {
    Q_EMIT replyToSignal(dlg.getCurrentMessageIndex());
  }
}

void MessagesFrame::replyClicked() {
  Q_EMIT replyToSignal(m_ui->m_messagesView->selectionModel()->currentIndex());
}

#if SQ_ADD_BM_WALLETUI
void MessagesFrame::currentGroupChanged(const QModelIndex& _currentIndex)
{
	m_visibleMessagesModel->setGroupId(_currentIndex.row());
	QModelIndex index = m_ui->m_messagesView->model()->index(0, 0);
	m_ui->m_messagesView->setCurrentIndex(index);
	if (!(index.isValid()))
		m_ui->m_replyButton->setEnabled(false);
}
#endif
}
