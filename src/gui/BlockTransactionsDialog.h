// Copyright (c) 2015-2016 darknetspace developer
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <stdint.h>
#include <QFrame>
#include <QStyledItemDelegate>
#include <QMenu>
#include <QAction>
#include "SortedBlockTransactionsModel.h"
#include "BlockTransactionsModel.h"
#include "BlockTransactionsListModel.h"
#include "BlockchainExplorerData.h"
#include "TransactionDetailsDialog.h"
#include "BlockTransactionDetailDialog.h"


namespace Ui
{
	class BlockTransactionsDialog;
}

namespace WalletGui
{
	//using namespace CryptoNote;
	
	class BlockTransactionsListModel;
	
	class BlockTransactionsDialog : public QDialog
	{
		Q_OBJECT
			Q_DISABLE_COPY(BlockTransactionsDialog)
			
	public:
		BlockTransactionsDialog(QWidget* _parent, uint32_t  _height = 0);
		BlockTransactionsDialog(QWidget* _parent, BlockDetails  *_blockdetail);
		~BlockTransactionsDialog();
		
		void scrollToTransaction(const QModelIndex& _index);
		uint32_t  m_height;
		BlockDetails *m_blockdetail;
		TransactionDetails *m_trandetail;

	private:
		QScopedPointer<Ui::BlockTransactionsDialog> m_ui;
		QScopedPointer<BlockTransactionsListModel> m_blocktransactionsModel;

		QMenu *popMenu;
		QAction *action;
		QString m_strhash;

		Q_SLOT void showBlockTransactionDetail(const QModelIndex& _index);
		Q_SLOT void on_customContextMenuRequested(QPoint pos);
		Q_SLOT void rightClickedOperation();
	};
}
