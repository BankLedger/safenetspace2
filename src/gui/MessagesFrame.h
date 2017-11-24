
// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <QFrame>
#include "CryptoNoteConfig.h"

namespace Ui
{
	class MessagesFrame;
}

namespace WalletGui
{
#if SQ_ADD_BM_WALLETUI
	class VisibleGroupMessagesModel;
	class MessagesGroupModel;
#else
	class VisibleMessagesModel;
#endif
	
	class MessagesFrame : public QFrame
	{
		Q_OBJECT
	public:
		MessagesFrame(QWidget* _parent);
		~MessagesFrame();
	
	private:
		QScopedPointer<Ui::MessagesFrame> m_ui;
#if SQ_ADD_BM_WALLETUI
		QScopedPointer<VisibleGroupMessagesModel> m_visibleMessagesModel;
		QScopedPointer<MessagesGroupModel> m_MessagesGroupModel;
		void currentGroupChanged(const QModelIndex& _currentIndex);
#else
		QScopedPointer<VisibleMessagesModel> m_visibleMessagesModel;
#endif
		
		void currentMessageChanged(const QModelIndex& _currentIndex);
		
		Q_SLOT void messageDoubleClicked(const QModelIndex& _index);
		Q_SLOT void replyClicked();
		
	Q_SIGNALS:
		void replyToSignal(const QModelIndex& _index);
	};
}
