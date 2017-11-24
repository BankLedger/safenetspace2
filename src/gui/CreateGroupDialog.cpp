
// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CreateGroupDialog.h"
#include "ui_creategroupdialog.h"

namespace WalletGui
{
	CreateGroupDialog::CreateGroupDialog(QWidget* _parent, const std::string& str_add, const std::string& str_vkey, const std::string& str_skey, const bool& _enable) : QDialog(_parent), m_ui(new Ui::CreateGroupDialog)
	{
		m_ui->setupUi(this);
//#if SQ_CHANGE_GROUP_PARM
#if 1
		m_ui->m_addressEdit->setText(QString::fromStdString(str_vkey + str_skey + str_add));
		m_ui->m_VKeyEdit->setVisible(false);
		m_ui->m_SKeyEdit->setVisible(false);
		m_ui->label_3->setVisible(false);
		m_ui->label_4->setVisible(false);
#else
		m_ui->m_addressEdit->setText(QString::fromStdString(str_add));
		m_ui->m_VKeyEdit->setText(QString::fromStdString(str_vkey));
		m_ui->m_SKeyEdit->setText(QString::fromStdString(str_skey));
#endif
		m_ui->m_okButton->setEnabled(_enable);
	}

	CreateGroupDialog::~CreateGroupDialog()
	{
	}

	QString CreateGroupDialog::getAddress() const
	{
		return m_ui->m_addressEdit->text();
	}


	QString CreateGroupDialog::getLabel() const
	{
		return m_ui->m_labelEdit->text();
	}

	QString CreateGroupDialog::getViewKey() const
	{
		return m_ui->m_VKeyEdit->text();
	}

	QString CreateGroupDialog::getSpendKey() const
	{
		return m_ui->m_SKeyEdit->text();
	}
}
