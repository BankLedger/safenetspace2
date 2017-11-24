
// Copyright (c) 2014-2016 XDN developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "KeysStorage.h"

#include "WalletLegacy/WalletLegacySerialization.h"
#include "Serialization/ISerializer.h"
#include "Serialization/SerializationOverloads.h"
#include "CryptoNoteCore/CryptoNoteSerialization.h"

namespace CryptoNote {

	void KeysStorage::serialize(ISerializer& serializer, const std::string& name) {
		serializer.beginObject(name);
		if (m_wallet_version == 1)
		{
			serializer(m_buf, "m_buf");
			serializer(TimeString, "TimeString");
			memcpy(&creationTimestamp, &TimeString, 8);
			serializer(BufSPKey, "BufSPKey");
			serializer(spendPublicKey, "spend_public_key");
			serializer(BufVPKey, "BufVPKey");
			serializer(viewPublicKey, "view_public_key");
			serializer(BufSVKey, "BufSVKey");
			serializer(spendSecretKey, "spend_secret_key");
			serializer(BufVSKey, "BufVSKey");
			serializer(viewSecretKey, "view_secret_key");
		}
		else
		{
			serializer(creationTimestamp, "creation_timestamp");
			serializer(spendPublicKey, "spend_public_key");
			serializer(spendSecretKey, "spend_secret_key");
			serializer(viewPublicKey, "view_public_key");
			serializer(viewSecretKey, "view_secret_key");
#if SQ_ADD_BM2
			if (m_wallet_version == WALLET_VERSION_BM)
			{
				serializer(m_id, "m_id");
				serializer(m_name, "m_name");
			}
#endif
		}
		
		serializer.endObject();
	}
}
