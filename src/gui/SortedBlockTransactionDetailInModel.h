// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <QSortFilterProxyModel>
#include "BlockchainExplorer/BlockchainExplorer.h"
#include "BlockchainExplorerData.h"
using namespace CryptoNote;

namespace WalletGui
{
	class SortedBlockTransactionDetailInModel : public QSortFilterProxyModel
	{
		Q_OBJECT
			Q_DISABLE_COPY(SortedBlockTransactionDetailInModel)
			
	public:
		static SortedBlockTransactionDetailInModel& instance();
		static SortedBlockTransactionDetailInModel& instance(const TransactionDetails *_blockdetail);
		
	protected:
		bool lessThan(const QModelIndex& _left, const QModelIndex& _right) const Q_DECL_OVERRIDE;
		const TransactionDetails *m_transactiondetail;
		
	private:
		SortedBlockTransactionDetailInModel();
		SortedBlockTransactionDetailInModel(const TransactionDetails *_blockdetail);
		~SortedBlockTransactionDetailInModel();
	};
}
