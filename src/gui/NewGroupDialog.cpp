
// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "NewGroupDialog.h"

#include "ui_newgroupdialog.h"

namespace WalletGui
{
	NewGroupDialog::NewGroupDialog(QWidget* _parent) : QDialog(_parent), m_ui(new Ui::NewGroupDialog)
	{
		m_ui->setupUi(this);
//#if SQ_CHANGE_GROUP_PARM
#if 1
		m_ui->m_VKeyEdit->setVisible(false);
		m_ui->m_SKeyEdit->setVisible(false);
		m_ui->label_3->setVisible(false);
		m_ui->label_4->setVisible(false);
#endif
	}

	NewGroupDialog::~NewGroupDialog()
	{
	}

	QString NewGroupDialog::getAddress() const
	{
		return m_ui->m_addressEdit->text();
	}
	
	QString NewGroupDialog::getLabel() const
	{
		return m_ui->m_labelEdit->text();
	}

	QString NewGroupDialog::getViewKey() const
	{
		return m_ui->m_VKeyEdit->text();
	}

	QString NewGroupDialog::getSpendKey() const
	{
		return m_ui->m_SKeyEdit->text();
	}
}
