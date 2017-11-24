
// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QCloseEvent>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QSystemTrayIcon>
#include <QTimer>

#include <Common/Base58.h>
#include <Common/Util.h>

#include "AboutDialog.h"
#include "AddressBookModel.h"
#include "AnimatedLabel.h"
#include "ChangePasswordDialog.h"
#include "CurrencyAdapter.h"
#include "ExitWidget.h"
#include "ImportKeyDialog.h"
#include "MainWindow.h"
#include "MessagesModel.h"
#include "NewPasswordDialog.h"
#include "NodeAdapter.h"
#include "PasswordDialog.h"
#include "Settings.h"
#include "WalletAdapter.h"
#include "WalletEvents.h"

#include "ui_mainwindow.h"

namespace WalletGui {

MainWindow* MainWindow::m_instance = nullptr;

MainWindow& MainWindow::instance() {
  if (m_instance == nullptr) {
    m_instance = new MainWindow;
  }

  return *m_instance;
}

MainWindow::MainWindow() : QMainWindow(), m_ui(new Ui::MainWindow), m_trayIcon(nullptr), m_tabActionGroup(new QActionGroup(this)),
  m_isAboutToQuit(false) {
  m_ui->setupUi(this);
  m_connectionStateIconLabel = new QLabel(this);
  m_encryptionStateIconLabel = new QLabel(this);
  m_synchronizationStateIconLabel = new AnimatedLabel(this);

#if SQ_CHANGE_TOOLBAR
  initToolBar();
#endif

  connectToSignals();
  initUi();

  walletClosed();
}

MainWindow::~MainWindow() {
}

#if SQ_CHANGE_TOOLBAR
#if SQ_SET_ACTION_SIZE
void MainWindow::func_pb_overview(bool b)
{
	if (!b)
	{
		if (!(m_pb_overview->isChecked()))
		{
			m_pb_overview->setChecked(true);
			return;
		}
		m_action_state = 1;
	}
	else
	{
		if (m_action_state == 1)
			return;
	}

	m_ui->m_overviewFrame->setVisible(b);
	m_pb_overview->setChecked(b);
}

void MainWindow::func_pb_deposits(bool b)
{
	if (!b)
	{
		if (!(m_pb_deposits->isChecked()))
		{
			m_pb_deposits->setChecked(true);
			return;
		}
		m_action_state = 2;
	}
	else
	{
		if (m_action_state == 2)
			return;
	}
	m_ui->m_depositsFrame->setVisible(b);
	m_pb_deposits->setChecked(b);
}

void MainWindow::func_pb_send(bool b)
{
	if (!b)
	{
		if (!(m_pb_send->isChecked()))
		{
			m_pb_send->setChecked(true);
			return;
		}
		m_action_state = 3;
	}
	else
	{
		if (m_action_state == 3)
			return;
	}

	m_ui->m_sendFrame->setVisible(b);
	m_pb_send->setChecked(b);
}

void MainWindow::func_pb_receive(bool b)
{
	if (!b)
	{
		if (!(m_pb_receive->isChecked()))
		{
			m_pb_receive->setChecked(true);
			return;
		}
		m_action_state = 4;
	}
	else
	{
		if (m_action_state == 4)
			return;
	}
	m_ui->m_receiveFrame->setVisible(b);
	m_pb_receive->setChecked(b);
}

void MainWindow::func_pb_transactions(bool b)
{
	if (!b)
	{
		if (!(m_pb_transactions->isChecked()))
		{
			m_pb_transactions->setChecked(true);
			return;
		}
		m_action_state = 5;
	}
	else
	{
		if (m_action_state == 5)
			return;
	}
	m_ui->m_transactionsFrame->setVisible(b);
	m_pb_transactions->setChecked(b);
}

void MainWindow::func_pb_messages(bool b)
{
	if (!b)
	{
		if (!(m_pb_messages->isChecked()))
		{
			m_pb_messages->setChecked(true);
			return;
		}
		m_action_state = 6;
	}
	else
	{
		if (m_action_state == 6)
			return;
	}
	m_ui->m_messagesFrame->setVisible(b);
	m_pb_messages->setChecked(b);
}

void MainWindow::func_pb_sendMessage(bool b)
{
	if (!b)
	{
		if (!(m_pb_sendMessage->isChecked()))
		{
			m_pb_sendMessage->setChecked(true);
			return;
		}
		m_action_state = 7;
	}
	else
	{
		if (m_action_state == 7)
			return;
	}
	m_ui->m_sendMessageFrame->setVisible(b);
	m_pb_sendMessage->setChecked(b);
}

void MainWindow::func_pb_addressBook(bool b)
{
	if (!b)
	{
		if (!(m_pb_addressBook->isChecked()))
		{
			m_pb_addressBook->setChecked(true);
			return;
		}
		m_action_state = 8;
	}
	else
	{
		if (m_action_state == 8)
			return;
	}
	m_ui->m_addressBookFrame->setVisible(b);
	m_pb_addressBook->setChecked(b);
}

void MainWindow::func_pb_mining(bool b)
{
	if (!b)
	{
		if (!(m_pb_mining->isChecked()))
		{
			m_pb_mining->setChecked(true);
			return;
		}
		m_action_state = 9;
	}
	else
	{
		if (m_action_state == 9)
			return;
	}
	m_ui->m_miningFrame->setVisible(b);
	m_pb_mining->setChecked(b);
}

void MainWindow::func_pb_blockchains(bool b)
{
	if (!b)
	{
		if (!(m_pb_blockchains->isChecked()))
		{
			m_pb_blockchains->setChecked(true);
			return;
		}
		m_action_state = 10;
	}
	else
	{
		if (m_action_state == 10)
			return;
	}
	m_ui->m_blockchainsFrame->setVisible(b);
	m_pb_blockchains->setChecked(b);
}

void MainWindow::func_pb_transactionsPool(bool b)
{
	if (!b)
	{
		if (!(m_pb_transactionsPool->isChecked()))
		{
			m_pb_transactionsPool->setChecked(true);
			return;
		}
		m_action_state = 11;
	}
	else
	{
		if (m_action_state == 11)
			return;
	}
	m_ui->m_transactionsPoolFrame->setVisible(b);
	m_pb_transactionsPool->setChecked(b);
}

#if SQ_ADD_PEER_FRAME
void MainWindow::func_pb_peer(bool b)
{
	if (!b)
	{
		if (!(m_pb_peer->isChecked()))
		{
			m_pb_peer->setChecked(true);
			return;
		}
		m_action_state = 12;
	}
	else
	{
		if (m_action_state == 12)
			return;
	}
	m_ui->m_peerFrame->setVisible(b);
	m_pb_peer->setChecked(b);
}
#endif

void MainWindow::initToolBar()
{
	int size_h, size_w;
	QString str_style;
	size_h = 40;
	size_w = 130;
	str_style = "QPushButton{text-align : left; icon-size:35px 35px;}";
	m_action_state = 0;

	toolBar = new QToolBar(this);
	toolBar->setObjectName(QStringLiteral("toolBar"));
	toolBar->setMovable(false);
	toolBar->setAllowedAreas(Qt::LeftToolBarArea);
	toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	addToolBar(Qt::LeftToolBarArea, toolBar);
	toolBar->setWindowTitle(QApplication::translate("MainWindow", "toolBar", 0));

	m_pb_overview = new QPushButton(this);
	m_pb_overview->setFixedSize(size_w, size_h);
	m_pb_overview->setText(QApplication::translate("MainWindow", "Overview", 0));
	m_pb_overview->setCheckable(true);
	m_pb_overview->setEnabled(true);
	m_pb_overview->setStyleSheet(str_style);
	m_overviewAction = new QWidgetAction(this);
	m_overviewAction->setCheckable(true);
	m_overviewAction->setEnabled(true);
	QIcon icon1;
	icon1.addFile(QStringLiteral(":/icons/overview"), QSize(), QIcon::Normal, QIcon::Off);
	m_pb_overview->setIcon(icon1);
	m_overviewAction->setDefaultWidget(m_pb_overview);
	QObject::connect(m_pb_overview, SIGNAL(toggled(bool)), m_overviewAction, SLOT(setChecked(bool)));
	QObject::connect(m_overviewAction, SIGNAL(toggled(bool)), this, SLOT(func_pb_overview(bool)));
	toolBar->addAction(m_overviewAction);

	m_pb_deposits = new QPushButton(this);
	m_pb_deposits->setFixedSize(size_w, size_h);
	m_pb_deposits->setText(QApplication::translate("MainWindow", "Deposits", 0));
	m_pb_deposits->setCheckable(true);
	m_pb_deposits->setEnabled(true);
	m_pb_deposits->setStyleSheet(str_style);
	m_depositsAction = new QWidgetAction(this);
	m_depositsAction->setCheckable(true);
	m_depositsAction->setEnabled(true);
	QIcon icon2;
	icon2.addFile(QStringLiteral(":/icons/deposits"), QSize(), QIcon::Normal, QIcon::Off);
	m_pb_deposits->setIcon(icon2);
	m_depositsAction->setDefaultWidget(m_pb_deposits);
	QObject::connect(m_pb_deposits, SIGNAL(toggled(bool)), m_depositsAction, SLOT(setChecked(bool)));
	QObject::connect(m_depositsAction, SIGNAL(toggled(bool)), this, SLOT(func_pb_deposits(bool)));
	toolBar->addAction(m_depositsAction);

	m_pb_send = new QPushButton(this);
	m_pb_send->setFixedSize(size_w, size_h);
	m_pb_send->setText(QApplication::translate("MainWindow", "Send", 0));
	m_pb_send->setCheckable(true);
	m_pb_send->setEnabled(true);
	m_pb_send->setStyleSheet(str_style);
	m_sendAction = new QWidgetAction(this);
	m_sendAction->setCheckable(true);
	m_sendAction->setEnabled(true);
	QIcon icon3;
	icon3.addFile(QStringLiteral(":/icons/send"), QSize(), QIcon::Normal, QIcon::Off);
	m_pb_send->setIcon(icon3);
	m_sendAction->setDefaultWidget(m_pb_send);
	QObject::connect(m_pb_send, SIGNAL(toggled(bool)), m_sendAction, SLOT(setChecked(bool)));
	QObject::connect(m_sendAction, SIGNAL(toggled(bool)), this, SLOT(func_pb_send(bool)));
	toolBar->addAction(m_sendAction);

	m_pb_receive = new QPushButton(this);
	m_pb_receive->setFixedSize(size_w, size_h);
	m_pb_receive->setText(QApplication::translate("MainWindow", "Receive", 0));
	m_pb_receive->setCheckable(true);
	m_pb_receive->setEnabled(true);
	m_pb_receive->setStyleSheet(str_style);
	m_receiveAction = new QWidgetAction(this);
	m_receiveAction->setCheckable(true);
	m_receiveAction->setEnabled(true);
	QIcon icon4;
	icon4.addFile(QStringLiteral(":/icons/receive"), QSize(), QIcon::Normal, QIcon::Off);
	m_pb_receive->setIcon(icon4);
	m_receiveAction->setDefaultWidget(m_pb_receive);
	QObject::connect(m_pb_receive, SIGNAL(toggled(bool)), m_receiveAction, SLOT(setChecked(bool)));
	QObject::connect(m_receiveAction, SIGNAL(toggled(bool)), this, SLOT(func_pb_receive(bool)));
	toolBar->addAction(m_receiveAction);

	m_pb_transactions = new QPushButton(this);
	m_pb_transactions->setFixedSize(size_w, size_h);
	m_pb_transactions->setText(QApplication::translate("MainWindow", "Transactions", 0));
	m_pb_transactions->setCheckable(true);
	m_pb_transactions->setEnabled(true);
	m_pb_transactions->setStyleSheet(str_style);
	m_transactionsAction = new QWidgetAction(this);
	m_transactionsAction->setCheckable(true);
	m_transactionsAction->setEnabled(true);
	QIcon icon5;
	icon5.addFile(QStringLiteral(":/icons/transactions"), QSize(), QIcon::Normal, QIcon::Off);
	m_pb_transactions->setIcon(icon5);
	m_transactionsAction->setDefaultWidget(m_pb_transactions);
	QObject::connect(m_pb_transactions, SIGNAL(toggled(bool)), m_transactionsAction, SLOT(setChecked(bool)));
	QObject::connect(m_transactionsAction, SIGNAL(toggled(bool)), this, SLOT(func_pb_transactions(bool)));
	toolBar->addAction(m_transactionsAction);

	m_pb_messages = new QPushButton(this);
	m_pb_messages->setFixedSize(size_w, size_h);
	m_pb_messages->setText(QApplication::translate("MainWindow", "Messages", 0));
	m_pb_messages->setCheckable(true);
	m_pb_messages->setEnabled(true);
	m_pb_messages->setStyleSheet(str_style);
	m_messagesAction = new QWidgetAction(this);
	m_messagesAction->setCheckable(true);
	m_messagesAction->setEnabled(true);
	QIcon icon6;
	icon6.addFile(QStringLiteral(":/icons/messages"), QSize(), QIcon::Normal, QIcon::Off);
	m_pb_messages->setIcon(icon6);
	m_messagesAction->setDefaultWidget(m_pb_messages);
	QObject::connect(m_pb_messages, SIGNAL(toggled(bool)), m_messagesAction, SLOT(setChecked(bool)));
	QObject::connect(m_messagesAction, SIGNAL(toggled(bool)), this, SLOT(func_pb_messages(bool)));
	toolBar->addAction(m_messagesAction);

	m_pb_sendMessage = new QPushButton(this);
	m_pb_sendMessage->setFixedSize(size_w, size_h);
	m_pb_sendMessage->setText(QApplication::translate("MainWindow", "Send message", 0));
	m_pb_sendMessage->setCheckable(true);
	m_pb_sendMessage->setEnabled(true);
	m_pb_sendMessage->setStyleSheet(str_style);
	m_sendMessageAction = new QWidgetAction(this);
	m_sendMessageAction->setCheckable(true);
	m_sendMessageAction->setEnabled(true);
	QIcon icon7;
	icon7.addFile(QStringLiteral(":/icons/send_message"), QSize(), QIcon::Normal, QIcon::Off);
	m_pb_sendMessage->setIcon(icon7);
	m_sendMessageAction->setDefaultWidget(m_pb_sendMessage);
	QObject::connect(m_pb_sendMessage, SIGNAL(toggled(bool)), m_sendMessageAction, SLOT(setChecked(bool)));
	QObject::connect(m_sendMessageAction, SIGNAL(toggled(bool)), this, SLOT(func_pb_sendMessage(bool)));
	toolBar->addAction(m_sendMessageAction);

	m_pb_addressBook = new QPushButton(this);
	m_pb_addressBook->setFixedSize(size_w, size_h);
	m_pb_addressBook->setText(QApplication::translate("MainWindow", "Address Book", 0));
	m_pb_addressBook->setCheckable(true);
	m_pb_addressBook->setEnabled(true);
	m_pb_addressBook->setStyleSheet(str_style);
	m_addressBookAction = new QWidgetAction(this);
	m_addressBookAction->setCheckable(true);
	m_addressBookAction->setEnabled(true);
	QIcon icon8;
	icon8.addFile(QStringLiteral(":/icons/address-book"), QSize(), QIcon::Normal, QIcon::Off);
	m_pb_addressBook->setIcon(icon8);
	m_addressBookAction->setDefaultWidget(m_pb_addressBook);
	QObject::connect(m_pb_addressBook, SIGNAL(toggled(bool)), m_addressBookAction, SLOT(setChecked(bool)));
	QObject::connect(m_addressBookAction, SIGNAL(toggled(bool)), this, SLOT(func_pb_addressBook(bool)));
	toolBar->addAction(m_addressBookAction);

	m_pb_mining = new QPushButton(this);
	m_pb_mining->setFixedSize(size_w, size_h);
	m_pb_mining->setText(QApplication::translate("MainWindow", "Mining", 0));
	m_pb_mining->setCheckable(true);
	m_pb_mining->setEnabled(true);
	m_pb_mining->setStyleSheet(str_style);
	m_miningAction = new QWidgetAction(this);
	m_miningAction->setCheckable(true);
	m_miningAction->setEnabled(true);
	QIcon icon9;
	icon9.addFile(QStringLiteral(":/icons/mining"), QSize(), QIcon::Normal, QIcon::Off);
	m_pb_mining->setIcon(icon9);
	m_miningAction->setDefaultWidget(m_pb_mining);
	QObject::connect(m_pb_mining, SIGNAL(toggled(bool)), m_miningAction, SLOT(setChecked(bool)));
	QObject::connect(m_miningAction, SIGNAL(toggled(bool)), this, SLOT(func_pb_mining(bool)));
	toolBar->addAction(m_miningAction);

	m_pb_blockchains = new QPushButton(this);
	m_pb_blockchains->setFixedSize(size_w, size_h);
	m_pb_blockchains->setText(QApplication::translate("MainWindow", "Blockchain", 0));
	m_pb_blockchains->setCheckable(true);
	m_pb_blockchains->setEnabled(true);
	m_pb_blockchains->setStyleSheet(str_style);
	m_blockchainsAction = new QWidgetAction(this);
	m_blockchainsAction->setCheckable(true);
	m_blockchainsAction->setEnabled(true);
	QIcon icon10;
	icon10.addFile(QStringLiteral(":/icons/blockchains"), QSize(), QIcon::Normal, QIcon::Off);
	m_pb_blockchains->setIcon(icon10);
	m_blockchainsAction->setDefaultWidget(m_pb_blockchains);
	QObject::connect(m_pb_blockchains, SIGNAL(toggled(bool)), m_blockchainsAction, SLOT(setChecked(bool)));
	QObject::connect(m_blockchainsAction, SIGNAL(toggled(bool)), this, SLOT(func_pb_blockchains(bool)));
	toolBar->addAction(m_blockchainsAction);

	m_pb_transactionsPool = new QPushButton(this);
	m_pb_transactionsPool->setFixedSize(size_w, size_h);
	m_pb_transactionsPool->setText(QApplication::translate("MainWindow", "Pool", 0));
	m_pb_transactionsPool->setCheckable(true);
	m_pb_transactionsPool->setEnabled(true);
	m_pb_transactionsPool->setStyleSheet(str_style);
	m_transactionsPoolAction = new QWidgetAction(this);
	m_transactionsPoolAction->setCheckable(true);
	m_transactionsPoolAction->setEnabled(true);
	QIcon icon11;
	icon11.addFile(QStringLiteral(":/icons/transpool"), QSize(), QIcon::Normal, QIcon::Off);
	m_pb_transactionsPool->setIcon(icon11);
	m_transactionsPoolAction->setDefaultWidget(m_pb_transactionsPool);
	QObject::connect(m_pb_transactionsPool, SIGNAL(toggled(bool)), m_transactionsPoolAction, SLOT(setChecked(bool)));
	QObject::connect(m_transactionsPoolAction, SIGNAL(toggled(bool)), this, SLOT(func_pb_transactionsPool(bool)));
	toolBar->addAction(m_transactionsPoolAction);

#if SQ_ADD_PEER_FRAME
	m_pb_peer = new QPushButton(this);
	m_pb_peer->setFixedSize(size_w, size_h);
	m_pb_peer->setText(QApplication::translate("MainWindow", "Peer", 0));
	m_pb_peer->setCheckable(true);
	m_pb_peer->setEnabled(true);
	m_pb_peer->setStyleSheet(str_style);
	m_peerAction = new QWidgetAction(this);
	m_peerAction->setCheckable(true);
	m_peerAction->setEnabled(true);
	QIcon icon12;
	icon12.addFile(QStringLiteral(":/icons/peer"), QSize(), QIcon::Normal, QIcon::Off);
	m_pb_peer->setIcon(icon12);
	m_peerAction->setDefaultWidget(m_pb_peer);
	QObject::connect(m_pb_peer, SIGNAL(toggled(bool)), m_peerAction, SLOT(setChecked(bool)));
	QObject::connect(m_peerAction, SIGNAL(toggled(bool)), this, SLOT(func_pb_peer(bool)));
	toolBar->addAction(m_peerAction);
#endif
}
#else
void MainWindow::initToolBar()
{
	toolBar = new QToolBar(this);
	toolBar->setObjectName(QStringLiteral("toolBar"));
	toolBar->setMovable(false);
	toolBar->setAllowedAreas(Qt::LeftToolBarArea);
	toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	addToolBar(Qt::LeftToolBarArea, toolBar);
	toolBar->setWindowTitle(QApplication::translate("MainWindow", "toolBar", 0));
	toolBar->setStyleSheet("QToolBar::item{ padding:200px;}");

	m_overviewAction = new QAction(this);
	m_overviewAction->setObjectName(QStringLiteral("m_overviewAction"));
	m_overviewAction->setCheckable(true);
	m_overviewAction->setEnabled(true);
	QIcon icon1;
	icon1.addFile(QStringLiteral(":/icons/overview"), QSize(), QIcon::Normal, QIcon::Off);
	m_overviewAction->setIcon(icon1);
	m_overviewAction->setText(QApplication::translate("MainWindow", "Overview", 0));
	QObject::connect(m_overviewAction, SIGNAL(toggled(bool)), m_ui->m_overviewFrame, SLOT(setVisible(bool)));

	m_depositsAction = new QAction(this);
	m_depositsAction->setObjectName(QStringLiteral("m_depositsAction"));
	m_depositsAction->setCheckable(true);
	QIcon icon9;
	icon9.addFile(QStringLiteral(":/icons/deposits"), QSize(), QIcon::Normal, QIcon::Off);
	m_depositsAction->setIcon(icon9);
	m_depositsAction->setText(QApplication::translate("MainWindow", "Deposits", 0));
	QObject::connect(m_depositsAction, SIGNAL(toggled(bool)), m_ui->m_depositsFrame, SLOT(setVisible(bool)));

	m_sendAction = new QAction(this);
	m_sendAction->setObjectName(QStringLiteral("m_sendAction"));
	m_sendAction->setCheckable(true);
	m_sendAction->setEnabled(true);
	QIcon icon2;
	icon2.addFile(QStringLiteral(":/icons/send"), QSize(), QIcon::Normal, QIcon::Off);
	m_sendAction->setIcon(icon2);
	m_sendAction->setText(QApplication::translate("MainWindow", "Send", 0));
	QObject::connect(m_sendAction, SIGNAL(toggled(bool)), m_ui->m_sendFrame, SLOT(setVisible(bool)));

	m_receiveAction = new QAction(this);
	m_receiveAction->setObjectName(QStringLiteral("m_receiveAction"));
	m_receiveAction->setCheckable(true);
	m_receiveAction->setEnabled(true);
	QIcon icon3;
	icon3.addFile(QStringLiteral(":/icons/receive"), QSize(), QIcon::Normal, QIcon::Off);
	m_receiveAction->setIcon(icon3);
	m_receiveAction->setText(QApplication::translate("MainWindow", "Receive", 0));
	QObject::connect(m_receiveAction, SIGNAL(toggled(bool)), m_ui->m_receiveFrame, SLOT(setVisible(bool)));

	m_transactionsAction = new QAction(this);
	m_transactionsAction->setObjectName(QStringLiteral("m_transactionsAction"));
	m_transactionsAction->setCheckable(true);
	m_transactionsAction->setEnabled(true);
	QIcon icon4;
	icon4.addFile(QStringLiteral(":/icons/transactions"), QSize(), QIcon::Normal, QIcon::Off);
	m_transactionsAction->setIcon(icon4);
	m_transactionsAction->setText(QApplication::translate("MainWindow", "Transactions", 0));
	QObject::connect(m_transactionsAction, SIGNAL(toggled(bool)), m_ui->m_transactionsFrame, SLOT(setVisible(bool)));

	m_messagesAction = new QAction(this);
	m_messagesAction->setObjectName(QStringLiteral("m_messagesAction"));
	m_messagesAction->setCheckable(true);
	QIcon icon6;
	icon6.addFile(QStringLiteral(":/icons/messages"), QSize(), QIcon::Normal, QIcon::Off);
	m_messagesAction->setIcon(icon6);
	m_messagesAction->setText(QApplication::translate("MainWindow", "Messages", 0));
	QObject::connect(m_messagesAction, SIGNAL(toggled(bool)), m_ui->m_messagesFrame, SLOT(setVisible(bool)));

	m_sendMessageAction = new QAction(this);
	m_sendMessageAction->setObjectName(QStringLiteral("m_sendMessageAction"));
	m_sendMessageAction->setCheckable(true);
	QIcon icon7;
	icon7.addFile(QStringLiteral(":/icons/send_message"), QSize(), QIcon::Normal, QIcon::Off);
	m_sendMessageAction->setIcon(icon7);
	m_sendMessageAction->setText(QApplication::translate("MainWindow", "Send message", 0));
	QObject::connect(m_sendMessageAction, SIGNAL(toggled(bool)), m_ui->m_sendMessageFrame, SLOT(setVisible(bool)));

	m_addressBookAction = new QAction(this);
	m_addressBookAction->setObjectName(QStringLiteral("m_addressBookAction"));
	m_addressBookAction->setCheckable(true);
	m_addressBookAction->setEnabled(true);
	QIcon icon5;
	icon5.addFile(QStringLiteral(":/icons/address-book"), QSize(), QIcon::Normal, QIcon::Off);
	m_addressBookAction->setIcon(icon5);
	m_addressBookAction->setText(QApplication::translate("MainWindow", "Address Book", 0));
	QObject::connect(m_addressBookAction, SIGNAL(toggled(bool)), m_ui->m_addressBookFrame, SLOT(setVisible(bool)));

	m_miningAction = new QAction(this);
	m_miningAction->setObjectName(QStringLiteral("m_miningAction"));
	m_miningAction->setCheckable(true);
	QIcon icon8;
	icon8.addFile(QStringLiteral(":/icons/mining"), QSize(), QIcon::Normal, QIcon::Off);
	m_miningAction->setIcon(icon8);
	m_miningAction->setText(QApplication::translate("MainWindow", "Mining", 0));
	QObject::connect(m_miningAction, SIGNAL(toggled(bool)), m_ui->m_miningFrame, SLOT(setVisible(bool)));

	m_blockchainsAction = new QAction(this);
	m_blockchainsAction->setObjectName(QStringLiteral("m_blockchainsAction"));
	m_blockchainsAction->setCheckable(true);
	QIcon icon10;
	icon10.addFile(QStringLiteral(":/icons/blockchains"), QSize(), QIcon::Normal, QIcon::Off);
	m_blockchainsAction->setIcon(icon10);
	m_blockchainsAction->setText(QApplication::translate("MainWindow", "BlockChains", 0));
	QObject::connect(m_blockchainsAction, SIGNAL(toggled(bool)), m_ui->m_blockchainsFrame, SLOT(setVisible(bool)));

	toolBar->addAction(m_overviewAction);
	toolBar->addAction(m_depositsAction);
	toolBar->addAction(m_sendAction);
	toolBar->addAction(m_receiveAction);
	toolBar->addAction(m_transactionsAction);
	toolBar->addAction(m_messagesAction);
	toolBar->addAction(m_sendMessageAction);
	toolBar->addAction(m_addressBookAction);
	toolBar->addAction(m_miningAction);
	toolBar->addAction(m_blockchainsAction);
}
#endif
#endif

void MainWindow::connectToSignals() {
  connect(&WalletAdapter::instance(), &WalletAdapter::openWalletWithPasswordSignal, this, &MainWindow::askForWalletPassword, Qt::QueuedConnection);
  connect(&WalletAdapter::instance(), &WalletAdapter::changeWalletPasswordSignal, this, &MainWindow::encryptWallet, Qt::QueuedConnection);
  connect(&WalletAdapter::instance(), &WalletAdapter::walletSynchronizationProgressUpdatedSignal, this, &MainWindow::walletSynchronizationInProgress,
    Qt::QueuedConnection);
  connect(&WalletAdapter::instance(), &WalletAdapter::walletSynchronizationCompletedSignal, this, &MainWindow::walletSynchronized,
    Qt::QueuedConnection);
  connect(&WalletAdapter::instance(), &WalletAdapter::walletStateChangedSignal, this, &MainWindow::setStatusBarText);
  connect(&WalletAdapter::instance(), &WalletAdapter::walletInitCompletedSignal, this, &MainWindow::walletOpened);
  connect(&WalletAdapter::instance(), &WalletAdapter::walletCloseCompletedSignal, this, &MainWindow::walletClosed);
  connect(&WalletAdapter::instance(), &WalletAdapter::walletTransactionCreatedSignal, this, [this]() {
    QApplication::alert(this);
  });
  connect(&NodeAdapter::instance(), &NodeAdapter::peerCountUpdatedSignal, this, &MainWindow::peerCountUpdated, Qt::QueuedConnection);
  connect(m_ui->m_exitAction, &QAction::triggered, qApp, &QApplication::quit);
  connect(m_ui->m_messagesFrame, &MessagesFrame::replyToSignal, this, &MainWindow::replyTo);
  connect(m_ui->m_addressBookFrame, &AddressBookFrame::payToSignal, this, &MainWindow::payTo);
}

void MainWindow::initUi() {
  setWindowTitle(QString("%1 Wallet version %2").arg(CurrencyAdapter::instance().getCurrencyDisplayName()).arg(Settings::instance().getVersion()));
#ifdef Q_OS_WIN32
  if (QSystemTrayIcon::isSystemTrayAvailable()) {
    m_trayIcon = new QSystemTrayIcon(QPixmap(":images/cryptonote"), this);
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::trayActivated);
  }
#endif
  m_ui->m_aboutCryptonoteAction->setText(QString(tr("About %1 Wallet")).arg(CurrencyAdapter::instance().getCurrencyDisplayName()));

  m_ui->m_overviewFrame->hide();
  m_ui->m_sendFrame->hide();
  m_ui->m_receiveFrame->hide();
  m_ui->m_transactionsFrame->hide();
  m_ui->m_addressBookFrame->hide();
  m_ui->m_messagesFrame->hide();
  m_ui->m_sendMessageFrame->hide();
  m_ui->m_miningFrame->hide();
  m_ui->m_depositsFrame->hide();
  m_ui->m_blockchainsFrame->hide();
  m_ui->m_transactionsPoolFrame->hide();
  m_ui->m_peerFrame->hide();

#if SQ_CHANGE_TOOLBAR
  m_tabActionGroup->addAction(m_overviewAction);
  m_tabActionGroup->addAction(m_sendAction);
  m_tabActionGroup->addAction(m_receiveAction);
  m_tabActionGroup->addAction(m_transactionsAction);
  m_tabActionGroup->addAction(m_addressBookAction);
  m_tabActionGroup->addAction(m_messagesAction);
  m_tabActionGroup->addAction(m_sendMessageAction);
  m_tabActionGroup->addAction(m_miningAction); 
  m_tabActionGroup->addAction(m_depositsAction);
  m_tabActionGroup->addAction(m_blockchainsAction);
  m_tabActionGroup->addAction(m_transactionsPoolAction);
#if SQ_ADD_PEER_FRAME
  m_tabActionGroup->addAction(m_peerAction);
#endif
  m_overviewAction->toggle();
#else
  m_tabActionGroup->addAction(m_ui->m_overviewAction);
  m_tabActionGroup->addAction(m_ui->m_sendAction);
  m_tabActionGroup->addAction(m_ui->m_receiveAction);
  m_tabActionGroup->addAction(m_ui->m_transactionsAction);
  m_tabActionGroup->addAction(m_ui->m_addressBookAction);
  m_tabActionGroup->addAction(m_ui->m_messagesAction);
  m_tabActionGroup->addAction(m_ui->m_sendMessageAction);
  m_tabActionGroup->addAction(m_ui->m_miningAction);
  m_tabActionGroup->addAction(m_ui->m_depositsAction);
  m_tabActionGroup->addAction(m_ui->m_blockchainsAction);
#if SQ_ADD_PEER_FRAME
  m_tabActionGroup->addAction(m_peerAction);
#endif

  m_ui->m_overviewAction->toggle();
#endif

  encryptedFlagChanged(false);
  statusBar()->addPermanentWidget(m_connectionStateIconLabel);
  statusBar()->addPermanentWidget(m_encryptionStateIconLabel);
  statusBar()->addPermanentWidget(m_synchronizationStateIconLabel);
  qobject_cast<AnimatedLabel*>(m_synchronizationStateIconLabel)->setSprite(QPixmap(":icons/sync_sprite"), QSize(16, 16), 5, 24);
  m_connectionStateIconLabel->setPixmap(QPixmap(":icons/disconnected").scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

#ifdef Q_OS_MAC
  installDockHandler();
#endif

#ifndef Q_OS_WIN
  m_ui->m_minimizeToTrayAction->deleteLater();
  m_ui->m_closeToTrayAction->deleteLater();
#endif
}

#ifdef Q_OS_WIN
void MainWindow::minimizeToTray(bool _on) {
  if (_on) {
    hide();
    m_trayIcon->show();
  } else {
    showNormal();
    activateWindow();
    m_trayIcon->hide();
  }
}
#endif

void MainWindow::scrollToTransaction(const QModelIndex& _index) {
#if SQ_CHANGE_TOOLBAR
	m_transactionsAction->setChecked(true);
#else
  m_ui->m_transactionsAction->setChecked(true);
#endif
  m_ui->m_transactionsFrame->scrollToTransaction(_index);
}

void MainWindow::quit() {
  if (!m_isAboutToQuit) {
    ExitWidget* exitWidget = new ExitWidget(nullptr);
    exitWidget->show();
    m_isAboutToQuit = true;
    close();
  }
}

#ifdef Q_OS_MAC
void MainWindow::restoreFromDock() {
  if (m_isAboutToQuit) {
    return;
  }

  showNormal();
}
#endif

void MainWindow::closeEvent(QCloseEvent* _event) {
#ifdef Q_OS_WIN
  if (m_isAboutToQuit) {
    QMainWindow::closeEvent(_event);
    return;
  } else if (Settings::instance().isCloseToTrayEnabled()) {
    minimizeToTray(true);
    _event->ignore();
  } else {
    QApplication::quit();
    return;
  }
#elif defined(Q_OS_LINUX)
  if (!m_isAboutToQuit) {
    QApplication::quit();
    return;
  }
#endif
  QMainWindow::closeEvent(_event);

}

#ifdef Q_OS_WIN
void MainWindow::changeEvent(QEvent* _event) {
  QMainWindow::changeEvent(_event);
  if (!QSystemTrayIcon::isSystemTrayAvailable()) {
    QMainWindow::changeEvent(_event);
    return;
  }

  switch (_event->type()) {
  case QEvent::WindowStateChange:
    if(Settings::instance().isMinimizeToTrayEnabled()) {
      minimizeToTray(isMinimized());
    }
    break;
  default:
    break;
  }

  QMainWindow::changeEvent(_event);
}
#endif

bool MainWindow::event(QEvent* _event) {
  switch (static_cast<WalletEventType>(_event->type())) {
    case WalletEventType::ShowMessage:
    showMessage(static_cast<ShowMessageEvent*>(_event)->messageText(), static_cast<ShowMessageEvent*>(_event)->messageType());
    return true;
  }

  return QMainWindow::event(_event);
}

void MainWindow::createWallet() {
  QString filePath = QFileDialog::getSaveFileName(this, tr("New wallet file"),
  #ifdef Q_OS_WIN
      QApplication::applicationDirPath(),
  #else
      QDir::homePath(),
  #endif
      tr("Wallets (*.wallet)")
      );

    if (!filePath.isEmpty() && !filePath.endsWith(".wallet")) {
      filePath.append(".wallet");
    }

    if (!filePath.isEmpty() && !QFile::exists(filePath)) {
      if (WalletAdapter::instance().isOpen()) {
        WalletAdapter::instance().close();
      }

      WalletAdapter::instance().setWalletFile(filePath);
      WalletAdapter::instance().open("");
    }
}

void MainWindow::openWallet() {
	QString filePath = QFileDialog::getOpenFileName(this, tr("Open .wallet/.keys file"),
#ifdef Q_OS_WIN
    QApplication::applicationDirPath(),
#else
    QDir::homePath(),
#endif
    tr("Wallet (*.wallet *.keys)"));
  if (!filePath.isEmpty()) {
    if (WalletAdapter::instance().isOpen()) {
      WalletAdapter::instance().close();
    }

    WalletAdapter::instance().setWalletFile(filePath);
    WalletAdapter::instance().open("");
  }
}

void MainWindow::convertWallet() {
	QString password="";
	QString oldfilePath = QFileDialog::getOpenFileName(this, tr("Open old .dnc.keys file"),
#ifdef Q_OS_WIN
		QApplication::applicationDirPath(),
#else
		QDir::homePath(),
#endif
		tr("Old Wallet (*.dnc.keys *.keys)"));
	
	if (oldfilePath.isEmpty()) {
		return;
	}

	PasswordDialog dlg(false, this);
	if (dlg.exec() == QDialog::Accepted) {
		password = dlg.getPassword();
	}

	QString filePath = QFileDialog::getSaveFileName(this, tr("Convert dnc wallet to"),
#ifdef Q_OS_WIN
		QApplication::applicationDirPath(),
#else
		QDir::homePath(),
#endif
		tr("Wallets (*.wallet)")
		);

	if (filePath.isEmpty()) {
		return;
	}

	if (!filePath.endsWith(".wallet")) {
		filePath.append(".wallet");
	}

	if (QFile::exists(filePath)) {
		QMessageBox::warning(&MainWindow::instance(), tr("File already exists"),
			tr("Warning! File already exists, please select another file name?"));
		return;
	}

	if (WalletAdapter::instance().isOpen()) {
		WalletAdapter::instance().close();
	}

	WalletAdapter::instance().setOldWalletFile(oldfilePath);
	WalletAdapter::instance().setWalletFile(filePath);
	WalletAdapter::instance().convertWallet(filePath, password);
}

void MainWindow::importKey() {
  ImportKeyDialog dlg(this);
  if (dlg.exec() == QDialog::Accepted) {
    QString keyString = dlg.getKeyString().trimmed();
    QString filePath = dlg.getFilePath();
    if (keyString.isEmpty() || filePath.isEmpty()) {
      return;
    }

    if (!filePath.endsWith(".wallet")) {
      filePath.append(".wallet");
    }

    uint64_t addressPrefix;
    std::string data;
    CryptoNote::AccountKeys keys;
    if (Tools::Base58::decode_addr(keyString.toStdString(), addressPrefix, data) && addressPrefix == CurrencyAdapter::instance().getAddressPrefix() &&
      data.size() == sizeof(keys)) {
      std::memcpy(&keys, data.data(), sizeof(keys));
      if (WalletAdapter::instance().isOpen()) {
        WalletAdapter::instance().close();
      }

      WalletAdapter::instance().setWalletFile(filePath);
      WalletAdapter::instance().createWithKeys(keys);
    }
  }
}

void MainWindow::backupWallet() {
  QString filePath = QFileDialog::getSaveFileName(this, tr("Backup wallet to..."),
  #ifdef Q_OS_WIN
      QApplication::applicationDirPath(),
  #else
      QDir::homePath(),
  #endif
      tr("Wallets (*.wallet)")
      );
    if (!filePath.isEmpty() && !filePath.endsWith(".wallet")) {
      filePath.append(".wallet");
    }

    if (!filePath.isEmpty() && !QFile::exists(filePath)) {
      WalletAdapter::instance().backup(filePath);
    }
}

void MainWindow::encryptWallet() {
  if (Settings::instance().isEncrypted()) {
    bool error = false;
    do {
      ChangePasswordDialog dlg(this);
      if (dlg.exec() == QDialog::Rejected) {
        return;
      }

      QString oldPassword = dlg.getOldPassword();
      QString newPassword = dlg.getNewPassword();
      error = !WalletAdapter::instance().changePassword(oldPassword, newPassword);
    } while (error);
  } else {
    NewPasswordDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
      QString password = dlg.getPassword();
      if (password.isEmpty()) {
        return;
      }

      encryptedFlagChanged(WalletAdapter::instance().changePassword("", password));
    }
  }
}

void MainWindow::aboutQt() {
  QMessageBox::aboutQt(this);
}

void MainWindow::setStartOnLogin(bool _on) {
  Settings::instance().setStartOnLoginEnabled(_on);
  m_ui->m_startOnLoginAction->setChecked(Settings::instance().isStartOnLoginEnabled());
}

void MainWindow::setMinimizeToTray(bool _on) {
#ifdef Q_OS_WIN
  Settings::instance().setMinimizeToTrayEnabled(_on);
#endif
}

void MainWindow::setCloseToTray(bool _on) {
#ifdef Q_OS_WIN
  Settings::instance().setCloseToTrayEnabled(_on);
#endif
}

void MainWindow::about() {
  AboutDialog dlg(this);
  dlg.exec();
}

void MainWindow::setStatusBarText(const QString& _text) {
  statusBar()->showMessage(_text);
}

void MainWindow::showMessage(const QString& _text, QtMsgType _type) {
  switch (_type) {
  case QtCriticalMsg:
    QMessageBox::critical(this, tr("Wallet error"), _text);
    break;
  case QtDebugMsg:
    QMessageBox::information(this, tr("Wallet"), _text);
    break;
  default:
    break;
  }
}

void MainWindow::askForWalletPassword(bool _error) {
  PasswordDialog dlg(_error, this);
  if (dlg.exec() == QDialog::Accepted) {
    QString password = dlg.getPassword();
    WalletAdapter::instance().open(password);
  }
}

void MainWindow::encryptedFlagChanged(bool _encrypted) {
  m_ui->m_encryptWalletAction->setEnabled(!_encrypted);
  m_ui->m_changePasswordAction->setEnabled(_encrypted);
  QString encryptionIconPath = _encrypted ? ":icons/encrypted" : ":icons/decrypted";
  QPixmap encryptionIcon = QPixmap(encryptionIconPath).scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  m_encryptionStateIconLabel->setPixmap(encryptionIcon);
  QString encryptionLabelTooltip = _encrypted ? tr("Encrypted") : tr("Not encrypted");
  m_encryptionStateIconLabel->setToolTip(encryptionLabelTooltip);
}

void MainWindow::peerCountUpdated(quint64 _peerCount) {
  QString connectionIconPath = _peerCount > 0 ? ":icons/connected" : ":icons/disconnected";
  QPixmap connectionIcon = QPixmap(connectionIconPath).scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  m_connectionStateIconLabel->setPixmap(connectionIcon);
  m_connectionStateIconLabel->setToolTip(QString(tr("%1 peers").arg(_peerCount)));
}

void MainWindow::walletSynchronizationInProgress() {
  qobject_cast<AnimatedLabel*>(m_synchronizationStateIconLabel)->startAnimation();
  m_synchronizationStateIconLabel->setToolTip(tr("Synchronization in progress"));
}

void MainWindow::walletSynchronized(int _error, const QString& _error_text) {
  QPixmap syncIcon = QPixmap(":icons/synced").scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  qobject_cast<AnimatedLabel*>(m_synchronizationStateIconLabel)->stopAnimation();
  m_synchronizationStateIconLabel->setPixmap(syncIcon);
  QString syncLabelTooltip = _error > 0 ? tr("Not synchronized") : tr("Synchronized");
  m_synchronizationStateIconLabel->setToolTip(syncLabelTooltip);
}

void MainWindow::walletOpened(bool _error, const QString& _error_text) {
  if (!_error) {
    m_encryptionStateIconLabel->show();
    m_synchronizationStateIconLabel->show();
    m_ui->m_backupWalletAction->setEnabled(true);
    encryptedFlagChanged(Settings::instance().isEncrypted());

    QList<QAction*> tabActions = m_tabActionGroup->actions();
    Q_FOREACH(auto action, tabActions) {
      action->setEnabled(true);
    }

#if SQ_CHANGE_TOOLBAR
	m_overviewAction->trigger();
#else
    m_ui->m_overviewAction->trigger();
#endif

	m_ui->m_overviewFrame->show();
  } else {
    walletClosed();
  }
}

void MainWindow::walletClosed() {
  m_ui->m_backupWalletAction->setEnabled(false);
  m_ui->m_encryptWalletAction->setEnabled(false);
  m_ui->m_changePasswordAction->setEnabled(false);
  m_ui->m_overviewFrame->hide();
  m_ui->m_sendFrame->hide();
  m_ui->m_transactionsFrame->hide();
  m_ui->m_addressBookFrame->hide();
  m_ui->m_messagesFrame->hide();
  m_ui->m_sendMessageFrame->hide();
  m_ui->m_miningFrame->hide();
  m_ui->m_depositsFrame->hide();
  m_ui->m_blockchainsFrame->hide();
  m_ui->m_transactionsPoolFrame->hide();
  m_ui->m_peerFrame->hide();
  m_encryptionStateIconLabel->hide();
  m_synchronizationStateIconLabel->hide();

  QList<QAction*> tabActions = m_tabActionGroup->actions();
  Q_FOREACH(auto action, tabActions) {
    action->setEnabled(false);
  }
}

void MainWindow::replyTo(const QModelIndex& _index) {
  m_ui->m_sendMessageFrame->setAddress(_index.data(MessagesModel::ROLE_HEADER_REPLY_TO).toString());

#if SQ_CHANGE_TOOLBAR
  m_sendMessageAction->trigger();
#else
  m_ui->m_sendMessageAction->trigger();
#endif
}

void MainWindow::payTo(const QModelIndex& _index) {
#if SQ_ADD_BM_WALLET
	if(_index.data(AddressBookModel::ROLE_TYPE).toInt() == 0)
	{
		m_ui->m_sendFrame->setAddress(_index.data(AddressBookModel::ROLE_ADDRESS).toString());
		m_sendAction->trigger();
	}
	else
	{
		m_ui->m_sendMessageFrame->setAddress(_index.data(AddressBookModel::ROLE_ADDRESS).toString());
		m_sendMessageAction->trigger();
	}
#else
  m_ui->m_sendFrame->setAddress(_index.data(AddressBookModel::ROLE_ADDRESS).toString());

#if SQ_CHANGE_TOOLBAR
  m_sendAction->trigger();
#else
  m_ui->m_sendAction->trigger();
#endif
#endif
}

#ifdef Q_OS_WIN
void MainWindow::trayActivated(QSystemTrayIcon::ActivationReason _reason) {
  showNormal();
  m_trayIcon->hide();
}
#endif

}
