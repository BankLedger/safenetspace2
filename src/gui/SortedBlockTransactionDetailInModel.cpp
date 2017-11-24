// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QDateTime>

#include "SortedBlockTransactionDetailInModel.h"
#include "BlockTransactionDetailInModel.h"

#include "BlockchainExplorer/BlockchainExplorer.h"
#include "BlockchainExplorerData.h"	
using namespace CryptoNote;

namespace WalletGui
{
	SortedBlockTransactionDetailInModel& SortedBlockTransactionDetailInModel::instance()
	{
		static SortedBlockTransactionDetailInModel inst;
		return inst;
	}

	SortedBlockTransactionDetailInModel& SortedBlockTransactionDetailInModel::instance(const TransactionDetails *_blockdetail)
	{
		static SortedBlockTransactionDetailInModel inst(_blockdetail);
		inst.setSourceModel(&BlockTransactionDetailInModel::instance(_blockdetail));
		return inst;
	}

	SortedBlockTransactionDetailInModel::SortedBlockTransactionDetailInModel() : QSortFilterProxyModel()
	{
		setSourceModel(&BlockTransactionDetailInModel::instance());
		setDynamicSortFilter(true);
		sort(BlockTransactionDetailInModel::COLUMN_NO, Qt::AscendingOrder);
	}

	SortedBlockTransactionDetailInModel::SortedBlockTransactionDetailInModel(const TransactionDetails *_blockdetail) :
		QSortFilterProxyModel(), m_transactiondetail(_blockdetail)
	{
		setDynamicSortFilter(true);
		sort(BlockTransactionDetailInModel::COLUMN_NO, Qt::AscendingOrder);
	}

	SortedBlockTransactionDetailInModel::~SortedBlockTransactionDetailInModel()
	{
	}

	bool SortedBlockTransactionDetailInModel::lessThan(const QModelIndex& _left, const QModelIndex& _right) const
	{
		uint leftDate = _left.data(BlockTransactionDetailInModel::COLUMN_NO).toUInt();
		uint rightDate = _right.data(BlockTransactionDetailInModel::COLUMN_NO).toUInt();

		return leftDate < rightDate;
	}
}