// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <QSortFilterProxyModel>

namespace WalletGui
{
	class SortedPeerModel : public QSortFilterProxyModel
	{
		Q_OBJECT
			Q_DISABLE_COPY(SortedPeerModel)
			
	public:
		static SortedPeerModel& instance();

	protected:
		bool lessThan(const QModelIndex& _left, const QModelIndex& _right) const Q_DECL_OVERRIDE;
		
	private:
		SortedPeerModel();
		~SortedPeerModel();
	};
}
