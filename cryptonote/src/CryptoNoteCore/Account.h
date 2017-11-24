
// Copyright (c) 2014-2016 XDN developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "CryptoNoteCore/CryptoNoteBasic.h"
#include "crypto/crypto.h"

namespace CryptoNote {

  class ISerializer;

  /************************************************************************/
  /*                                                                      */
  /************************************************************************/
  class AccountBase {
  public:
    AccountBase();
    void generate();

    const AccountKeys& getAccountKeys() const;
    void setAccountKeys(const AccountKeys& keys);
    uint64_t get_createtime() const { return m_creation_timestamp; }
    void set_createtime(uint64_t val) { m_creation_timestamp = val; }
    void serialize(ISerializer& s);

    template <class t_archive>
    inline void serialize(t_archive &a, const unsigned int /*ver*/) {
      a & m_keys;
      a & m_creation_timestamp;
    }

#if SQ_ADD_BM2
	std::string  m_name;
	uint8_t m_id;

	size_t get_BMSize();
	std::vector<AccountKeys>& get_BMKeys();
	bool checkAddrExist(const std::string& str_addr, uint8_t& m_id);
	bool addBMGroup(const std::string& strAddress, AccountKeys& m_key, std::string& strName, int &errcode);
	bool deleteBMGroup(const std::string& strAddress, int& id, int &errcode);
	bool listBMGroup(std::vector<std::string>& vecadd, int &errcode);
	bool addAccountKeys(AccountKeys& m_key);
	const AccountKeys& getAccountKeyById(const size_t i) const;
#endif

#if SQ_ADD_BM_WALLET
	bool getGroupList(std::vector<CryptoNote::AddressGroup>& _grouplist);
	int getGroupIdById(const int& _rowId);
#endif
#if SQ_ADD_BM_SEGMENT
	bool getOwnAddress(std::string& ownAddr);
#endif
  private:
    void setNull();
    AccountKeys m_keys;
	uint64_t m_creation_timestamp;

#if SQ_ADD_BM2
	std::vector<AccountKeys> m_keysBM;
#endif
  };
}
