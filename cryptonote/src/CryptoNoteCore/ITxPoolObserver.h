
// Copyright (c) 2014-2016 XDN developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

namespace CryptoNote {
class ITxPoolObserver {
public:
  virtual ~ITxPoolObserver() {
  }

  virtual void txDeletedFromPool() = 0;
};
}