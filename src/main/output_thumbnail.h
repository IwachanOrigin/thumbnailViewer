
#ifndef OUTPUT_THUMBNAIL_H_
#define OUTPUT_THUMBNAIL_H_

#include <windows.h>
#include <string>

#include <mfapi.h>
#include <mfplay.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <wmcodecdsp.h>

#include <wrl/client.h>

using namespace Microsoft::WRL;

class QImage;

class OutputThumbnail
{

struct FormatInfo
{
  UINT32 imageWidthPels;
  UINT32 imageHeightPels;
  BOOL   bTopDown;
  RECT   rcPicture;    // Corrected for pixel aspect ratio

  FormatInfo() : imageWidthPels(0), imageHeightPels(0), bTopDown(FALSE)
  {
    SetRectEmpty(&rcPicture);
  }
};

public:
  explicit OutputThumbnail();
  ~OutputThumbnail();

  int getThumbnail(const std::string inputFilename, QImage& rtnImage);
  int getFileInfo(const std::string inputFilename, UINT32& width, UINT32& height, UINT32& bitrate, LONGLONG& duration, UINT32& channelCount, UINT32& samplesPerSec, UINT32& bitsPerSample);

  bool createAPI();
  void destroyAPI();

private:
  RECT correctAspectRatio(const RECT& src, const MFRatio& srcPAR);
  HRESULT canSeek(BOOL* pbCanSeek, IMFSourceReader* reader);
  HRESULT getDuration(LONGLONG* phnsDuration, IMFSourceReader* reader);
};

#endif // OUTPUT_THUMBNAIL_H_
