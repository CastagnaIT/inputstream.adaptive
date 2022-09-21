/*
 *  Copyright (C) 2022 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#ifdef INPUTSTREAM_TEST_BUILD
#include "../test/KodiStubs.h"
#else
#include <kodi/AddonBase.h>
#endif

#include <string>
#include <string_view>
#include <vector>

namespace ADAPTIVE_TREE
{

class ATTR_DLL_LOCAL BaseUrl
{
public:
  BaseUrl(std::string_view url);
  BaseUrl(std::string_view url, std::string_view serviceLocation, int priority, int weight);
  ~BaseUrl() {}

  const std::string GetUrl() const { return m_url; }
  const int GetWeight() const { return m_weight; }
  const int GetPriority() const { return m_priority; }

  bool operator==(const BaseUrl& other) const;

  bool operator<(const BaseUrl& other) const;

  /*! 
   * \brief Select an url by taking in account priority and weight.
   *        (ETSI TS 103 285 - DVB)
   * \param list The list of BaseUrl
   * \return The selected BaseUrl
   */
  static BaseUrl SelectUrl(const std::vector<BaseUrl>& list);

private:
  /*! 
   * \brief Select an url randomly but proportional to its weight.
   *        (ETSI TS 103 285 - DVB)
   * \param list The list of BaseUrl
   * \return The selected BaseUrl
   */
  static BaseUrl SelectUrlWeighted(const std::vector<BaseUrl>& list);

  std::string m_url;
  std::string m_serviceLocation;
  int m_priority;
  int m_weight;
};

} // namespace ADAPTIVE_TREE
