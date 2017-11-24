// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once
#include <stdint.h>
#include <QSortFilterProxyModel>
#include "BlockchainExplorerData.h"
using namespace CryptoNote;

namespace WalletGui
{
	class SortedBlockTransactionsModel : public QSortFilterProxyModel
	{
		Q_OBJECT
			Q_DISABLE_COPY(SortedBlockTransactionsModel)
			
	public:
		static SortedBlockTransactionsModel& instance();
		static SortedBlockTransactionsModel& instance(uint32_t _height);
		static SortedBlockTransactionsModel& instance(BlockDetails  *_blockdetail);
		uint32_t m_height;
		BlockDetails  *m_blockdetail;
		
	protected:
		bool lessThan(const QModelIndex& _left, const QModelIndex& _right) const Q_DECL_OVERRIDE;
		
	private:
		SortedBlockTransactionsModel();
		SortedBlockTransactionsModel(uint32_t _height);
		SortedBlockTransactionsModel(BlockDetails  *_blockdetail);
		~SortedBlockTransactionsModel();
	};
}
