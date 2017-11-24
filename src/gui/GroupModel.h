
// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <IWalletLegacy.h>
#include "Message.h"

namespace WalletGui
{
	typedef QPair<CryptoNote::TransactionId, Message> TransactionMessageId;

	class GroupModel : public QAbstractItemModel
	{
		Q_OBJECT
			Q_ENUMS(Columns)
			Q_ENUMS(Roles)
	public:
		enum Columns
		{
			COLUMN_NO = 0, COLUMN_NAME, COLUMN_ID, COLUMN_ADDRESS, COLUMN_TYPE
		};
		
		enum Roles
		{
			ROLE_NO = Qt::UserRole, ROLE_NAME, ROLE_ID, ROLE_ADDRESS, ROLE_TYPE, ROLE_COLUMN, ROLE_ROW
		};

		static GroupModel& instance();
		
		Qt::ItemFlags flags(const QModelIndex& _index) const Q_DECL_OVERRIDE;
		int columnCount(const QModelIndex& _parent = QModelIndex()) const Q_DECL_OVERRIDE;
		int rowCount(const QModelIndex& _parent = QModelIndex()) const Q_DECL_OVERRIDE;
		
		QVariant headerData(int _section, Qt::Orientation _orientation, int _role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
		QVariant data(const QModelIndex& _index, int _role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
		QModelIndex index(int _row, int _column, const QModelIndex& _parent = QModelIndex()) const Q_DECL_OVERRIDE;
		QModelIndex	parent(const QModelIndex& _index) const Q_DECL_OVERRIDE;
		
	private:
		int m_row;
		std::vector<CryptoNote::AddressGroup> m_groupList;
		
		GroupModel();
		~GroupModel();
		
		QVariant getDisplayRole(const QModelIndex& _index) const;
		QVariant getDecorationRole(const QModelIndex& _index) const;
		QVariant getAlignmentRole(const QModelIndex& _index) const;
		QVariant getUserRole(const QModelIndex& _index, int _role, int _bh, const CryptoNote::AddressGroup& _group) const;
		
		void reloadWalletTransactions();
		void reset();
	};
}
