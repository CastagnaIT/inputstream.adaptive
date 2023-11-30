/*
 *  Copyright (C) 2022 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include <bento4/Ap4.h>

#ifdef INPUTSTREAM_TEST_BUILD
#include "test/KodiStubs.h"
#else
#include <kodi/AddonBase.h>
#endif

// forwards
namespace adaptive
{
class AdaptiveStream;
}

class ATTR_DLL_LOCAL CAdaptiveByteStream : public AP4_ByteStream
{
public:
  CAdaptiveByteStream(adaptive::AdaptiveStream* adStream) : m_adStream(adStream){}
  virtual ~CAdaptiveByteStream(){}

  // AP4_ByteStream methods
  AP4_Result ReadPartial(void* buffer, AP4_Size bytesToRead, AP4_Size& bytesRead) override;
  AP4_Result WritePartial(const void* buffer,
                          AP4_Size bytesToWrite,
                          AP4_Size& bytesWritten) override;
  AP4_Result Seek(AP4_Position position) override;
  AP4_Result Tell(AP4_Position& position) override;
  AP4_Result GetSize(AP4_LargeSize& size) override;
  AP4_Result GetSegmentSize(size_t& size);

  // AP4_Referenceable methods
  void AddReference() override{};
  void Release() override{};
  bool waitingForSegment() const;
  void FixateInitialization(bool on);
  void SetSegmentFileOffset(uint64_t offset);

  /*!
   * \brief Check if current segment has been signalled as the last one.
   * \return True if it is the last segment, otherwise false.
   */
  bool IsEosSegment();

protected:
  adaptive::AdaptiveStream* m_adStream;
};
