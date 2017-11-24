
// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <QDialog>

namespace Ui {
class CreateGroupDialog;
}

namespace WalletGui
{
	class CreateGroupDialog : public QDialog
	{
		Q_OBJECT
			Q_DISABLE_COPY(CreateGroupDialog)
			
	public:
		CreateGroupDialog(QWidget* _parent, const std::string& str_add, const std::string& str_vkey, const std::string& str_skey, const bool& _enable);
		~CreateGroupDialog();
		
		QString getAddress() const;
		QString getLabel() const;
		QString getViewKey() const;
		QString getSpendKey() const;
		
	private:
		QScopedPointer<Ui::CreateGroupDialog> m_ui;
	};
}
