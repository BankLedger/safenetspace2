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
	class PeerFrame;
}

namespace WalletGui
{
	class PeerListModel;
	
	class PeerFrame : public QFrame
	{
		Q_OBJECT
			Q_DISABLE_COPY(PeerFrame)
			
	public:
		PeerFrame(QWidget* _parent);
		~PeerFrame();
		
		void scrollToTransaction(const QModelIndex& _index);
		
	private:
		QScopedPointer<Ui::PeerFrame> m_ui;
		QScopedPointer<PeerListModel> m_peerlistModel;

		QMenu *popMenu;
		QAction *action_hash, *action_payid;
		QString m_strhash, m_strpayid;

		Q_SLOT void showBlockChainDetails(const QModelIndex& _index);
		Q_SLOT void on_customContextMenuRequested(QPoint pos);
		Q_SLOT void rightClickedOperation();
		Q_SLOT void rightClickedOperation_PayID();
	};
}
