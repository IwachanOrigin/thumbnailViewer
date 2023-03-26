
#include <vector>

#include "output_thumbnail.h"
#include "MFUtility.h"
#include "sprite.h"

#include <wrl/client.h>

using namespace Microsoft::WRL;

const LONGLONG SEEK_TOLERRANCE = 10000000;
const LONGLONG MAX_FRAMES_TO_SKIP = 10;
const DWORD MAX_SPRITES = 4;

#define SAMPLE_COUNT 100

OutputThumnail::OutputThumnail()
{
}

OutputThumnail::~OutputThumnail()
{
}

int OutputThumnail::open(const std::string inputFilename)
{
  HRESULT hr = S_OK;
  ComPtr<IMFSourceResolver> pSourceResolver = nullptr;
  ComPtr<IUnknown> uSource = nullptr;
  ComPtr<IMFMediaSource> mediaFileSource = nullptr;
  ComPtr<IMFAttributes> pVideoReaderAttributes = nullptr;
  ComPtr<IMFSourceReader> pSourceReader = nullptr;
  ComPtr<IMFMediaType> pFileVideoMediaType = nullptr;
  ComPtr<IMFMediaType> pMediaType = nullptr;
  MF_OBJECT_TYPE ObjectType = MF_OBJECT_INVALID;
  uint32_t width = 0, height = 0;
  LONG lStride = 0;
  MFRatio par{};

  // Start processing frames.
  ComPtr<IMFSample> pVideoSample = nullptr;
  ComPtr<IMFSample> pCopyVideoSample = nullptr;
  ComPtr<IMFSample> pH264DecodeOutSample = nullptr;
  DWORD streamIndex = 0, flags = 0;
  LONGLONG llVideoTimeStamp = 0, llSampleDuration = 0;
  int sampleCount = 0;
  DWORD sampleFlags = 0;

  // Init sprites
  std::vector<Sprite> sprites;

  // Init videoFormat
  FormatInfo videoFormat{};

  // Set up the reader for the file.
  CHECK_HR(MFCreateSourceResolver(&pSourceResolver), "MFCreateSourceResolver failed.");

  wchar_t wfilename[256]{};
  mbstowcs(wfilename, inputFilename.c_str(), sizeof(wfilename) / sizeof(wchar_t));

  CHECK_HR(pSourceResolver->CreateObjectFromURL(
    wfilename         // URL of the source.
    , MF_RESOLUTION_MEDIASOURCE   // Create a source object.
    , nullptr                     // Optional property store.
    , &ObjectType                 // Receives the created object type.
    , &uSource					          // Receives a pointer to the media source.
  ),
    "Failed to create media source resolver for file.");

  CHECK_HR(uSource->QueryInterface(IID_PPV_ARGS(&mediaFileSource)), "Failed to create media file source.");
  CHECK_HR(MFCreateAttributes(&pVideoReaderAttributes, 2), "Failed to create attributes object for video reader.");

  CHECK_HR(pVideoReaderAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID)
           , "Failed to set dev source attribute type for reader config.");

  CHECK_HR(pVideoReaderAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, 1)
           , "Failed to set enable video processing attribute type for reader config.");

  CHECK_HR(MFCreateSourceReaderFromMediaSource(mediaFileSource.Get(), pVideoReaderAttributes.Get(), &pSourceReader)
           , "Error creating media source reader.");

  CHECK_HR(pSourceReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, &pFileVideoMediaType)
           , "Error retrieving current media type from first video stream.");

  MFCreateMediaType(&pMediaType);
  CHECK_HR(pMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video), "Failed to set media mejor type.");
  CHECK_HR(pMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32), "Failed to set media sub type.");
  CHECK_HR(pSourceReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, pMediaType.Get())
           , "Failed to set current media type.");
  CHECK_HR(pSourceReader->SetStreamSelection((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, true), "Failed to set stream selection.");

  CHECK_HR(MFGetAttributeSize(pMediaType.Get(), MF_MT_FRAME_SIZE, &width, &height), "Faile to get frame size.");
  lStride = (LONG)MFGetAttributeUINT32(pMediaType.Get(), MF_MT_DEFAULT_STRIDE, 1);
  videoFormat.bTopDown = (lStride > 0);
  CHECK_HR(MFGetAttributeRatio(pMediaType.Get(), MF_MT_PIXEL_ASPECT_RATIO, (UINT32*)&par.Numerator, (UINT32*)&par.Denominator)
           , "Failed to get attribute ratio.");
  if (par.Denominator != par.Numerator)
  {
    RECT rcSrc = {0, 0, (LONG)width, (LONG)height};
    videoFormat.rcPicture = correctAspectRatio(rcSrc, par);
  }
  else
  {
    // Either the PAR is not set (assume 1:1), or the PAR is set to 1:1.
    SetRect(&videoFormat.rcPicture, 0, 0, width, height);
  }
  videoFormat.imageWidthPels = width;
  videoFormat.imageHeightPels = height;

  // Create the Direct2D resources.
  

  while (sampleCount <= SAMPLE_COUNT)
  {
    CHECK_HR(pSourceReader->ReadSample(
      MF_SOURCE_READER_FIRST_VIDEO_STREAM
      , 0
      , &streamIndex
      , &flags
      , &llVideoTimeStamp
      , &pVideoSample
    ), "Error reading video sample.");

    if (flags & MF_SOURCE_READERF_STREAMTICK)
    {
      printf("\tStream tick.\n");
    }
    if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
    {
      printf("\tEnd of stream.\n");
      break;
    }
    if (flags & MF_SOURCE_READERF_NEWSTREAM)
    {
      printf("\tNew stream.\n");
      break;
    }
    if (flags & MF_SOURCE_READERF_NATIVEMEDIATYPECHANGED)
    {
      printf("\tNative type changed.\n");
      break;
    }
    if (flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)
    {
      printf("\tCurrent type changed.\n");
      break;
    }

    if (pVideoSample)
    {
      printf("Processing sample %i.\n", sampleCount);

      CHECK_HR(pVideoSample->SetSampleTime(llVideoTimeStamp), "Error setting the video sample time.");
      CHECK_HR(pVideoSample->GetSampleDuration(&llSampleDuration), "Error getting video sample duration.");
      CHECK_HR(pVideoSample->GetSampleFlags(&sampleFlags), "Error getting sample flags.");

      printf("Sample count %d, Sample flags %d, sample duration %I64d, sample time %I64d\n", sampleCount, sampleFlags, llSampleDuration, llVideoTimeStamp);

      // Replicate transmitting the sample across the network and reconstructing.
      CHECK_HR(CreateAndCopySingleBufferIMFSample(pVideoSample.Get(), &pCopyVideoSample), "Failed to copy single buffer IMF sample.");
    }
  }

done:

  return hr;
}

bool OutputThumnail::createAPI()
{
  CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
  HRESULT hr = MFStartup(MF_VERSION);
  return(hr == S_OK);
}

void OutputThumnail::destroyAPI()
{
  MFShutdown();
}

//-----------------------------------------------------------------------------
// CorrectAspectRatio
//
// Converts a rectangle from the source's pixel aspect ratio (PAR) to 1:1 PAR.
// Returns the corrected rectangle.
//
// For example, a 720 x 486 rect with a PAR of 9:10, when converted to 1x1 PAR,  
// is stretched to 720 x 540. 
//-----------------------------------------------------------------------------
RECT OutputThumnail::correctAspectRatio(const RECT& src, const MFRatio& srcPAR)
{
  // Start with a rectangle the same size as src, but offset to the origin (0,0).
  RECT rc = {0, 0, src.right - src.left, src.bottom - src.top};

  if ((srcPAR.Numerator != 1) || (srcPAR.Denominator != 1))
  {
    // Correct for the source's PAR.

    if (srcPAR.Numerator > srcPAR.Denominator)
    {
      // The source has "wide" pixels, so stretch the width.
      rc.right = MulDiv(rc.right, srcPAR.Numerator, srcPAR.Denominator);
    }
    else if (srcPAR.Numerator < srcPAR.Denominator)
    {
      // The source has "tall" pixels, so stretch the height.
      rc.bottom = MulDiv(rc.bottom, srcPAR.Denominator, srcPAR.Numerator);
    }
    // else: PAR is 1:1, which is a no-op.
  }
  return rc;
}
