
// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QDateTime>
#include <QFont>
#include <QMetaEnum>
#include <QPixmap>
#include <QTextStream>

#include "CurrencyAdapter.h"
#include "NodeAdapter.h"
#include "GroupModel.h"
#include "WalletAdapter.h"

namespace WalletGui
{
	enum class MessageType : quint8 {INPUT, OUTPUT};
	
#ifdef SQ_UI_TEST
	const int GROUP_MODEL_COLUMN_COUNT = 5;
#else
	const int GROUP_MODEL_COLUMN_COUNT =
		GroupModel::staticMetaObject.enumerator(GroupModel::staticMetaObject.indexOfEnumerator("Columns")).keyCount();
#endif

	GroupModel& GroupModel::instance()
	{
		static GroupModel inst;
		return inst;
	}
	
	GroupModel::GroupModel() : QAbstractItemModel()
	{
		connect(&WalletAdapter::instance(), &WalletAdapter::reloadWalletTransactionsSignal, this, &GroupModel::reloadWalletTransactions, Qt::QueuedConnection);
		connect(&WalletAdapter::instance(), &WalletAdapter::addMessageGroupSignal, this, &GroupModel::reloadWalletTransactions, Qt::QueuedConnection);
		connect(&WalletAdapter::instance(), &WalletAdapter::deleteMessageGroupSignal, this, &GroupModel::reloadWalletTransactions, Qt::QueuedConnection);
	}
	
	GroupModel::~GroupModel()
	{
	}
	
	Qt::ItemFlags GroupModel::flags(const QModelIndex& _index) const
	{
		Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable;
		return flags;
	}
	
	int GroupModel::columnCount(const QModelIndex& _parent) const
	{
		return GROUP_MODEL_COLUMN_COUNT;
	}
	
	int GroupModel::rowCount(const QModelIndex& _parent) const
	{
		return m_groupList.size();
	}
	
	QVariant GroupModel::headerData(int _section, Qt::Orientation _orientation, int _role) const
	{
		if(_orientation != Qt::Horizontal)
		{
			return QVariant();
		}
		
		switch(_role)
		{
		case Qt::DisplayRole:
			switch(_section)
			{
			case COLUMN_NO:
				return tr("No");
			case COLUMN_NAME:
				return tr("Name");
			case COLUMN_ID:
				return tr("ID");
			case COLUMN_ADDRESS:
				return tr("Address");
			case COLUMN_TYPE:
				return tr("Type");
			default:
				break;
			}
			
		case ROLE_COLUMN:
			return _section;
		}
		
		return QVariant();
	}
	
	QVariant GroupModel::data(const QModelIndex& _index, int _role) const
	{
		if(!_index.isValid())
			return QVariant();
		int bh = (_index.row());
		if (bh >= m_groupList.size())
			return QVariant();

		switch(_role)
		{
		case Qt::DisplayRole:
		case Qt::EditRole:
			return getDisplayRole(_index);
		case Qt::DecorationRole:
			return getDecorationRole(_index);
		case Qt::TextAlignmentRole:
			return getAlignmentRole(_index);
		default:
			return getUserRole(_index, _role, bh, (m_groupList[bh]));
		}
		
		return QVariant();
	}
	
	QModelIndex GroupModel::index(int _row, int _column, const QModelIndex& _parent) const
	{
		if(_parent.isValid())
		{
			return QModelIndex();
		}
		
		return createIndex(_row, _column, _row);
	}
	
	QModelIndex GroupModel::parent(const QModelIndex& _index) const
	{
		return QModelIndex();
	}
	
	QVariant GroupModel::getDisplayRole(const QModelIndex& _index) const
	{
		switch(_index.column())
		{
		case COLUMN_NO:
			return QString::number(_index.data(ROLE_NO).value<quint32>());
		case COLUMN_NAME:
			return _index.data(ROLE_NAME);
		case COLUMN_ID:
			return QString::number(_index.data(ROLE_ID).value<quint32>());
		case COLUMN_ADDRESS:
			return _index.data(ROLE_ADDRESS);
		case COLUMN_TYPE:
			return QString::number(_index.data(ROLE_TYPE).value<quint32>());
		default:
			break;
		}
		
		return QVariant();
	}
	
	QVariant GroupModel::getDecorationRole(const QModelIndex& _index) const
	{
		return QVariant();
	}
	
	QVariant GroupModel::getAlignmentRole(const QModelIndex& _index) const
	{
		return headerData(_index.column(), Qt::Horizontal, Qt::TextAlignmentRole);
	}
	
	QVariant GroupModel::getUserRole(const QModelIndex& _index, int _role, int _bh, const CryptoNote::AddressGroup& _group) const
	{
		switch(_role)
		{
		case ROLE_NO:
			return static_cast<quint32>(_bh);
		case ROLE_NAME:
			return QString::fromStdString(_group.m_name);
		case ROLE_ID:
			return (_group.m_id);
		case ROLE_ADDRESS:
			return QString::fromStdString(_group.m_address);
		case ROLE_TYPE:
			return (_group.m_type);
		case ROLE_COLUMN:
			return headerData(_index.column(), Qt::Horizontal, ROLE_COLUMN);
		case ROLE_ROW:
			return _index.row();
		}
		
		return QVariant();
	}
	
	void GroupModel::reloadWalletTransactions()
	{
		reset();
		WalletAdapter::instance().getGroupList(m_groupList);
		quint32 rowCount = m_groupList.size();
		if (rowCount > 0)
		{
			beginInsertRows(QModelIndex(), 0, rowCount - 1);
			endInsertRows();
		}
	}

	void GroupModel::reset()
	{
		beginResetModel();
		m_groupList.clear();
		endResetModel();
	}
}
