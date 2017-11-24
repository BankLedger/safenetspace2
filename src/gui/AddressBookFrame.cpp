
// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QApplication>
#include <QClipboard>

#include "CurrencyAdapter.h"
#include "AddressBookModel.h"
#include "AddressBookFrame.h"
#include "MainWindow.h"
#include "NewAddressDialog.h"
#include "WalletEvents.h"

#include "ui_addressbookframe.h"

#if SQ_ADD_BM_WALLET
#include "NewGroupDialog.h"
#include "WalletAdapter.h"
#include "CreateGroupDialog.h"
#endif

namespace WalletGui
{
#if SQ_ADD_BM_WALLET
	AddressBookFrame::AddressBookFrame(QWidget* _parent) : QFrame(_parent), m_ui(new Ui::AddressBookFrame)
	{
		m_ui->setupUi(this);
		m_ui->m_addressBookView->setModel(&AddressBookModel::instance());

		connect(m_ui->m_addressBookView->selectionModel(), &QItemSelectionModel::currentChanged, this, &AddressBookFrame::currentAddressChanged);
		m_ui->m_addGroupButton->setEnabled(false);
		connect(&WalletAdapter::instance(), &WalletAdapter::walletInitCompletedSignal, this, &AddressBookFrame::walletInitCompleted, Qt::QueuedConnection);
	}

	AddressBookFrame::~AddressBookFrame()
	{
	}

	void AddressBookFrame::addClicked()
	{
		NewAddressDialog dlg(&MainWindow::instance());
		if (dlg.exec() == QDialog::Accepted)
		{
			QString label = dlg.getLabel();
			QString address = dlg.getAddress();
			if (!CurrencyAdapter::instance().validateAddress(address))
			{
				QCoreApplication::postEvent(&MainWindow::instance(), new ShowMessageEvent(tr("Invalid address"), QtCriticalMsg));
				return;
			}
			AddressBookModel::instance().addAddress(label, address);
		}
	}

	void AddressBookFrame::copyClicked()
	{
		QApplication::clipboard()->setText(m_ui->m_addressBookView->currentIndex().data(AddressBookModel::ROLE_ADDRESS).toString());
	}

	void AddressBookFrame::deleteClicked()
	{
		int row = m_ui->m_addressBookView->currentIndex().row();
		AddressBookModel::instance().removeAddress(row);
	}

	void AddressBookFrame::addGroupClicked()
	{
		NewGroupDialog dlg(&MainWindow::instance());
		if (dlg.exec() == QDialog::Accepted)
		{
			QString label = dlg.getLabel();
			QString address = dlg.getAddress();
#if SQ_CHANGE_GROUP_PARM
			std::string str_addr = address.toStdString();
			std::string str_view = str_addr.substr(0, 64);
			std::string str_spend = str_addr.substr(64, 64);
			str_addr = str_addr.substr(128, -1);
			if (WalletAdapter::instance().addBMGroup(str_addr, str_view, str_spend, label.toStdString()))
			{
				AddressBookModel::instance().addAddress(label, QString::fromStdString(str_addr), 1, 1);
#else
			QString viewKey = dlg.getViewKey();
			QString spendKey = dlg.getSpendKey();
			if (WalletAdapter::instance().addBMGroup(address.toStdString(), viewKey.toStdString(), spendKey.toStdString(), label.toStdString()))
			{
				AddressBookModel::instance().addAddress(label, address, 1, 1);
#endif
				WalletAdapter::instance().getGroupList(m_groupList);
				m_ui->m_addGroupButton->setEnabled(m_groupList.size() < BM_GROUP_NUM);
			}
			else
			{
				QCoreApplication::postEvent(&MainWindow::instance(), new ShowMessageEvent(tr("Invalid group"), QtCriticalMsg));
			}
		}
	}

	void AddressBookFrame::deleteGroupClicked()
	{
		int row = m_ui->m_addressBookView->currentIndex().row();
		bool re =WalletAdapter::instance().deleteBMGroup(m_ui->m_addressBookView->currentIndex().data(AddressBookModel::ROLE_ADDRESS).toString().toStdString());
		if (!re)
			return;
		AddressBookModel::instance().removeAddress(row);
		WalletAdapter::instance().getGroupList(m_groupList);
		m_ui->m_addGroupButton->setEnabled(m_groupList.size() < BM_GROUP_NUM);
	}

	void AddressBookFrame::currentAddressChanged(const QModelIndex& _index)
	{
		m_ui->m_copyAddressButton->setEnabled(_index.isValid());
		int type = m_ui->m_addressBookView->currentIndex().data(AddressBookModel::ROLE_TYPE).toInt();
		m_ui->m_deleteAddressButton->setEnabled((_index.isValid()) && (type == 0));
		m_ui->m_deleteGroupButton->setEnabled((_index.isValid()) && (type == 1));
	}

	void AddressBookFrame::createGroupClicked()
	{
		std::string str_add, str_vkey, str_skey;
		if (!(WalletAdapter::instance().createGroup(str_add, str_vkey, str_skey)))
			return;
		bool _enable = m_groupList.size() < BM_GROUP_NUM;
		CreateGroupDialog dlg(&MainWindow::instance(), str_add, str_vkey, str_skey, _enable);
		if (dlg.exec() == QDialog::Accepted)
		{
			QString label = dlg.getLabel();
			if (WalletAdapter::instance().addBMGroup(str_add, str_vkey, str_skey, label.toStdString()))
			{
				AddressBookModel::instance().addAddress(label, QString::fromStdString(str_add), 1, 1);
				WalletAdapter::instance().getGroupList(m_groupList);
				m_ui->m_addGroupButton->setEnabled(m_groupList.size() < BM_GROUP_NUM);
			}
			else
			{
				QCoreApplication::postEvent(&MainWindow::instance(), new ShowMessageEvent(tr("Invalid group"), QtCriticalMsg));
			}
		}
	}

	void AddressBookFrame::walletInitCompleted(int _error, const QString& _error_text)
	{
		if (!_error)
		{
			WalletAdapter::instance().getGroupList(m_groupList);
			m_ui->m_addGroupButton->setEnabled(m_groupList.size() < BM_GROUP_NUM);
		}
	}
#else
	AddressBookFrame::AddressBookFrame(QWidget* _parent) : QFrame(_parent), m_ui(new Ui::AddressBookFrame) {
		m_ui->setupUi(this);
		m_ui->m_addressBookView->setModel(&AddressBookModel::instance());

		connect(m_ui->m_addressBookView->selectionModel(), &QItemSelectionModel::currentChanged, this, &AddressBookFrame::currentAddressChanged);
	}
	AddressBookFrame::~AddressBookFrame() {
	}

	void AddressBookFrame::addClicked() {
		NewAddressDialog dlg(&MainWindow::instance());
		if (dlg.exec() == QDialog::Accepted) {
			QString label = dlg.getLabel();
			QString address = dlg.getAddress();
			if (!CurrencyAdapter::instance().validateAddress(address)) {
				QCoreApplication::postEvent(&MainWindow::instance(), new ShowMessageEvent(tr("Invalid address"), QtCriticalMsg));
				return;
			}

			AddressBookModel::instance().addAddress(label, address);
		}
	}

	void AddressBookFrame::copyClicked() {
		QApplication::clipboard()->setText(m_ui->m_addressBookView->currentIndex().data(AddressBookModel::ROLE_ADDRESS).toString());
	}

	void AddressBookFrame::deleteClicked() {
		int row = m_ui->m_addressBookView->currentIndex().row();
		AddressBookModel::instance().removeAddress(row);
	}

	void AddressBookFrame::currentAddressChanged(const QModelIndex& _index) {
		m_ui->m_copyAddressButton->setEnabled(_index.isValid());
		m_ui->m_deleteAddressButton->setEnabled(_index.isValid());
	}
	void AddressBookFrame::addGroupClicked()
	{
	}

	void AddressBookFrame::deleteGroupClicked()
	{
	}
#endif

void AddressBookFrame::addressDoubleClicked(const QModelIndex& _index) {
  if (!_index.isValid()) {
    return;
  }

  Q_EMIT payToSignal(_index);
}
}
