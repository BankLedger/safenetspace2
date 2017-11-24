// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QDateTime>
#include <QFont>
#include <QMetaEnum>
#include <QPixmap>
#include <QTextStream>

#include "TransactionsPoolModel.h"
namespace WalletGui
{
	enum class TransactionType : quint8 {MINED, INPUT, OUTPUT, INOUT, DEPOSIT};
	
#ifdef SQ_UI_TEST
	const int POOLTRANSACTION_MODEL_COUNT = 12;
#else
	const int POOLTRANSACTION_MODEL_COUNT =
		TransactionsPoolModel::staticMetaObject.enumerator(TransactionsPoolModel::staticMetaObject.indexOfEnumerator("Columns")).keyCount();
#endif

	TransactionsPoolModel& TransactionsPoolModel::instance()
	{
		static TransactionsPoolModel inst;
		return inst;
	}

	TransactionsPoolModel& TransactionsPoolModel::instance(uint32_t _height)
	{
		static TransactionsPoolModel inst;
		return inst;
	}

	TransactionsPoolModel::TransactionsPoolModel() : QAbstractItemModel()
	{
		m_row = 0;
		connect(&WalletAdapter::instance(), &WalletAdapter::reloadWalletTransactionsSignal, this, &TransactionsPoolModel::ShowPoolTransactions, Qt::QueuedConnection);
		connect(&NodeAdapter::instance(), &NodeAdapter::localBlockchainUpdatedSignal, this, &TransactionsPoolModel::ShowPoolTransactions, Qt::QueuedConnection);
		connect(&NodeAdapter::instance(), &NodeAdapter::nodeInitCompletedSignal1, this, &TransactionsPoolModel::ShowPoolTransactions, Qt::QueuedConnection);
		connect(&WalletAdapter::instance(), &WalletAdapter::walletCloseCompletedSignal, this, &TransactionsPoolModel::reset, Qt::QueuedConnection);
		connect(&NodeAdapter::instance(), &NodeAdapter::lastKnownBlockHeightUpdatedSignal, this, &TransactionsPoolModel::ShowPoolTransactions, Qt::QueuedConnection);
		connect(&WalletAdapter::instance(), &WalletAdapter::walletSynchronizationProgressUpdatedSignal, this, &TransactionsPoolModel::ShowPoolTransactions, Qt::QueuedConnection);
	}
	
	TransactionsPoolModel::~TransactionsPoolModel()
	{
		reset();
	}

	int TransactionsPoolModel::columnCount(const QModelIndex& _parent) const
	{
		return POOLTRANSACTION_MODEL_COUNT;
	}
	
	int TransactionsPoolModel::rowCount(const QModelIndex& _parent) const
	{
		return m_row;
	}

	QVariant TransactionsPoolModel::headerData(int _section, Qt::Orientation _orientation, int _role) const
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
			case COLUMN_DATE:
				return tr("Date");
			case COLUMN_TYPE:
				return tr("Type");
			case COLUMN_HASH:
				return tr("Hash");
			case COLUMN_ADDRESS:
				return tr("Address");
			case COLUMN_AMOUNT:
				return tr("AmountIn");
			case COLUMN_AMOUNTOUT:
				return tr("AmountOut");
			case COLUMN_FEE:
				return tr("Fee");
			case COLUMN_PAYMENT_ID:
				return tr("PaymentID");
			case COLUMN_MESSAGE:
				return tr("Message");
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
	
	QVariant TransactionsPoolModel::data(const QModelIndex& _index, int _role) const
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
			if (bh < m_transactions.size())
				return getUserRoleDetails(_index, _role, bh, &(m_transactions[bh]));
			else
				return QVariant();
		}
		return QVariant();
	}
	
	QModelIndex TransactionsPoolModel::index(int _row, int _column, const QModelIndex& _parent) const
	{
		if(_parent.isValid())
		{
			return QModelIndex();
		}
		
		return createIndex(_row, _column, _row);
	}
	
	QModelIndex TransactionsPoolModel::parent(const QModelIndex& _index) const
	{
		return QModelIndex();
	}
	
	QVariant TransactionsPoolModel::getDisplayRole(const QModelIndex& _index) const
	{
		switch(_index.column())
		{
		case COLUMN_NO:
			return QString::number(_index.data(ROLE_NO).value<quint32>());

		case COLUMN_DATE:
		{
			QDateTime date = _index.data(ROLE_DATE).toDateTime();
			return (date.isNull() || !date.isValid() ? "-" : date.toString("yyyy-MM-dd HH:mm"));
		}

		case COLUMN_ADDRESS:
			return _index.data(ROLE_ADDRESS).toString();

		case COLUMN_HASH:
			return _index.data(ROLE_HASH).toByteArray().toHex()/*.toUpper()*/;
			
		case COLUMN_AMOUNT:
		{
			qint64 amount = _index.data(ROLE_AMOUNT).value<qint64>();
			QString amountStr = CurrencyAdapter::instance().formatAmount(qAbs(amount));
			return (amount < 0 ? "-" + amountStr : amountStr);
		}

		case COLUMN_AMOUNTOUT:
		{
			qint64 amount = _index.data(ROLE_AMOUNTOUT).value<qint64>();
			QString amountStr = CurrencyAdapter::instance().formatAmount(qAbs(amount));
			return (amount < 0 ? "-" + amountStr : amountStr);
		}

		case COLUMN_PAYMENT_ID:
			return _index.data(ROLE_PAYMENT_ID).toByteArray().toHex().toUpper();

		case COLUMN_FEE:
		{
			qint64 fee = _index.data(ROLE_FEE).value<qint64>();
			return CurrencyAdapter::instance().formatAmount(fee);
		}

		case COLUMN_HEIGHT:
		{
			quint64 transactionHeight = _index.data(ROLE_HEIGHT).value<quint64>();
			if (transactionHeight == CryptoNote::WALLET_LEGACY_UNCONFIRMED_TRANSACTION_HEIGHT)
			{
				return QVariant();
			}
			return QString::number(transactionHeight);
		}

		case COLUMN_MESSAGE:
		{
			QString messageString = _index.data(ROLE_MESSAGE).toString();
			QTextStream messageStream(&messageString);
			return messageStream.readLine();
		}
		default:
			break;
		}
		
		return QVariant();
	}

	QVariant TransactionsPoolModel::getDecorationRole(const QModelIndex& _index) const
	{
		return QVariant();
	}
	
	QVariant TransactionsPoolModel::getAlignmentRole(const QModelIndex& _index) const
	{
		return headerData(_index.column(), Qt::Horizontal, Qt::TextAlignmentRole);
	}

	QVariant TransactionsPoolModel::getUserRoleDetails(const QModelIndex& _index, int _role, CryptoNote::BlockHeight _bh, const TransactionDetails* _trandetails) const
	{
		switch (_role)
		{
		case ROLE_NO:
			return static_cast<quint32>(_bh);

		case ROLE_DATE:
			return (_trandetails->timestamp > 0 ? QDateTime::fromTime_t(_trandetails->timestamp) : QDateTime());

		case ROLE_HASH:
			return QByteArray(reinterpret_cast<const char*>(&_trandetails->hash), sizeof(_trandetails->hash));

		case ROLE_ADDRESS:
			return QVariant();

		case ROLE_AMOUNT:
			return static_cast<qint64>(_trandetails->totalInputsAmount);

		case ROLE_AMOUNTOUT:
			return static_cast<qint64>(_trandetails->totalOutputsAmount);

		case ROLE_FEE:
			return static_cast<quint64>(_trandetails->fee);

		case ROLE_HEIGHT:
			return static_cast<quint32>(_trandetails->blockHeight);

		case ROLE_PAYMENT_ID:
			if (_trandetails->paymentId.data[0] != 0x00)
			{
				return QByteArray(reinterpret_cast<const char*>(&_trandetails->paymentId), sizeof(_trandetails->paymentId));
			}
			else
			{
				return QVariant();
			}

		case ROLE_TRANSACTION_ID:
			return QVariant();

		case ROLE_COLUMN:
			return headerData(_index.column(), Qt::Horizontal, ROLE_COLUMN);

		case ROLE_ROW:
			return _index.row();
		}

		return QVariant();
	}

	void TransactionsPoolModel::ShowPoolTransactions()
	{
		reset();

		NodeAdapter::instance().GetPoolTransactions(m_transactions);
		m_row = uint32_t(m_transactions.size());
		if (m_row > 0)
		{
			beginInsertRows(QModelIndex(), 0, m_row - 1);
			endInsertRows();
			return;
		}
	}

	void TransactionsPoolModel::reset()
	{
		beginResetModel();
		m_transactions.clear();
		m_row = 0;
		endResetModel();
	}
	
}
