
// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <QLabel>
#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QTimer>

#ifndef SQ_CHANGE_TOOLBAR
#define SQ_CHANGE_TOOLBAR 1
#endif

#ifndef SQ_SET_ACTION_SIZE
#define SQ_SET_ACTION_SIZE 1
#endif

#if SQ_CHANGE_TOOLBAR
#include <QToolBar>
#include <QAction>
#endif

class QActionGroup;

#if SQ_SET_ACTION_SIZE
#include <QPushButton>
#include <QWidgetAction>
#endif

namespace Ui {
class MainWindow;
}

namespace WalletGui {

class MainWindow : public QMainWindow {
  Q_OBJECT
  Q_DISABLE_COPY(MainWindow)

public:
  static MainWindow& instance();
  void scrollToTransaction(const QModelIndex& _index);
  void quit();

#if SQ_SET_ACTION_SIZE
  Q_SLOT void func_pb_overview(bool b);
  Q_SLOT void func_pb_deposits(bool b);
  Q_SLOT void func_pb_send(bool b);
  Q_SLOT void func_pb_receive(bool b);
  Q_SLOT void func_pb_transactions(bool b);
  Q_SLOT void func_pb_messages(bool b);
  Q_SLOT void func_pb_sendMessage(bool b);
  Q_SLOT void func_pb_addressBook(bool b);
  Q_SLOT void func_pb_mining(bool b);
  Q_SLOT void func_pb_blockchains(bool b);
  Q_SLOT void func_pb_transactionsPool(bool b);
//#if SQ_ADD_PEER_FRAME
  Q_SLOT void func_pb_peer(bool b);
//#endif
#endif

protected:
  void closeEvent(QCloseEvent* _event) Q_DECL_OVERRIDE;
  bool event(QEvent* _event) Q_DECL_OVERRIDE;

private:
  QScopedPointer<Ui::MainWindow> m_ui;
  QLabel* m_connectionStateIconLabel;
  QLabel* m_encryptionStateIconLabel;
  QLabel* m_synchronizationStateIconLabel;
  QSystemTrayIcon* m_trayIcon;
  QActionGroup* m_tabActionGroup;
  bool m_isAboutToQuit;

#if SQ_CHANGE_TOOLBAR
  QToolBar *toolBar;

#if SQ_SET_ACTION_SIZE
  QPushButton    *m_pb_overview;
  QPushButton    *m_pb_deposits;
  QPushButton    *m_pb_send;
  QPushButton    *m_pb_receive;
  QPushButton    *m_pb_transactions;
  QPushButton    *m_pb_messages;
  QPushButton    *m_pb_sendMessage;
  QPushButton    *m_pb_addressBook;
  QPushButton    *m_pb_mining;
  QPushButton    *m_pb_blockchains;
  QPushButton    *m_pb_transactionsPool;

  QWidgetAction *m_overviewAction;
  QWidgetAction *m_depositsAction;
  QWidgetAction *m_sendAction;
  QWidgetAction *m_receiveAction;
  QWidgetAction *m_transactionsAction;
  QWidgetAction *m_messagesAction;
  QWidgetAction *m_sendMessageAction;
  QWidgetAction *m_addressBookAction;
  QWidgetAction *m_miningAction;
  QWidgetAction *m_blockchainsAction;
  QWidgetAction *m_transactionsPoolAction;

#if SQ_ADD_PEER_FRAME
  QPushButton    *m_pb_peer;
  QWidgetAction *m_peerAction;
#endif

  int m_action_state;
#else
  QAction *m_overviewAction;
  QAction *m_sendAction;
  QAction *m_receiveAction;
  QAction *m_transactionsAction;
  QAction *m_addressBookAction;
  QAction *m_messagesAction;
  QAction *m_sendMessageAction;
  QAction *m_miningAction;
  QAction *m_depositsAction;
  QAction *m_blockchainsAction;
#endif
#endif

  static MainWindow* m_instance;

  MainWindow();
  ~MainWindow();

#if SQ_CHANGE_TOOLBAR
  void initToolBar();
#endif

  void connectToSignals();
  void initUi();

  void minimizeToTray(bool _on);
  void setStatusBarText(const QString& _text);
  void showMessage(const QString& _text, QtMsgType _type);
  void askForWalletPassword(bool _error);
  void encryptedFlagChanged(bool _encrypted);
  void peerCountUpdated(quint64 _peer_count);
  void walletSynchronizationInProgress();
  void walletSynchronized(int _error, const QString& _error_text);
  void walletOpened(bool _error, const QString& _error_text);
  void walletClosed();
  void replyTo(const QModelIndex& _index);
  void payTo(const QModelIndex& _index);

  Q_SLOT void createWallet();
  Q_SLOT void openWallet();
  Q_SLOT void importKey();
  Q_SLOT void backupWallet();
  Q_SLOT void encryptWallet();
  Q_SLOT void aboutQt();
  Q_SLOT void about();
  Q_SLOT void setStartOnLogin(bool _on);
  Q_SLOT void setMinimizeToTray(bool _on);
  Q_SLOT void setCloseToTray(bool _on);
  Q_SLOT void convertWallet();

#ifdef Q_OS_MAC
public:
  void restoreFromDock();

private:
  void installDockHandler();
#elif defined(Q_OS_WIN)
protected:
  void changeEvent(QEvent* _event) Q_DECL_OVERRIDE;

private:
  void trayActivated(QSystemTrayIcon::ActivationReason _reason);
#endif
};

}
