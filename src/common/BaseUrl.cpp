/*
 *  Copyright (C) 2022 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "BaseUrl.h"

#include "..\utils\Utils.h"

#include <algorithm>
#include <tuple>

using namespace ADAPTIVE_TREE;

namespace
{
constexpr int DEFAULT_WEIGHT = 1;
constexpr int DEFAULT_PRIORITY = 1;

} // unnamed namespace

BaseUrl::BaseUrl(std::string_view url) : BaseUrl{url, "", DEFAULT_PRIORITY, DEFAULT_WEIGHT}
{
}

BaseUrl::BaseUrl(std::string_view url, std::string_view serviceLocation, int priority, int weight)
  : m_url{url}, m_serviceLocation{serviceLocation}, m_priority{priority}, m_weight{weight}
{
}

bool BaseUrl::operator==(const BaseUrl& other) const
{
  if (m_priority == other.m_priority && m_weight == other.m_weight && m_url == other.m_url &&
      m_serviceLocation == other.m_serviceLocation)
    return true;

  return false;
}

// Operator overload to sort e.g. std::sort(vec.begin(), vec.end());
// sort by "proprity" then "service location"
bool ADAPTIVE_TREE::BaseUrl::operator<(const BaseUrl& other) const
{
  return std::tie(m_priority, m_serviceLocation) <
         std::tie(other.m_priority, other.m_serviceLocation);
}

BaseUrl ADAPTIVE_TREE::BaseUrl::SelectUrl(const std::vector<BaseUrl>& list)
{
  if (list.empty())
    return BaseUrl("");

  //! @TODO: apply exclusions based of service location and priorities

  // Sort by priority and service location to make the sort order deterministic
  std::sort(list.begin(), list.end());

  // Get candidates of the lowest priority from the head of the sorted list
  int lowestPriority = list.front().GetPriority();

  std::vector<BaseUrl> candidates;

  for (const BaseUrl& bUrl : list)
  {
    if (lowestPriority != bUrl.GetPriority())
    {
      if (candidates.size() == 1)
      {
        // Only a single candidate of lowest priority, no choice
        return list.front();
      }
      break;
    }
    candidates.push_back(bUrl);
  }

  // Weighted random selection from multiple candidates of the same priority
  return SelectUrlWeighted(candidates);
}

BaseUrl ADAPTIVE_TREE::BaseUrl::SelectUrlWeighted(const std::vector<BaseUrl>& list)
{
  if (list.empty())
    return BaseUrl("");

  int totalWeight{0};

  for (const BaseUrl& bUrl : list)
  {
    totalWeight += bUrl.GetWeight();
  }

  const int randomChoice = UTILS::GenerateRandomNumber(0, totalWeight);
  totalWeight = 0;

  for (const BaseUrl& bUrl : list)
  {
    totalWeight += bUrl.GetWeight();

    if (randomChoice < totalWeight)
    {
      return bUrl;
    }
  }

  return list.back();
}
