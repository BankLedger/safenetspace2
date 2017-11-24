// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

#include "CurrencyAdapter.h"
#include "Message.h"
#include "NodeAdapter.h"
#include "WalletAdapter.h"

#include "P2p/PeerListManager.h"

struct PeerListUI
{
	QString id;
	QString addr;
	QString seen;
	QString strtype;
};

namespace WalletGui
{
	typedef QPair<CryptoNote::TransactionId, CryptoNote::TransferId> TransactionTransferId; 
	typedef QPair<CryptoNote::BlockHeight, CryptoNote::BlockHeight2> BlockHeight;
	
	class PeerModel : public QAbstractItemModel
	{
		Q_OBJECT
			Q_ENUMS(Columns)
			Q_ENUMS(Roles)
			
	public:
		enum Columns {
			COLUMN_NO = 0, COLUMN_TYPE, COLUMN_ID, COLUMN_ADDRESS, COLUMN_SEEN
		};

		enum Roles {
			ROLE_NO = Qt::UserRole, ROLE_TYPE, ROLE_ID, ROLE_ADDRESS, ROLE_SEEN, ROLE_COLUMN, ROLE_ROW
		};
		static PeerModel& instance();
		static PeerModel& instance(uint32_t _height);
		
		int columnCount(const QModelIndex& _parent = QModelIndex()) const Q_DECL_OVERRIDE;
		int rowCount(const QModelIndex& _parent = QModelIndex()) const Q_DECL_OVERRIDE;
		
		QVariant headerData(int _section, Qt::Orientation _orientation, int _role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
		QVariant data(const QModelIndex& _index, int _role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
		QModelIndex index(int _row, int _column, const QModelIndex& _parent = QModelIndex()) const Q_DECL_OVERRIDE;
		QModelIndex	parent(const QModelIndex& _index) const Q_DECL_OVERRIDE;

		Q_SLOT void ShowPeer();
	
	private:
		uint32_t m_row;
		std::vector<PeerListUI> m_peerlist;
		
		PeerModel();
		~PeerModel();
		
		QVariant getDisplayRole(const QModelIndex& _index) const;
		QVariant getDecorationRole(const QModelIndex& _index) const;
		QVariant getAlignmentRole(const QModelIndex& _index) const;
		QVariant getUserRoleDetails(const QModelIndex& _index, int _role, int _bh, const PeerListUI* _peer) const;
		
		void reset();
	};
}
