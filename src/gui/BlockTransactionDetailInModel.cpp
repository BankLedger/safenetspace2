// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QDateTime>
#include <QFont>
#include <QMetaEnum>
#include <QPixmap>
#include <QTextStream>
#include "BlockChainsModel.h"
#include "BlockTransactionDetailInModel.h"
namespace WalletGui
{
	enum class TransactionType : quint8 {MINED, INPUT, OUTPUT, INOUT, DEPOSIT};
	
#ifdef SQ_UI_TEST
	const int BLOCKTRANSACTIONDETAIL_IN_MODEL_COLUMN_COUNT = 3;
#else
	const int BLOCKTRANSACTIONDETAIL_IN_MODEL_COLUMN_COUNT =
		BlockTransactionDetailInModel::staticMetaObject.enumerator(BlockChainsModel::staticMetaObject.indexOfEnumerator("Columns")).keyCount();
#endif

	BlockTransactionDetailInModel& BlockTransactionDetailInModel::instance()
	{
		static BlockTransactionDetailInModel inst;
		return inst;
	}

	BlockTransactionDetailInModel& BlockTransactionDetailInModel::instance(uint32_t _height)
	{
		static BlockTransactionDetailInModel inst;
		return inst;
	}

	BlockTransactionDetailInModel& BlockTransactionDetailInModel::instance(const TransactionDetails *_blockdetail)
	{
		static BlockTransactionDetailInModel inst(_blockdetail);
		inst.m_transactiondetail = _blockdetail;
		inst.showBlockChains();
		return inst;
	}

	BlockTransactionDetailInModel::BlockTransactionDetailInModel() : QAbstractItemModel()
	{
		m_row = 0;
	}

	BlockTransactionDetailInModel::BlockTransactionDetailInModel(const TransactionDetails *_blockdetail) :
		QAbstractItemModel(), m_transactiondetail(_blockdetail)
	{
		m_row = 0;
	}

	BlockTransactionDetailInModel::~BlockTransactionDetailInModel()
	{
		reset();
	}

	int BlockTransactionDetailInModel::columnCount(const QModelIndex& _parent) const
	{
		return BLOCKTRANSACTIONDETAIL_IN_MODEL_COLUMN_COUNT;
	}
	
	int BlockTransactionDetailInModel::rowCount(const QModelIndex& _parent) const
	{
		return m_row;
	}

	QVariant BlockTransactionDetailInModel::headerData(int _section, Qt::Orientation _orientation, int _role) const
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
			case COLUMN_KEYIMAGE:
				return tr("Key Image");
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
	
	QVariant BlockTransactionDetailInModel::data(const QModelIndex& _index, int _role) const
	{
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
			return getUserRoleDetails(_index, _role, bh, &(m_transactiondetail->inputs[bh]));
		}

		return QVariant();
	}
	
	QModelIndex BlockTransactionDetailInModel::index(int _row, int _column, const QModelIndex& _parent) const
	{
		if(_parent.isValid())
		{
			return QModelIndex();
		}
		
		return createIndex(_row, _column, _row);
	}
	
	QModelIndex BlockTransactionDetailInModel::parent(const QModelIndex& _index) const
	{
		return QModelIndex();
	}
	
	QVariant BlockTransactionDetailInModel::getDisplayRole(const QModelIndex& _index) const
	{
		switch(_index.column())
		{
		case COLUMN_NO:
			return QString::number(_index.data(ROLE_NO).value<quint32>());
		case COLUMN_AMOUNT:
		{
			qint64 amount = _index.data(ROLE_AMOUNT).value<qint64>();
			QString amountStr = CurrencyAdapter::instance().formatAmount(qAbs(amount));
			return (amount < 0 ? "-" + amountStr : amountStr);
		}
		//	return QString::number((_index.data(ROLE_AMOUNT).value<quint64>()) / 100000000.0 );
		case COLUMN_KEYIMAGE:
			return _index.data(ROLE_KEYIMAGE).toByteArray().toHex().toUpper();
		default:
			break;
		}
		
		return QVariant();
	}

	QVariant BlockTransactionDetailInModel::getDecorationRole(const QModelIndex& _index) const
	{
		return QVariant();
	}
	
	QVariant BlockTransactionDetailInModel::getAlignmentRole(const QModelIndex& _index) const
	{
		return headerData(_index.column(), Qt::Horizontal, Qt::TextAlignmentRole);
	}

	QVariant BlockTransactionDetailInModel::getUserRoleDetails(const QModelIndex& _index, int _role, CryptoNote::BlockHeight _bh, const TransactionInputDetails* _inputdetails) const
	{
		switch (_role)
		{
		case ROLE_NO:
			return static_cast<quint32>(_bh);

		case ROLE_AMOUNT:
			return static_cast<quint64>(_inputdetails->amount);

		case ROLE_KEYIMAGE:
			if (_inputdetails->input.type() == typeid(TransactionInputGenerateDetails))
			{
				return QVariant();
			}
			else if (_inputdetails->input.type() == typeid(TransactionInputToKeyDetails))
			{
				return QByteArray(reinterpret_cast<const char*>(&((boost::get<TransactionInputToKeyDetails>(_inputdetails->input)).keyImage)), sizeof(Crypto::KeyImage));
			}
			else if (_inputdetails->input.type() == typeid(TransactionInputMultisignatureDetails))
			{
				return QByteArray(reinterpret_cast<const char*>(&((boost::get<TransactionInputMultisignatureDetails>(_inputdetails->input)).output.transactionHash)), sizeof(Crypto::Hash));
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

	void BlockTransactionDetailInModel::reloadBlockChains()
	{
		reset();
		m_row = uint32_t(m_transactiondetail->inputs.size());
		
		if (m_row > 0)
		{
			beginInsertRows(QModelIndex(), 0, m_row - 1);
			endInsertRows();
		}
	}

	bool BlockTransactionDetailInModel::showBlockChains(uint32_t _startheight)
	{
		reset();

		m_row = uint32_t(m_transactiondetail->inputs.size());
		if (m_row > 0)
		{
			beginInsertRows(QModelIndex(), 0, m_row - 1);
			endInsertRows();
			return true;
		}

		return false;
	}
			
	void BlockTransactionDetailInModel::reset()
	{
		beginResetModel();
		m_row = 0;
		endResetModel();
	}
	
}
