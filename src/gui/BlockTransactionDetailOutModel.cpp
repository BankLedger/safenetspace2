// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QDateTime>
#include <QFont>
#include <QMetaEnum>
#include <QPixmap>
#include <QTextStream>
#include "BlockChainsModel.h"
#include "BlockTransactionDetailOutModel.h"

namespace WalletGui
{
	enum class TransactionType : quint8 {MINED, INPUT, OUTPUT, INOUT, DEPOSIT};
	
#ifdef SQ_UI_TEST
	const int BLOCKTRANSACTIONDETAIL_OUT_MODEL_COLUMN_COUNT = 4;
#else
	const int BLOCKTRANSACTIONDETAIL_OUT_MODEL_COLUMN_COUNT =
		BlockTransactionDetailOutModel::staticMetaObject.enumerator(BlockChainsModel::staticMetaObject.indexOfEnumerator("Columns")).keyCount();
#endif

	BlockTransactionDetailOutModel& BlockTransactionDetailOutModel::instance()
	{
		static BlockTransactionDetailOutModel inst;
		return inst;
	}

	BlockTransactionDetailOutModel& BlockTransactionDetailOutModel::instance(uint32_t _height)
	{
		static BlockTransactionDetailOutModel inst;
		return inst;
	}

	BlockTransactionDetailOutModel& BlockTransactionDetailOutModel::instance(const TransactionDetails  *_transactiondetail)
	{
		static BlockTransactionDetailOutModel inst(_transactiondetail);
		inst.m_transactiondetail = _transactiondetail;
		inst.showBlockChains();
		return inst;
	}

	BlockTransactionDetailOutModel::BlockTransactionDetailOutModel() : QAbstractItemModel()
	{
	}

	BlockTransactionDetailOutModel::BlockTransactionDetailOutModel(const TransactionDetails  *_transactiondetail) : 
		QAbstractItemModel(), m_transactiondetail( _transactiondetail)
	{
	}

	BlockTransactionDetailOutModel::~BlockTransactionDetailOutModel()
	{
		reset();
	}

	int BlockTransactionDetailOutModel::columnCount(const QModelIndex& _parent) const
	{
		return BLOCKTRANSACTIONDETAIL_OUT_MODEL_COLUMN_COUNT;
	}
	
	int BlockTransactionDetailOutModel::rowCount(const QModelIndex& _parent) const
	{
		return m_row;
	}

	QVariant BlockTransactionDetailOutModel::headerData(int _section, Qt::Orientation _orientation, int _role) const
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
			case COLUMN_AMOUNT:
				return tr("Amount");
			case COLUMN_INDEX:
				return tr("Global Index");
			case COLUMN_OUTKEY:
				return tr("Out Key");
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
	
	QVariant BlockTransactionDetailOutModel::data(const QModelIndex& _index, int _role) const
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
			CryptoNote::BlockHeight bh = uint32_t(_index.row());
			return getUserRoleDetails(_index, _role, bh, &(m_transactiondetail->outputs[bh]));
		}
		return QVariant();
	}
	
	QModelIndex BlockTransactionDetailOutModel::index(int _row, int _column, const QModelIndex& _parent) const
	{
		if(_parent.isValid())
		{
			return QModelIndex();
		}
		
		return createIndex(_row, _column, _row);
	}
	
	QModelIndex BlockTransactionDetailOutModel::parent(const QModelIndex& _index) const
	{
		return QModelIndex();
	}
	
	QVariant BlockTransactionDetailOutModel::getDisplayRole(const QModelIndex& _index) const
	{
		switch(_index.column())
		{
		case COLUMN_NO:
		{
			return QString::number(_index.data(ROLE_NO).value<quint32>());
		}
		case COLUMN_AMOUNT:
		{
			qint64 amount = _index.data(ROLE_AMOUNT).value<qint64>();
			QString amountStr = CurrencyAdapter::instance().formatAmount(qAbs(amount));
			return (amount < 0 ? "-" + amountStr : amountStr);
		}
		//	return QString::number((_index.data(ROLE_AMOUNT).value<quint64>()) / 100000000.0 );
			
		case COLUMN_INDEX:
			return QString::number(_index.data(ROLE_INDEX).value<quint32>());
			
		case COLUMN_OUTKEY:
			return _index.data(ROLE_OUTKEY).toByteArray().toHex().toUpper();

		default:
			break;
		}
		
		return QVariant();
	}

	QVariant BlockTransactionDetailOutModel::getDecorationRole(const QModelIndex& _index) const
	{
		return QVariant();
	}
	
	QVariant BlockTransactionDetailOutModel::getAlignmentRole(const QModelIndex& _index) const
	{
		return headerData(_index.column(), Qt::Horizontal, Qt::TextAlignmentRole);
	}
	
	QVariant BlockTransactionDetailOutModel::getUserRoleDetails(const QModelIndex& _index, int _role, CryptoNote::BlockHeight _bh, const TransactionOutputDetails* _outputdetails) const
	{
		switch (_role)
		{
		case ROLE_NO:
			return static_cast<quint32>(_bh);

		case ROLE_AMOUNT:
			return static_cast<quint64>(_outputdetails->amount);

		case ROLE_INDEX:
			return static_cast<quint32>(_outputdetails->globalIndex);

		case ROLE_OUTKEY:
			if (_outputdetails->output.type() == typeid(TransactionOutputToKeyDetails))
			{
				return QByteArray(reinterpret_cast<const char*>(&((boost::get<TransactionOutputToKeyDetails>(_outputdetails->output)).txOutKey)), sizeof(Crypto::PublicKey));
			}
			else if (_outputdetails->output.type() == typeid(TransactionOutputMultisignatureDetails))
			{
				//const TransactionOutputMultisignatureDetails* txOutMultiDetails;
				//txOutMultiDetails = &((boost::get<TransactionOutputMultisignatureDetails>(_outputdetails->output)));
				return QByteArray(reinterpret_cast<const char*>(&((boost::get<TransactionOutputMultisignatureDetails>(_outputdetails->output)).keys[0])), sizeof(Crypto::PublicKey));
			}
			else
			{
				return QVariant();
			}

		case ROLE_COLUMN:
			return headerData(_index.column(), Qt::Horizontal, ROLE_COLUMN);

		case ROLE_ROW:
			return _index.row();
		}

		return QVariant();
	}

	void BlockTransactionDetailOutModel::reloadBlockChains()
	{
		reset();	

		m_row = uint32_t(m_transactiondetail->outputs.size());
		if (m_row > 0)
		{
			beginInsertRows(QModelIndex(), 0, m_row - 1);
			endInsertRows();
		}
	}

	bool BlockTransactionDetailOutModel::showBlockChains(uint32_t _startheight)
	{
		reset();

		m_row = m_transactiondetail->outputs.size();
		if (m_row > 0)
		{
			beginInsertRows(QModelIndex(), 0, m_row - 1);
			endInsertRows();
			return true;
		}

		return false;
	}

	void BlockTransactionDetailOutModel::reset()
	{
		beginResetModel();
		m_row = 0;
		endResetModel();
	}
	
}
