// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <QSortFilterProxyModel>

namespace WalletGui
{
	class TransactionsPoolListModel : public QSortFilterProxyModel
	{
		Q_OBJECT
			
	public:
		TransactionsPoolListModel();
		~TransactionsPoolListModel();
		
	protected:
		bool filterAcceptsColumn(int _sourceColumn, const QModelIndex& _sourceParent) const Q_DECL_OVERRIDE;
	};
}
