// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <QSortFilterProxyModel>
#include "SortedBlockTransactionsModel.h"
#include "BlockTransactionsModel.h"

namespace WalletGui
{
	class BlockTransactionsListModel : public QSortFilterProxyModel
	{
		Q_OBJECT
			
	public:
		BlockTransactionsListModel();
		BlockTransactionsListModel(uint32_t _height);
		BlockTransactionsListModel(BlockDetails  *_blockdetail);
		~BlockTransactionsListModel();
		uint32_t m_height;
		BlockDetails  *m_blockdetail;
	protected:
		bool filterAcceptsColumn(int _sourceColumn, const QModelIndex& _sourceParent) const Q_DECL_OVERRIDE;
	};
}
