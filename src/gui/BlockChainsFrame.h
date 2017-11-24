// Copyright (c) 2015-2016 darknetspace developer
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <stdint.h>
#include <QFrame>
#include <QStyledItemDelegate>
#include <QMenu>
#include <QAction>


namespace Ui
{
	class BlockChainsFrame;
}

namespace WalletGui
{
	
	class BlockChainsListModel;
	
	class BlockChainsFrame : public QFrame
	{
		Q_OBJECT
			Q_DISABLE_COPY(BlockChainsFrame)
			
	public:
		BlockChainsFrame(QWidget* _parent);
		~BlockChainsFrame();
		
		void scrollToTransaction(const QModelIndex& _index);
		
	private:
		QScopedPointer<Ui::BlockChainsFrame> m_ui;
		QScopedPointer<BlockChainsListModel> m_blockchainsModel;

		QMenu *popMenu;
		QAction *action;
		QString m_strhash;
		bool   m_b_first;

		Q_SLOT void showBlockChainDetails(const QModelIndex& _index);
		Q_SLOT void showFirstBlockChains();
		Q_SLOT void showLastestBlockChains();
		Q_SLOT void showNextBlockChains();
		Q_SLOT void showLastBlockChains();
		Q_SLOT void SearchBlockChains();
		Q_SLOT void changeBlocksPerPage();
		Q_SLOT void turnPage();
		Q_SLOT void on_customContextMenuRequested(QPoint pos);
		Q_SLOT void rightClickedOperation();
	};
}
