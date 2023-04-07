
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

class OutputThumbnail
{

struct FormatInfo
{
  UINT32          imageWidthPels;
  UINT32          imageHeightPels;
  BOOL            bTopDown;
  RECT            rcPicture;    // Corrected for pixel aspect ratio

  FormatInfo() : imageWidthPels(0), imageHeightPels(0), bTopDown(FALSE)
  {
    SetRectEmpty(&rcPicture);
  }
};

public:
  explicit OutputThumbnail();
  ~OutputThumbnail();

  int open(const std::string inputFilename);

  bool createAPI();
  void destroyAPI();

private:
  RECT correctAspectRatio(const RECT& src, const MFRatio& srcPAR);
  HRESULT canSeek(BOOL* pbCanSeek, IMFSourceReader* reader);
  HRESULT getDuration(LONGLONG* phnsDuration, IMFSourceReader* reader);
};

#endif // OUTPUT_THUMBNAIL_H_
