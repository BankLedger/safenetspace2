// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <QSortFilterProxyModel>
#include "BlockchainExplorer/BlockchainExplorer.h"
#include "BlockchainExplorerData.h"
#include "LoggerAdapter.h"
using namespace CryptoNote;

namespace WalletGui
{
	class SortedBlockTransactionDetailOutModel : public QSortFilterProxyModel
	{
		Q_OBJECT
			Q_DISABLE_COPY(SortedBlockTransactionDetailOutModel)
			
	public:
		static SortedBlockTransactionDetailOutModel& instance();
		static SortedBlockTransactionDetailOutModel& instance(const TransactionDetails* _transactiondetail);
		
	protected:
		bool lessThan(const QModelIndex& _left, const QModelIndex& _right) const Q_DECL_OVERRIDE;
		
	private:
		SortedBlockTransactionDetailOutModel();
		SortedBlockTransactionDetailOutModel(const TransactionDetails  *_transactiondetail);
		~SortedBlockTransactionDetailOutModel();
	};
}
