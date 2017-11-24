// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QDateTime>
#include <QFont>
#include <QMetaEnum>
#include <QPixmap>
#include <QTextStream>

#include "PeerModel.h"
namespace WalletGui
{	
#ifdef SQ_UI_TEST
	const int PEER_MODEL_COUNT = 5;
#else
	const int PEER_MODEL_COUNT =
		PeerModel::staticMetaObject.enumerator(PeerModel::staticMetaObject.indexOfEnumerator("Columns")).keyCount();
#endif

	PeerModel& PeerModel::instance()
	{
		static PeerModel inst;
		return inst;
	}

	PeerModel& PeerModel::instance(uint32_t _height)
	{
		static PeerModel inst;
		return inst;
	}

	PeerModel::PeerModel() : QAbstractItemModel()
	{
		m_row = 0;
		connect(&WalletAdapter::instance(), &WalletAdapter::reloadWalletTransactionsSignal, this, &PeerModel::ShowPeer, Qt::QueuedConnection);
		connect(&NodeAdapter::instance(), &NodeAdapter::localBlockchainUpdatedSignal, this, &PeerModel::ShowPeer, Qt::QueuedConnection);
		connect(&NodeAdapter::instance(), &NodeAdapter::nodeInitCompletedSignal1, this, &PeerModel::ShowPeer, Qt::QueuedConnection);
		connect(&WalletAdapter::instance(), &WalletAdapter::walletCloseCompletedSignal, this, &PeerModel::reset, Qt::QueuedConnection);
		connect(&NodeAdapter::instance(), &NodeAdapter::lastKnownBlockHeightUpdatedSignal, this, &PeerModel::ShowPeer, Qt::QueuedConnection);
		connect(&WalletAdapter::instance(), &WalletAdapter::walletSynchronizationProgressUpdatedSignal, this, &PeerModel::ShowPeer, Qt::QueuedConnection);
	}
	
	PeerModel::~PeerModel()
	{
		reset();
	}

	int PeerModel::columnCount(const QModelIndex& _parent) const
	{
		return PEER_MODEL_COUNT;
	}
	
	int PeerModel::rowCount(const QModelIndex& _parent) const
	{
		return m_row;
	}

	QVariant PeerModel::headerData(int _section, Qt::Orientation _orientation, int _role) const
	{
		if(_orientation != Qt::Horizontal)
		{
			return QVariant();
		}
		
		switch(_role)
		{
		case Qt::DisplayRole:
			switch (_section)
			{
			case COLUMN_NO:
				return tr("");
			case COLUMN_TYPE:
				return tr("Type");
			case COLUMN_ID:
				return tr("ID");
			case COLUMN_ADDRESS:
				return tr("Address");
			case COLUMN_SEEN:
				return tr("Last seen");
			default:
				break;
			}
			
		case Qt::TextAlignmentRole:
			return QVariant();
			
		case ROLE_COLUMN:
			return _section;
		}
		
		return QVariant();
	}
	
	QVariant PeerModel::data(const QModelIndex& _index, int _role) const
	{
		if(!_index.isValid())
		{
			return QVariant();
		}
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
			int bh = (_index.row());
			if (bh < m_row)
				return getUserRoleDetails(_index, _role, bh, &(m_peerlist[bh]));
			else
				return QVariant();
		}
		return QVariant();
	}
	
	QModelIndex PeerModel::index(int _row, int _column, const QModelIndex& _parent) const
	{
		if(_parent.isValid())
		{
			return QModelIndex();
		}
		
		return createIndex(_row, _column, _row);
	}
	
	QModelIndex PeerModel::parent(const QModelIndex& _index) const
	{
		return QModelIndex();
	}
	
	QVariant PeerModel::getDisplayRole(const QModelIndex& _index) const
	{
		switch(_index.column())
		{
		case COLUMN_NO:
			return QString::number(_index.data(ROLE_NO).value<quint32>());

		case COLUMN_TYPE:
			return _index.data(ROLE_TYPE);

		case COLUMN_ID:
			return _index.data(ROLE_ID);

		case COLUMN_ADDRESS:
			return _index.data(ROLE_ADDRESS);

		case COLUMN_SEEN:
			return _index.data(ROLE_SEEN);
			
		default:
			break;
		}
		
		return QVariant();
	}

	QVariant PeerModel::getDecorationRole(const QModelIndex& _index) const
	{
		return QVariant();
	}
	
	QVariant PeerModel::getAlignmentRole(const QModelIndex& _index) const
	{
		return headerData(_index.column(), Qt::Horizontal, Qt::TextAlignmentRole);
	}

	QVariant PeerModel::getUserRoleDetails(const QModelIndex& _index, int _role, int _bh, const PeerListUI* _peer) const
	{
		switch (_role)
		{
		case ROLE_NO:
			return static_cast<quint32>(_bh);

		case ROLE_TYPE:
			return (_peer->strtype);

		case ROLE_ID:
			return (_peer->id);

		case ROLE_ADDRESS:
			return (_peer->addr);

		case ROLE_SEEN:
			return (_peer->seen);

		case ROLE_COLUMN:
			return headerData(_index.column(), Qt::Horizontal, ROLE_COLUMN);

		case ROLE_ROW:
			return _index.row();
		}

		return QVariant();
	}

	void PeerModel::ShowPeer()
	{
		reset();

		std::list<PeerlistEntry> pl_white;
		std::list<PeerlistEntry> pl_gray;
		NodeAdapter::instance().GetPeerList(pl_white, pl_gray);
		time_t now_time = 0;
		time(&now_time);
		std::stringstream ss;
		ss << std::setfill('0') << std::setw(8) << std::hex << std::noshowbase;
		for (const auto& pe : pl_white)
		{
			PeerListUI peer;
			ss << pe.id;
			peer.id = QString::fromStdString(ss.str());
			ss.str("");
			ss << pe.adr;
			peer.addr = QString::fromStdString(ss.str());
			ss.str("");
			ss << pe.adr;
			peer.seen = QString::fromStdString(Common::timeIntervalToString(now_time - pe.last_seen));
			peer.strtype = "White";
			m_peerlist.push_back(peer);
		}
		for (const auto& pe : pl_gray)
		{
			PeerListUI peer;
			ss << pe.id;
			peer.id = QString::fromStdString(ss.str());
			ss.str("");
			ss << pe.adr;
			peer.addr = QString::fromStdString(ss.str());
			ss.str("");
			ss << pe.adr;
			peer.seen = QString::fromStdString(Common::timeIntervalToString(now_time - pe.last_seen));
			peer.strtype = "Gray";
			m_peerlist.push_back(peer);
		}
		m_row = m_peerlist.size();
		if (m_row > 0)
		{
			beginInsertRows(QModelIndex(), 0, m_row - 1);
			endInsertRows();
			return;
		}
	}

	void PeerModel::reset()
	{
		beginResetModel();
		m_peerlist.clear();
		m_row = 0;
		endResetModel();
	}
	
}
