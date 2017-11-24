// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QDateTime>
#include <QFont>
#include <QMetaEnum>
#include <QPixmap>

#include "CurrencyAdapter.h"
#include "Message.h"
#include "NodeAdapter.h"
#include "BlockTransactionsModel.h"
#include "WalletAdapter.h"

namespace WalletGui
{
	enum class TransactionType : quint8 {MINED, INPUT, OUTPUT, INOUT, DEPOSIT};
	
#ifdef SQ_UI_TEST
	const int BLOCKTRANSACTIONS_MODEL_COUNT = 5;
#else
	const int BLOCKTRANSACTIONS_MODEL_COUNT =
		BlockTransactionsModel::staticMetaObject.enumerator(BlockTransactionsModel::staticMetaObject.indexOfEnumerator("Columns")).keyCount();
#endif

	BlockTransactionsModel& BlockTransactionsModel::instance()
	{
		static BlockTransactionsModel inst;
		return inst;
	}

	BlockTransactionsModel& BlockTransactionsModel::instance(uint32_t _height)
	{
		static BlockTransactionsModel inst(_height);
		if (inst.m_height != _height)
		{
			inst.m_height = _height;
			inst.showBlockTransactions(_height);
		}
		return inst;
	}

	BlockTransactionsModel& BlockTransactionsModel::instance(BlockDetails  *_blockdetail)
	{
		static BlockTransactionsModel inst(_blockdetail);
		inst.showBlockTransactions(_blockdetail);
		return inst;
	}

	BlockTransactionsModel::BlockTransactionsModel() : QAbstractItemModel()
	{
		m_row = 50;
		m_height = 0;
	}

	BlockTransactionsModel::BlockTransactionsModel(uint32_t _height) : QAbstractItemModel()
	{
		m_row = 50;
		m_height = _height;
		showBlockTransactions(_height);
	}

	BlockTransactionsModel::BlockTransactionsModel(BlockDetails  *_blockdetail) : QAbstractItemModel()
	{
		m_row = 50;
		m_height = 0;
		//showBlockTransactions(_blockdetail);
	}

	BlockTransactionsModel::~BlockTransactionsModel()
	{
	}

	int BlockTransactionsModel::columnCount(const QModelIndex& _parent) const
	{
		return BLOCKTRANSACTIONS_MODEL_COUNT;
	}
	
	int BlockTransactionsModel::rowCount(const QModelIndex& _parent) const
	{
		return m_blockheights.size();
	}

	QVariant BlockTransactionsModel::headerData(int _section, Qt::Orientation _orientation, int _role) const
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
				return tr("No");
			case COLUMN_HASH:
				return tr("Hash");
			case COLUMN_SIZE:
				return tr("Size");
			case COLUMN_FEE:
				return tr("Fee");
			case COLUMN_AMOUNT:
				return tr("Total Amount");
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
	
	QVariant BlockTransactionsModel::data(const QModelIndex& _index, int _role) const
	{
		if(!_index.isValid())
		{
			return QVariant();
		}
		CryptoNote::TransactionNo tno = m_blockheights.value(_index.row()).first;
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
			return getUserRole(_index, _role, tno);
		}
		
		return QVariant();
	}
	
	QModelIndex BlockTransactionsModel::index(int _row, int _column, const QModelIndex& _parent) const
	{
		if(_parent.isValid())
		{
			return QModelIndex();
		}
		
		return createIndex(_row, _column, _row);
	}
	
	QModelIndex BlockTransactionsModel::parent(const QModelIndex& _index) const
	{
		return QModelIndex();
	}
	
	QVariant BlockTransactionsModel::getDisplayRole(const QModelIndex& _index) const
	{
		switch(_index.column())
		{
		case COLUMN_NO:
			return QString::number(_index.data(ROLE_NO).value<quint32>());

		case COLUMN_HASH:
			return _index.data(ROLE_HASH).toByteArray().toHex()/*.toUpper()*/;
			
		case COLUMN_SIZE:
			return QString::number(_index.data(ROLE_SIZE).value<quint32>());
			
		case COLUMN_FEE:
		{
			qint64 amount = _index.data(ROLE_FEE).value<qint64>();
			QString amountStr = CurrencyAdapter::instance().formatAmount(qAbs(amount));
			return (amount < 0 ? "-" + amountStr : amountStr);
		}

		case COLUMN_AMOUNT:
		{
			qint64 amount = _index.data(ROLE_AMOUNT).value<qint64>();
			QString amountStr = CurrencyAdapter::instance().formatAmount(qAbs(amount));
			return (amount < 0 ? "-" + amountStr : amountStr);
		}

		default:
			break;
		}
		
		return QVariant();
	}

	QVariant BlockTransactionsModel::getDecorationRole(const QModelIndex& _index) const
	{
		return QVariant();
	}
	
	QVariant BlockTransactionsModel::getAlignmentRole(const QModelIndex& _index) const
	{
		return headerData(_index.column(), Qt::Horizontal, Qt::TextAlignmentRole);
	}
	
	QVariant BlockTransactionsModel::getUserRole(const QModelIndex& _index, int _role, CryptoNote::TransactionNo _tno) const
	{
		switch(_role)
		{
		case ROLE_NO:
			return static_cast<quint32>(_tno);

		case ROLE_HASH:
			return QByteArray(reinterpret_cast<const char*>(&(m_blockdetail.transactions[_tno].hash)), sizeof(Crypto::Hash));
			
		case ROLE_SIZE:
			return static_cast<uint32_t>(m_blockdetail.transactions[_tno].size);
			
		case ROLE_FEE:
			return static_cast<uint32_t>(m_blockdetail.transactions[_tno].fee);
			
		case ROLE_AMOUNT:
			return static_cast<uint32_t>(m_blockdetail.transactions[_tno].totalOutputsAmount);
			
		case ROLE_COLUMN:
			return headerData(_index.column(), Qt::Horizontal, ROLE_COLUMN);
			
		case ROLE_ROW:
			return _index.row();
		}
		
		return QVariant();
	}
	
	void BlockTransactionsModel::reloadBlockTransactions()
	{
		reset();
		
		quint32 row_count = 0;
		quint64 totalheight = NodeAdapter::instance().getLastLocalBlockHeight();
		for (CryptoNote::BlockHeight height = 0; height < totalheight; ++height)
		{
			appendBlockTransaction(height, row_count);
		}
		
		if (row_count > 0)
		{
			beginInsertRows(QModelIndex(), 0, row_count - 1);
			endInsertRows();
		}
	}

	bool BlockTransactionsModel::showBlockTransactions(uint32_t _height)
	{
		reset();

		uint32_t row_count = 0;

		std::vector<std::vector<BlockDetails>> mblockdetails;
		std::vector<uint32_t> mheight;

		mheight.push_back(_height);
		if (!(NodeAdapter::instance().doGetBlocks(mheight, mblockdetails)))
		{
			return false;
		}
		if (mblockdetails.empty())
		{
			return false;
		}
		m_blockdetail = (mblockdetails.at(0).at(0));
		uint32_t totalsize = uint32_t(m_blockdetail.transactions.size());
		for (CryptoNote::TransactionNo tno = 0; tno < totalsize; ++tno)
		{
			appendBlockTransaction(tno, row_count);
		}

		if (row_count > 0)
		{
			beginInsertRows(QModelIndex(), 0, row_count - 1);
			endInsertRows();
			return true;
		}

		return false;
	}

	bool BlockTransactionsModel::showBlockTransactions(BlockDetails  *_blockdetail)
	{
		reset();

		uint32_t row_count = 0;

		if (_blockdetail == NULL)
			return false;

		m_blockdetail = *_blockdetail;
		uint32_t totalsize = uint32_t(_blockdetail->transactions.size());
		for (CryptoNote::TransactionNo tno = 0; tno < totalsize; ++tno)
		{
			appendBlockTransaction(tno, row_count);
		}

		if (row_count > 0)
		{
			beginInsertRows(QModelIndex(), 0, row_count - 1);
			endInsertRows();
			return true;
		}

		return false;
	}

	
	void BlockTransactionsModel::appendBlockTransaction(CryptoNote::TransactionNo _no, quint32& _insertedRowCount)
	{
		m_blockRow[_no] = qMakePair(_no, _no);
		m_blockheights.append(TransactionNo12(_no, _no));
		++_insertedRowCount;
	}
	
	void BlockTransactionsModel::appendBlockTransaction(CryptoNote::TransactionNo _no)
	{
		if (m_blockRow.contains(_no))
		{
			return;
		}
	
		quint32 oldRowCount = rowCount();
		quint32 insertedRowCount = 0;
		for (quint64 transactionId = m_blockRow.size(); transactionId <= _no; ++transactionId)
		{
			appendBlockTransaction(transactionId, insertedRowCount);
		}
	
		if (insertedRowCount > 0)
		{
			beginInsertRows(QModelIndex(), oldRowCount, oldRowCount + insertedRowCount - 1);
			endInsertRows();
		}
	}
	
	void BlockTransactionsModel::updateBlockTransaction(CryptoNote::TransactionNo _no)
	{
		quint32 firstRow = m_blockRow.value(_no).first;
		quint32 lastRow = firstRow + 1;
		Q_EMIT dataChanged(index(firstRow, COLUMN_NO), index(lastRow, COLUMN_AMOUNT));
	}
	
	void BlockTransactionsModel::lastKnownHeightUpdated(quint64 _height)
	{
		if(rowCount() > 0)
		{
			Q_EMIT dataChanged(index(0, COLUMN_NO), index(rowCount() - 1, COLUMN_AMOUNT));
		}
	}
	
	void BlockTransactionsModel::reset()
	{
		beginResetModel();
		m_blockheights.clear();
		m_blockRow.clear();
		endResetModel();
	}
	
}
