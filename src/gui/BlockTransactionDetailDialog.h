// Copyright (c) 2015-2016 darknetspace developer
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <stdint.h>
#include <QFrame>
#include <QStyledItemDelegate>
#include <QMenu>
#include <QAction>
//#include <QModelIndex>
#include "SortedBlockTransactionDetailInModel.h"
#include "BlockTransactionDetailInModel.h"
#include "BlockTransactionDetailInListModel.h"
#include "SortedBlockTransactionDetailOutModel.h"
#include "BlockTransactionDetailOutModel.h"
#include "BlockTransactionDetailOutListModel.h"
#include "BlockchainExplorerData.h"
#include "TransactionDetailsDialog.h"

namespace Ui
{
	class BlockTransactionDetailDialog;
}

namespace WalletGui
{
	//using namespace CryptoNote;
	
	class BlockTransactionDetailInListModel;
	class BlockTransactionDetailOutListModel;
	
	class BlockTransactionDetailDialog : public QDialog
	{
		Q_OBJECT
			Q_DISABLE_COPY(BlockTransactionDetailDialog)
			
	public:
		BlockTransactionDetailDialog(QWidget* _parent);
		BlockTransactionDetailDialog(QWidget* _parent, const TransactionDetails  *_blockdetail);
		~BlockTransactionDetailDialog();
		void func_inittopinfo(const TransactionDetails *_blockdetail);
		void func_initmenu();
		void func_initview(const TransactionDetails *_blockdetail);
		void func_inittable(const TransactionDetails *_blockdetail);
		
		void scrollToTransaction(const QModelIndex& _index);
		uint32_t  m_height;

	private:
		QScopedPointer<Ui::BlockTransactionDetailDialog> m_ui;
		QScopedPointer<BlockTransactionDetailInListModel> m_blocktransactiondetailinlistModel;
		QScopedPointer<BlockTransactionDetailOutListModel> m_blocktransactiondetailoutlistModel;

		QMenu *popMenuIn, *popMenuOut, *popMenuTable;
		QAction *actionIn, *actionOut, *actionTable;
		QString m_strIn, m_strOut, m_strTable;

		Q_SLOT void on_customContextMenuRequestedIn(QPoint pos);
		Q_SLOT void rightClickedOperationIn();
		Q_SLOT void on_customContextMenuRequestedOut(QPoint pos);
		Q_SLOT void rightClickedOperationOut();
		Q_SLOT void on_customContextMenuRequestedTable(QPoint pos);
		Q_SLOT void rightClickedOperationTable();
	};
}
