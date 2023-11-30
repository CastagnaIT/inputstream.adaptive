/*
 *  Copyright (C) 2022 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include <algorithm>
#include <limits>
#include <string_view>
#include <vector>

#include "utils/log.h"

// forwards
class AP4_Movie;
namespace adaptive
{
class AdaptiveStream;
}
namespace kodi::addon
{
class InputstreamInfo;
}

namespace PLAYLIST
{
// Marker for the default psshset position
constexpr uint16_t PSSHSET_POS_DEFAULT = 0;
// Marker for not valid psshset position
constexpr uint16_t PSSHSET_POS_INVALID = std::numeric_limits<uint16_t>::max();
// Marker for not set/not found segment position
constexpr size_t SEGMENT_NO_POS = std::numeric_limits<size_t>::max();
// Marker for not set/not found segment number
constexpr uint64_t SEGMENT_NO_NUMBER = std::numeric_limits<uint64_t>::max();
// Marker for undefined timestamp value
constexpr uint64_t NO_PTS_VALUE = std::numeric_limits<uint64_t>::max();
// Marker for undefined value
constexpr uint64_t NO_VALUE = std::numeric_limits<uint64_t>::max();

enum class EncryptionState
{
  UNENCRYPTED,
  ENCRYPTED, // Unhandled/unsupported encrypted stream
  ENCRYPTED_SUPPORTED, // Supported encrypted stream
};

enum class EncryptionType
{
  NOT_SUPPORTED,
  CLEAR,
  AES128,
  WIDEVINE,
  UNKNOWN,
};

enum class PrepareRepStatus
{
  FAILURE,
  OK,
  DRMCHANGED,
  DRMUNCHANGED,
};

enum class ContainerType
{
  NOTYPE,
  INVALID,
  MP4,
  TS,
  ADTS,
  WEBM,
  MATROSKA,
  TEXT,
};

enum class StreamType
{
  NOTYPE,
  VIDEO,
  AUDIO,
  SUBTITLE,
  VIDEO_AUDIO,
};

/*!
 * \brief Convert StreamType enum value into a human readable string.
 * \param streamType The stream type to convert
 * \return Human readable StreamType string
 */
std::string_view StreamTypeToString(const StreamType streamType);

/*!
 * \brief Parse a range string, as RFC 7233 (e.g. for DASH).
 * \param range The range string to parse of format like "n-n"
 * \param start [OUT] The start position
 * \param end [OUT] The end position
 * \return True when parsed correctly, otherwise false
 */
bool ParseRangeRFC(std::string_view range, uint64_t& start, uint64_t& end);

/*!
 * \brief Parse range values, the second value splitted by a separator char is optional.
 * \param range The range string to parse of format, e.g. "n" or "n[separator]n"
 * \param first [OUT] The first value, if not parsed nothing will be written
 * \param second [OUT] The second optional value, if not parsed nothing will be written
 * \return True when parsed correctly, otherwise false
 */
bool ParseRangeValues(std::string_view range,
                      uint64_t& first,
                      uint64_t& second,
                      char separator = '@');

/*!
 * \brief Create a Movie atom based on stream properties and info.
 * \param adStream The adaptive stream
 * \param streamInfo The stream info
 * \return The Movie atom
 */
AP4_Movie* CreateMovieAtom(adaptive::AdaptiveStream& adStream,
                           kodi::addon::InputstreamInfo& streamInfo);

constexpr uint32_t MP4_BOX_LMSG_BRAND = 'lmsg'; // Last Media Segment signal

// \brief MP4 Atom
class CAtom
{
public:
  CAtom(const uint8_t* dataBuffer, size_t size);

  uint64_t GetSize() const { return m_boxSize; }
  uint32_t GetType() const { return m_boxType; }

protected:
  const uint8_t* m_payload{nullptr};
  uint64_t m_boxSize{0};
  uint32_t m_boxType{0};
};

// \brief MP4 STYP Atom
class CAtomStyp : public CAtom
{
public:
  CAtomStyp(const CAtom& atom);

  uint32_t GetMajorBrand() const { return m_majorBrand; }
  bool IsBrandCompatible(uint32_t brand) const;

protected:
  uint32_t m_majorBrand{0};
  uint32_t m_minorVersion{0};
  std::vector<uint32_t> m_compatibleBrands;
};

template<typename T>
class CSpinCache
{
public:
  /*!
   * \brief Get the <T> value pointer from the specified position
   * \param pos The position of <T>
   * \return <T> value pointer, otherwise nullptr if not found
   */
  const T* Get(size_t pos) const
  {
    if (pos == SEGMENT_NO_POS || m_data.empty())
      return nullptr;
    size_t realPos = m_basePos + pos;
    if (realPos >= m_data.size())
    {
      realPos -= m_data.size();
      if (realPos == m_basePos)
        return nullptr;
      else if (realPos >= m_data.size())
      {
        LOG::LogF(LOGWARNING, "Requested data from out-of-range position %zu of %zu",
                  realPos, m_data.size());
        return nullptr;
      }
    }
    return &m_data[realPos];
  }

  /*!
   * \brief Get the <T> value pointer from the specified position
   * \param pos The position of <T>
   * \return <T> value pointer, otherwise nullptr if not found
   */
  T* Get(size_t pos)
  {
    if (pos == SEGMENT_NO_POS || m_data.empty())
      return nullptr;
    size_t realPos = m_basePos + pos;
    if (realPos >= m_data.size())
    {
      realPos -= m_data.size();
      if (realPos == m_basePos)
        return nullptr;
      else if (realPos >= m_data.size())
      {
        LOG::LogF(LOGWARNING, "Requested data from out-of-range position %zu of %zu", realPos,
                  m_data.size());
        return nullptr;
      }
    }
    return &m_data[realPos];
  }

  /*!
   * \brief Get index position of <T> value pointer
   * \param elem The <T> pointer to get the position
   * \return The index position
   */
  const size_t GetPosition(const T* elem) const
  {
    if (m_data.empty())
      return 0;

    size_t realPos = elem - &m_data[0];
    if (realPos < m_basePos)
      realPos += m_data.size() - m_basePos;
    else
      realPos -= m_basePos;
    return realPos;
  };

  void Insert(const T& elem)
  {
    m_data[m_basePos] = elem;
    ++m_basePos;
    if (m_basePos == m_data.size())
      m_basePos = 0;
  }

  void Swap(CSpinCache<T>& other)
  {
    m_data.swap(other.m_data);
    std::swap(m_basePos, other.m_basePos);
  }

  void Clear()
  {
    m_data.clear();
    m_basePos = 0;
  }

  bool IsEmpty() const { return m_data.empty(); }

  size_t GetSize() const { return m_data.size(); }

  std::vector<T>& GetData() { return m_data; }

private:
  size_t m_basePos{0};
  std::vector<T> m_data;
};

// \brief Get the position of a pointer within a vector of unique_ptr
template<typename T, typename Ptr>
size_t GetPtrPosition(const std::vector<std::unique_ptr<T>>& container, const Ptr* ptr)
{
  auto itRepr = std::find_if(container.begin(), container.end(),
                             [&ptr](const std::unique_ptr<T>& r) { return r.get() == ptr; });
  return itRepr - container.begin();
}

} // namespace adaptive
