
// Copyright (c) 2014-2016 XDN developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Account.h"
#include "CryptoNoteSerialization.h"

#if SQ_ADD_BM2
#include "CryptoNoteBasicImpl.h"
#endif

namespace CryptoNote {
//-----------------------------------------------------------------
AccountBase::AccountBase() {
	setNull();
}
//-----------------------------------------------------------------
void AccountBase::setNull() {
	m_keys = AccountKeys();
}
//-----------------------------------------------------------------
void AccountBase::generate() {
  Crypto::generate_keys(m_keys.address.spendPublicKey, m_keys.spendSecretKey);
  Crypto::generate_keys(m_keys.address.viewPublicKey, m_keys.viewSecretKey);
  m_creation_timestamp = time(NULL);
}
//-----------------------------------------------------------------
const AccountKeys &AccountBase::getAccountKeys() const {
#if SQ_ADD_BM2
	if (m_keysBM.size() > 0)
		return m_keysBM.front();
#endif
  return m_keys;
}

void AccountBase::setAccountKeys(const AccountKeys &keys) {
	m_keys = keys;
#if SQ_ADD_BM2
	m_keysBM.clear();
	m_keysBM.push_back(keys);
#endif
}
//-----------------------------------------------------------------

void AccountBase::serialize(ISerializer &s) {
  s(m_keys, "m_keys");
  s(m_creation_timestamp, "m_creation_timestamp");
}

#if SQ_ADD_BM2
size_t AccountBase::get_BMSize()
{
	return m_keysBM.size();
}

std::vector<AccountKeys>& AccountBase::get_BMKeys()
{
	return m_keysBM;
}

bool AccountBase::checkAddrExist(const std::string& str_addr, uint8_t& m_id)
{
	for (auto addr : m_keysBM)
	{
		if (str_addr == getAccountAddressAsStr(parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX, addr.address))
		{
			m_id = addr.m_id;
			return true;
		}
	}

	return false;
}

bool AccountBase::addBMGroup(const std::string& strAddress, AccountKeys& m_key, std::string& strName, int &errcode)
{
	errcode = 0;
	if (m_keysBM.size() >= BM_GROUP_NUM)
	{
		errcode = 1;
		return false;
	}

	for (auto &addr : m_keysBM)
	{
		if (strAddress == getAccountAddressAsStr(parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX, addr.address))
		{
			errcode = 4;
			return false;
		}
	}
	
	uint8_t id = 0;
	std::string buf;
	for (auto &item : m_keysBM)
	{
		id = std::max(id, item.m_id);
		if (item.m_name.find_first_of("group_") == 0)
		{
			buf = item.m_name;
			int pn = 0;
			if ((Common::fromString(buf.substr(6, buf.size() - 6), pn)) && (pn > id))
			{
				id = pn;
			}
		}
	}

	id++;
	if (strName == "")
	{
		char buf[10];
		sprintf(buf, "group_%d", id);
		strName = buf;
	}
	m_key.m_name = strName;
	m_key.m_id = id;
	m_keysBM.push_back(m_key);

	return true;
}

bool AccountBase::deleteBMGroup(const std::string& strAddress, int& id, int &errcode)
{
	errcode = 0;
	id = -1;
	std::vector<CryptoNote::AccountKeys>::iterator addr;
	for (addr = m_keysBM.begin(); (addr != m_keysBM.end()); )
	{
		if (getAccountAddressAsStr(parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX, addr->address) == strAddress)
		{
			id = addr->m_id;
			addr = m_keysBM.erase(addr);
			return true;
		}
		else
			++addr;
	}
	return false;
}

bool AccountBase::listBMGroup(std::vector<std::string>& vecadd, int &errcode)
{
	errcode = 0;
	vecadd.clear();
	size_t len = m_keysBM.size();
	for (size_t i = 1; i < len; i++)
	{
		std::string strbuf = getAccountAddressAsStr(parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX, m_keysBM[i].address);
		vecadd.push_back(strbuf);
	}
	return true;
}

bool AccountBase::addAccountKeys(AccountKeys& m_key)
{
	m_keysBM.push_back(m_key);
	return true;
}

const AccountKeys &AccountBase::getAccountKeyById(const size_t i) const
{
	if (i < 0)
		return m_keys;
	if (m_keysBM.size() > i)
		return m_keysBM[i];
	return m_keys;
}
#endif

#if SQ_ADD_BM_WALLET
bool AccountBase::getGroupList(std::vector<CryptoNote::AddressGroup>& _grouplist)
{
	_grouplist.clear();
	size_t len = m_keysBM.size();
	for (size_t i = 0; i < len; i++)
	{
		CryptoNote::AddressGroup group;
		group.m_type = 1;
		group.m_id = i;
		group.m_address = getAccountAddressAsStr(parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX, m_keysBM[i].address);
		group.m_name = m_keysBM[i].m_name;
		_grouplist.push_back(group);
	}
	if (len == 0)
	{
		CryptoNote::AddressGroup group;
		group.m_type = 1;
		group.m_id = 0;
		group.m_address = getAccountAddressAsStr(parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX, m_keys.address);
		group.m_name = m_keys.m_name;
		_grouplist.push_back(group);
	}
	return true;
}

int AccountBase::getGroupIdById(const int& _rowId)
{
	size_t len = m_keysBM.size();
	if (_rowId >= len)
		return 0;
	return m_keysBM[_rowId].m_id;
}
#endif

#if SQ_ADD_BM_SEGMENT
bool AccountBase::getOwnAddress(std::string& ownAddr)
{
	if (m_keysBM.size() > 0)
		ownAddr = getAccountAddressAsStr(parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX, m_keysBM[0].address);
	else
		ownAddr = getAccountAddressAsStr(parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX, m_keys.address);

	return true;
}
#endif
}
