// Copyright (c) 2015-2016 darknetspace developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QDateTime>
#include <QFont>
#include <QMetaEnum>
#include <QPixmap>
#include <QTextStream>

#include "BlockChainsModel.h"
namespace WalletGui
{
	enum class TransactionType : quint8 {MINED, INPUT, OUTPUT, INOUT, DEPOSIT};
	
#ifdef SQ_UI_TEST
	const int BLOCKCHAINS_MODEL_COUNT = 5;
#else
	const int BLOCKCHAINS_MODEL_COUNT =
		BlockChainsModel::staticMetaObject.enumerator(BlockChainsModel::staticMetaObject.indexOfEnumerator("Columns")).keyCount();
#endif

	BlockChainsModel& BlockChainsModel::instance()
	{
		static BlockChainsModel inst;
		return inst;
	}

	BlockChainsModel& BlockChainsModel::instance(uint32_t _height)
	{
		static BlockChainsModel inst;
		return inst;
	}

	BlockChainsModel::BlockChainsModel() : QAbstractItemModel()
	{
		connect(&WalletAdapter::instance(), &WalletAdapter::reloadWalletTransactionsSignal, this, &BlockChainsModel::showLastestBlockChains, Qt::QueuedConnection);
		connect(&NodeAdapter::instance(), &NodeAdapter::localBlockchainUpdatedSignal, this, &BlockChainsModel::updateBlockChains, Qt::QueuedConnection);
		connect(&NodeAdapter::instance(), &NodeAdapter::nodeInitCompletedSignal1, this, &BlockChainsModel::showLastestBlockChains, Qt::QueuedConnection);
		connect(&WalletAdapter::instance(), &WalletAdapter::walletCloseCompletedSignal, this, &BlockChainsModel::reset, Qt::QueuedConnection); 
		connect(&NodeAdapter::instance(), &NodeAdapter::lastKnownBlockHeightUpdatedSignal, this, &BlockChainsModel::updateBlockChains, Qt::QueuedConnection);
		connect(&WalletAdapter::instance(), &WalletAdapter::walletSynchronizationProgressUpdatedSignal, this, &BlockChainsModel::updateBlockChains, Qt::QueuedConnection);
		m_row = 60;
		m_start_height = 0;
		m_b_lasted = true;
	}
	
	BlockChainsModel::~BlockChainsModel()
	{
		reset();
	}

	int BlockChainsModel::columnCount(const QModelIndex& _parent) const
	{
		return BLOCKCHAINS_MODEL_COUNT;
	}
	
	int BlockChainsModel::rowCount(const QModelIndex& _parent) const
	{
		return m_blockheights.size();
	}

	QVariant BlockChainsModel::headerData(int _section, Qt::Orientation _orientation, int _role) const
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
			case COLUMN_HEIGHT:
				return tr("Height");
			case COLUMN_TRANSACTIONS:
				return tr("Transactions");
			case COLUMN_TIMESTAMP:
				return tr("Date");
			case COLUMN_HASH:
				return tr("Hash");
			case COLUMN_SIZE:
				return tr("Size");
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
	
	QVariant BlockChainsModel::data(const QModelIndex& _index, int _role) const
	{
		if(!_index.isValid())
		{
			return QVariant();
		}
#if SQ_DOGETBLOCK_TEST
#if 1
		if(m_blockdetails.empty())
		{
			return QVariant();
		}
		const BlockDetails *blockdetails;
		CryptoNote::BlockHeight bh = m_blockheights.value(_index.row()).first;
		blockdetails = &(m_blockdetails.at(_index.row()));
#else
		std::vector<std::vector<BlockDetails>> mblockdetails;
		std::vector<uint32_t> mheight;
		CryptoNote::BlockHeight bh = m_blockheights.value(_index.row()).first;

		mheight.push_back(bh);
		if (!(NodeAdapter::instance().doGetBlocks(mheight, mblockdetails)))
		{
			return QVariant();
		}
		if (mblockdetails.empty())
		{
			return QVariant();
		}
		BlockDetails *blockdetails;
		blockdetails = &(mblockdetails.at(0).at(0));
#endif
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
			return getUserRoleDetails(_index, _role, bh, blockdetails);
		}
#else
		CryptoNote::BlockChain blockchain;
		CryptoNote::BlockHeight bh = m_blockheights.value(_index.row()).first;
		
		if ((bh > NodeAdapter::instance().getLastLocalBlockHeight()))
		{
			return QVariant();
		}
		
		if (!(NodeAdapter::instance().getBlockByHeight(bh, blockchain)))
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
			return getUserRole(_index, _role, bh, blockchain);
		}
#endif

		return QVariant();
	}
	
	QModelIndex BlockChainsModel::index(int _row, int _column, const QModelIndex& _parent) const
	{
		if(_parent.isValid())
		{
			return QModelIndex();
		}
		
		return createIndex(_row, _column, _row);
	}
	
	QModelIndex BlockChainsModel::parent(const QModelIndex& _index) const
	{
		return QModelIndex();
	}
	
	QVariant BlockChainsModel::getDisplayRole(const QModelIndex& _index) const
	{
		switch(_index.column())
		{
		case COLUMN_TIMESTAMP:
		{
			QDateTime date = _index.data(ROLE_DATE).toDateTime();
			return (date.isNull() || !date.isValid() ? "-" : date.toString("yyyy-MM-dd HH:mm"));
		}
		case COLUMN_HASH:
			return _index.data(ROLE_HASH).toByteArray().toHex()/*.toUpper()*/;
			
		case COLUMN_SIZE:
			return QString::number(_index.data(ROLE_SIZE).value<quint32>());
			
		case COLUMN_TRANSACTIONS:
			return QString::number(_index.data(ROLE_TRANSACTIONS).value<quint32>());
			
		case COLUMN_HEIGHT:
		{
			quint64 transactionHeight = _index.data(ROLE_HEIGHT).value<quint64>();
			if (transactionHeight == CryptoNote::WALLET_LEGACY_UNCONFIRMED_TRANSACTION_HEIGHT)
			{
				return QVariant();
			}
			return QString::number(transactionHeight);
		}
		default:
			break;
		}
		
		return QVariant();
	}

	QVariant BlockChainsModel::getDecorationRole(const QModelIndex& _index) const
	{
		return QVariant();
	}
	
	QVariant BlockChainsModel::getAlignmentRole(const QModelIndex& _index) const
	{
		return headerData(_index.column(), Qt::Horizontal, Qt::TextAlignmentRole);
	}
	
	QVariant BlockChainsModel::getUserRole(const QModelIndex& _index, int _role, CryptoNote::BlockHeight _bh, const CryptoNote::BlockChain& _blockchain) const
	{
		switch(_role)
		{
		case ROLE_DATE:
			return (_blockchain.block.timestamp > 0 ? QDateTime::fromTime_t(_blockchain.block.timestamp) : QDateTime());
	
		case ROLE_HASH:
			return QByteArray(reinterpret_cast<const char*>(&_blockchain.hash), sizeof(_blockchain.hash));
			
		case ROLE_SIZE:
			return static_cast<quint32>(_blockchain.size);
			
		case ROLE_TRANSACTIONS:
			return static_cast<quint32>(_blockchain.transactions);
			
		case ROLE_HEIGHT:
			return static_cast<quint32>(_blockchain.height);
			
		case ROLE_COLUMN:
			return headerData(_index.column(), Qt::Horizontal, ROLE_COLUMN);
			
		case ROLE_ROW:
			return _index.row();
		}
		
		return QVariant();
	}

	QVariant BlockChainsModel::getUserRoleDetails(const QModelIndex& _index, int _role, CryptoNote::BlockHeight _bh, const BlockDetails* _blockdetails) const
	{
		switch (_role)
		{
		case ROLE_DATE:
			return (_blockdetails->timestamp > 0 ? QDateTime::fromTime_t(_blockdetails->timestamp) : QDateTime());

		case ROLE_HASH:
			return QByteArray(reinterpret_cast<const char*>(&_blockdetails->hash), sizeof(_blockdetails->hash));

		case ROLE_SIZE:
			return static_cast<quint32>(_blockdetails->blockSize);

		case ROLE_TRANSACTIONS:
			return static_cast<quint32>(_blockdetails->transactions.size());

		case ROLE_HEIGHT:
			return static_cast<quint32>(_blockdetails->height);

		case ROLE_COLUMN:
			return headerData(_index.column(), Qt::Horizontal, ROLE_COLUMN);

		case ROLE_ROW:
			return _index.row();
		}

		return QVariant();
	}

	void BlockChainsModel::reloadBlockChains()
	{
		reset();
		
		quint32 row_count = 0;
		quint64 totalheight = NodeAdapter::instance().getLastLocalBlockHeight();
		for (CryptoNote::BlockHeight height = 0; height < totalheight; ++height)
		{
			appendBlockChain(height, row_count);
		}
		
		if (row_count > 0)
		{
			beginInsertRows(QModelIndex(), 0, row_count - 1);
			endInsertRows();
		}
		m_start_height = 0;
	}

	bool BlockChainsModel::showBlockChains(uint32_t _startheight)
	{
		uint32_t row_count = 0;
		uint32_t totalheight = uint32_t(NodeAdapter::instance().getLastLocalBlockHeight());
		if (_startheight > totalheight)
		{
			return false;
		}

		reset();

		m_start_height = (totalheight >= m_row) ? (qMin((totalheight - m_row + 1), _startheight)) : 0;

		m_b_lasted = (totalheight <= (m_start_height + m_row - 1));

		totalheight = qMin(totalheight, (m_start_height + m_row - 1));

		for (CryptoNote::BlockHeight height = m_start_height; height <= totalheight; ++height)
		{
			appendBlockChain(height, row_count);
		}

		if (row_count > 0)
		{
			beginInsertRows(QModelIndex(), 0, row_count - 1);
			endInsertRows();
			return true;
		}

		return false;
	}

	bool BlockChainsModel::showBlockChainsEnd(uint32_t _endheight)
	{
		uint32_t row_count = 0;
		uint32_t totalheight = uint32_t(NodeAdapter::instance().getLastLocalBlockHeight());
		if (_endheight > totalheight)
		{
			return false;
		}

		reset();

		m_b_lasted = (totalheight <= _endheight);
		totalheight = qMin(totalheight, qMax(_endheight, (m_row - 1)));
		m_start_height = (totalheight >= m_row) ? (totalheight - m_row + 1) : 0;

		for (CryptoNote::BlockHeight height = m_start_height; height <= totalheight; ++height)
		{
			appendBlockChain(height, row_count);
		}

		if (row_count > 0)
		{
			beginInsertRows(QModelIndex(), 0, row_count - 1);
			endInsertRows();
			return true;
		}

		return false;
	}

	void BlockChainsModel::showLastestBlockChains()
	{
		m_b_lasted = true;
		uint32_t totalheight = uint32_t(NodeAdapter::instance().getLastLocalBlockHeight());
		if (totalheight >= m_row)
			totalheight -= (m_row - 1);
		else
			totalheight = 0;
		showBlockChains(totalheight);
	}

	void BlockChainsModel::showLastBlockChains()
	{
		uint32_t totalheight = uint32_t(NodeAdapter::instance().getLastLocalBlockHeight());
		if ((m_start_height + m_row) >= totalheight)
		{
			//m_b_lasted = true;
			return;
		}

		showBlockChains(m_start_height + m_row);
	}

	void BlockChainsModel::showNextBlockChains()
	{
		if (m_start_height < 1)
			return;

		showBlockChains((m_start_height < m_row) ? 0 : (m_start_height - m_row));
	}

	void BlockChainsModel::showFirstBlockChains()
	{
		if (m_start_height < 1)
			return;

		showBlockChains(0);
	}

	void BlockChainsModel::appendBlockChain(CryptoNote::BlockHeight _height, quint32& _insertedRowCount)
	{
		m_blockRow[_height] = qMakePair(_height, _height);
		m_blockheights.append(BlockHeight(_height, _height));

		std::vector<std::vector<BlockDetails>> mblockdetails;
		std::vector<uint32_t> mheight;
		mheight.push_back(_height);
		if ((NodeAdapter::instance().doGetBlocks(mheight, mblockdetails)) && (!(mblockdetails.empty())))
		{
			m_blockdetails.push_back(mblockdetails.at(0).at(0));
		}

		++_insertedRowCount;
	}
	
	void BlockChainsModel::appendBlockChain(CryptoNote::BlockHeight _height)
	{
		if (m_blockRow.contains(_height))
		{
			return;
		}
	
		quint32 oldRowCount = rowCount();
		quint32 insertedRowCount = 0;
		for (quint64 transactionId = m_blockRow.size(); transactionId <= _height; ++transactionId)
		{
			appendBlockChain(transactionId, insertedRowCount);
		}
	
		if (insertedRowCount > 0)
		{
			beginInsertRows(QModelIndex(), oldRowCount, oldRowCount + insertedRowCount - 1);
			endInsertRows();
		}
	}

	void BlockChainsModel::updateBlockChains()
	{
		if (!m_b_lasted)
			return;
		uint32_t totalheight = uint32_t(NodeAdapter::instance().getLastLocalBlockHeight());
		if (m_start_height < (totalheight - m_row + 1))
		{
			showLastestBlockChains();
		}
	}

	void BlockChainsModel::updateBlockChain(CryptoNote::BlockHeight _height)
	{
		quint32 firstRow = m_blockRow.value(_height).first;
		quint32 lastRow = firstRow + 1;
		Q_EMIT dataChanged(index(firstRow, COLUMN_TIMESTAMP), index(lastRow, COLUMN_TIMESTAMP));
	}
	
	void BlockChainsModel::lastKnownHeightUpdated(quint64 _height)
	{
		if(rowCount() > 0)
		{
			Q_EMIT dataChanged(index(0, COLUMN_HEIGHT), index(rowCount() - 1, COLUMN_HEIGHT));
		}
	}

	bool BlockChainsModel::changeBlocksPerPage(uint32_t _blockspage)
	{
		if (this->m_row == _blockspage)
		{
			return true;
		}

		this->m_row = _blockspage;
		return showBlockChains(this->m_start_height);
	}

	bool BlockChainsModel::turnPage(uint32_t _page)
	{
		return showBlockChains(this->m_row * (_page - 1));
	}

	void BlockChainsModel::reset()
	{
		beginResetModel();
		m_blockheights.clear();
		m_blockRow.clear();
		m_blockdetails.clear();
		endResetModel();
	}
	
}
