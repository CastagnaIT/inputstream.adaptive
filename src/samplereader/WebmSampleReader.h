/*
 *  Copyright (C) 2022 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "AdaptiveByteStream.h"
#include "SampleReader.h"
#include "demuxers/WebmReader.h"

class ATTR_DLL_LOCAL CWebmSampleReader : public ISampleReader, public WebmReader
{
public:
  CWebmSampleReader(AP4_ByteStream* input, AP4_UI32 streamId);

  bool IsStarted() const override { return m_started; }
  bool EOS() const override { return m_eos; }
  uint64_t DTS() const override { return m_dts; }
  uint64_t PTS() const override { return m_pts; }
  bool Initialize() override;
  AP4_Result Start(bool& bStarted) override;
  AP4_Result ReadSample() override;
  void Reset(bool bEOS) override;
  bool GetInformation(kodi::addon::InputstreamInfo& info) override
  {
    return WebmReader::GetInformation(info);
  }
  bool TimeSeek(uint64_t pts, bool preceeding) override;
  void SetPTSOffset(uint64_t offset) override { m_ptsOffs = offset; }
  uint64_t GetStartPTS() const override { return m_startPts; }
  void SetStartPTS(uint64_t pts) override { m_startPts = pts; }
  int64_t GetPTSDiff() const override { return m_ptsDiff; }
  uint32_t GetTimeScale() const override { return 1000; }
  AP4_UI32 GetStreamId() const override { return m_streamId; }
  AP4_Size GetSampleDataSize() const override { return GetPacketSize(); }
  const AP4_Byte* GetSampleData() const override { return GetPacketData(); }
  uint64_t GetDuration() const override { return WebmReader::GetDuration() * 1000; }
  bool IsEncrypted() const override { return false; }

private:
  AP4_UI32 m_streamId;
  uint64_t m_pts{0};
  uint64_t m_dts{0};
  uint64_t m_ptsOffs{~0ULL};
  int64_t m_ptsDiff{0};
  uint64_t m_startPts{STREAM_NOPTS_VALUE};
  bool m_eos{false};
  bool m_started{false};
  CAdaptiveByteStream* m_adByteStream;
};
