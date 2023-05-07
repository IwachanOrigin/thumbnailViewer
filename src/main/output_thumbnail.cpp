
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>

#include <propvarutil.h>
#include <assert.h>
#include <dwrite.h>
#include <wincodec.h>

#include <QImage>
#include <QPainter>
#include <QGraphicsScene>

#include "MFUtility.h"
#include "output_thumbnail.h"

const LONGLONG SEEK_TOLERANCE = 10000000;
const LONGLONG MAX_FRAMES_TO_SKIP = 10;
const DWORD MAX_SPRITES = 1;

#define SAMPLE_COUNT 100

OutputThumbnail::OutputThumbnail()
{
}

OutputThumbnail::~OutputThumbnail()
{
}

int OutputThumbnail::getThumbnail(const std::string inputFilename, QImage& rtnImage)
{
  HRESULT hr = S_OK;
  ComPtr<IMFMediaSource> mediaFileSource = nullptr;
  ComPtr<IMFAttributes> pVideoReaderAttributes = nullptr;
  ComPtr<IMFSourceReader> pSourceReader = nullptr;
  ComPtr<IMFMediaType> pMediaType = nullptr;
  MF_OBJECT_TYPE ObjectType = MF_OBJECT_INVALID;
  uint32_t width = 0, height = 0;
  LONG lStride = 0;
  MFRatio par{};

  // create bitmaps
  LONGLONG hnsDuration = 0;
  LONGLONG hnsRangeStart = 0;
  LONGLONG hnsRangeEnd = 0;
  LONGLONG hnsIncrement = 0;
  BYTE* pBitmapData = nullptr;   // Bitmap data
  DWORD       cbBitmapData = 0;  // Size of data, in bytes
  LONGLONG    hnsTimeStamp = 0;
  BOOL        bCanSeek = FALSE;  // Can the source seek?  
  DWORD       cSkipped = 0;      // Number of skipped frames

  ComPtr<IMFMediaBuffer> pBuffer = nullptr;
  ComPtr<IMFSample> pSample = nullptr;
  DWORD dwFlags = 0;

  // Init videoFormat
  FormatInfo videoFormat{};
  ComPtr<IMFMediaType> pType = nullptr;
  GUID subtype = { 0 };

  wchar_t wfilename[256]{};
  size_t convertedCharas = 0;
  mbstowcs_s(&convertedCharas, wfilename, inputFilename.c_str(), sizeof(wfilename) / sizeof(wchar_t));

  hr = MFCreateAttributes(pVideoReaderAttributes.GetAddressOf(), 1);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to create attributes object for video reader.", L"Error", MB_OK);
    return -1;
  }

  hr = pVideoReaderAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to set MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING.", L"Error", MB_OK);
    return -1;
  }

  hr = MFCreateSourceReaderFromURL(wfilename, pVideoReaderAttributes.Get(), pSourceReader.GetAddressOf());
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to MFCreateSourceReaderFromURL.", L"Error", MB_OK);
    return -1;
  }

  hr = MFCreateMediaType(pMediaType.GetAddressOf());
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to create media type.", L"Error", MB_OK);
    return -1;
  }

  hr = pMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to set media mejor type.", L"Error", MB_OK);
    return -1;
  }

  hr = pMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to set media sub type.", L"Error", MB_OK);
    return -1;
  }

  hr = pSourceReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, pMediaType.Get());
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to set current media type.", L"Error", MB_OK);
    return -1;
  }

  hr = pSourceReader->SetStreamSelection((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, true);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to set stream selection.", L"Error", MB_OK);
    return -1;
  }

  hr = pSourceReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, pType.GetAddressOf());
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to get current media type.", L"Error", MB_OK);
    return -1;
  }
  hr = pType->GetGUID(MF_MT_SUBTYPE, &subtype);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to get current subtype.", L"Error", MB_OK);
    return -1;
  }

  hr = MFGetAttributeSize(pType.Get(), MF_MT_FRAME_SIZE, &width, &height);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to get frame size.", L"Error", MB_OK);
    return -1;
  }

  lStride = (LONG)MFGetAttributeUINT32(pType.Get(), MF_MT_DEFAULT_STRIDE, 1);
  videoFormat.bTopDown = (lStride > 0);
  hr = MFGetAttributeRatio(pType.Get(), MF_MT_PIXEL_ASPECT_RATIO, (UINT32*)&par.Numerator, (UINT32*)&par.Denominator);
  if (FAILED(hr))
  {
    MessageBoxW(nullptr, L"Failed to get attribute ratio.", L"Error", MB_OK);
    return -1;
  }

  if (par.Denominator != par.Numerator)
  {
    RECT rcSrc = { 0, 0, (LONG)width, (LONG)height };
    videoFormat.rcPicture = correctAspectRatio(rcSrc, par);
  }
  else
  {
    // Either the PAR is not set (assume 1:1), or the PAR is set to 1:1.
    SetRect(&videoFormat.rcPicture, 0, 0, width, height);
  }
  videoFormat.imageWidthPels = width;
  videoFormat.imageHeightPels = height;

  // Create bitmaps
  hr = this->canSeek(&bCanSeek, pSourceReader.Get());
  if (FAILED(hr))
  {
    return hr;
  }

  if (bCanSeek)
  {
    hr = this->getDuration(&hnsDuration, pSourceReader.Get());

    if (FAILED(hr))
    {
      return hr;
    }

    hnsRangeStart = 0;
    hnsRangeEnd = hnsDuration;

    // We have the file duration , so we'll take bitmaps from
    // several positions in the file. Occasionally, the first frame 
    // in a video is black, so we don't start at time 0.

    hnsIncrement = (hnsRangeEnd - hnsRangeStart) / (MAX_SPRITES + 1);

    // Generate the bitmaps and invalidate the button controls so
    // they will be redrawn.
    for (DWORD i = 0; i < MAX_SPRITES; i++)
    {
      LONGLONG hPos = hnsIncrement * (i + 1);
      hr = this->canSeek(&bCanSeek, pSourceReader.Get());
      if (FAILED(hr))
      {
        continue;
      }
      if (bCanSeek && (hPos > 0))
      {
        PROPVARIANT var;
        PropVariantInit(&var);

        var.vt = VT_I8;
        var.hVal.QuadPart = hPos;

        hr = pSourceReader->SetCurrentPosition(GUID_NULL, var);
        if (FAILED(hr))
        {
          if (pBitmapData)
          {
            pBuffer->Unlock();
          }
        }

        // Pulls video frames from the source reader.

        // NOTE: Seeking might be inaccurate, depending on the container
        //       format and how the file was indexed. Therefore, the first
        //       frame that we get might be earlier than the desired time.
        //       If so, we skip up to MAX_FRAMES_TO_SKIP frames.

        while (1)
        {
          ComPtr<IMFSample> pSampleTmp = nullptr;

          hr = pSourceReader->ReadSample(
            (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM
            , 0
            , nullptr
            , &dwFlags
            , nullptr
            , pSampleTmp.GetAddressOf()
          );
          if (FAILED(hr))
          {
            if (pBitmapData)
            {
              pBuffer->Unlock();
            }
          }

          if (dwFlags & MF_SOURCE_READERF_ENDOFSTREAM)
          {
            break;
          }

          if (dwFlags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)
          {
            // Type change. Get the new format.
            ComPtr<IMFMediaType> pType = nullptr;
            GUID subtype = { 0 };
            hr = pSourceReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, &pType);
            if (FAILED(hr))
            {
              MessageBoxW(nullptr, L"Failed to get current media type.", L"Error", MB_OK);
              return -1;
            }
            hr = pType->GetGUID(MF_MT_SUBTYPE, &subtype);
            if (FAILED(hr))
            {
              MessageBoxW(nullptr, L"Failed to get current subtype.", L"Error", MB_OK);
              return -1;
            }

            hr = MFGetAttributeSize(pType.Get(), MF_MT_FRAME_SIZE, &width, &height);
            if (FAILED(hr))
            {
              MessageBoxW(nullptr, L"Faile to get frame size.", L"Error", MB_OK);
              return -1;
            }
            lStride = (LONG)MFGetAttributeUINT32(pType.Get(), MF_MT_DEFAULT_STRIDE, 1);
            videoFormat.bTopDown = (lStride > 0);
            hr = MFGetAttributeRatio(pType.Get(), MF_MT_PIXEL_ASPECT_RATIO, (UINT32*)&par.Numerator, (UINT32*)&par.Denominator);
            if (FAILED(hr))
            {
              MessageBoxW(nullptr, L"Failed to get attribute ratio.", L"Error", MB_OK);
              return -1;
            }

            if (par.Denominator != par.Numerator)
            {
              RECT rcSrc = { 0, 0, (LONG)width, (LONG)height };
              videoFormat.rcPicture = correctAspectRatio(rcSrc, par);
            }
            else
            {
              // Either the PAR is not set (assume 1:1), or the PAR is set to 1:1.
              SetRect(&videoFormat.rcPicture, 0, 0, width, height);
            }
            videoFormat.imageWidthPels = width;
            videoFormat.imageHeightPels = height;
          }

          if (pSampleTmp == nullptr)
          {
            continue;
          }

          // We got a sample. Hold onto it.
          if (pSample)
          {
            pSample->Release();
          }

          pSample = pSampleTmp;
          pSample->AddRef();

          if (SUCCEEDED(pSample->GetSampleTime(&hnsTimeStamp)))
          {
            // Keep going until we get a frame that is within tolerance of the
            // desired seek position, or until we skip MAX_FRAMES_TO_SKIP frames.

            // During this process, we might reach the end of the file, so we
            // always cache the last sample that we got (pSample).

            if ((cSkipped < MAX_FRAMES_TO_SKIP) &&
              (hnsTimeStamp + SEEK_TOLERANCE < hPos))
            {
              pSampleTmp->Release();

              ++cSkipped;
              continue;
            }
          }

          hPos = hnsTimeStamp;
          break;
        }

        if (pSample)
        {
          UINT32 pitch = 4 * videoFormat.imageWidthPels;

          // Get the bitmap data from the sample, and use it to create a
          // Direct2D bitmap object. Then use the Direct2D bitmap to 
          // initialize the sprite.

          hr = pSample->ConvertToContiguousBuffer(pBuffer.GetAddressOf());

          if (FAILED(hr))
          {
            if (pBitmapData)
            {
              pBuffer->Unlock();
            }
          }

          hr = pBuffer->Lock(&pBitmapData, NULL, &cbBitmapData);

          if (FAILED(hr))
          {
            if (pBitmapData)
            {
              pBuffer->Unlock();
            }
          }

          assert(cbBitmapData == (pitch * videoFormat.imageHeightPels));

          // Create Bitmap
          QImage img((const unsigned char*)pBitmapData, videoFormat.imageWidthPels, videoFormat.imageHeightPels, QImage::Format_RGB32);
          rtnImage = img;
          //bool b = img.save("test.png");

          if (pBitmapData)
          {
            pBuffer->Unlock();
          }
        }
        else
        {
          hr = MF_E_END_OF_STREAM;
        }
      }
    }
  }

  if (pBitmapData)
  {
    pBuffer->Unlock();
  }

  return hr;
}

int OutputThumbnail::getFileInfo(const std::string inputFilename, UINT32& width, UINT32& height, UINT32& bitrate, LONGLONG& duration, UINT32& channelCount, UINT32& samplesPerSec, UINT32& bitsPerSample)
{
  HRESULT hr = S_OK;
  ComPtr<IMFSourceResolver> pSourceResolver = nullptr;
  ComPtr<IUnknown> pSource = nullptr;
  ComPtr<IMFMediaSource> pMediaSource = nullptr;
  ComPtr<IMFPresentationDescriptor> pPresentationDesc = nullptr;
  ComPtr<IMFStreamDescriptor> pStreamDesc = nullptr;
  ComPtr<IMFMediaTypeHandler> pMediaTypeHandler = nullptr;

  ComPtr<IMFSourceReader> pSourceReader = nullptr;
  ComPtr<IMFMediaType> pMediaType = nullptr;
  MFRatio par{};

  wchar_t wfilename[256]{};
  size_t convertedCharas = 0;
  mbstowcs_s(&convertedCharas, wfilename, inputFilename.c_str(), sizeof(wfilename) / sizeof(wchar_t));

  CHECK_HR(MFCreateSourceResolver(&pSourceResolver), "Failed to MFCreateSourceResolver.");
  MF_OBJECT_TYPE objectType;
  CHECK_HR(pSourceResolver->CreateObjectFromURL(wfilename, MF_RESOLUTION_MEDIASOURCE, nullptr, &objectType, &pSource), "Failed to CreateObjectFromURL.");
  CHECK_HR(pSource->QueryInterface(IID_PPV_ARGS(&pMediaSource)), "Failed to QueryInterface.");
  CHECK_HR(pMediaSource->CreatePresentationDescriptor(&pPresentationDesc), "Failed to CreatePresentationDescriptor.");
  DWORD streamCount = 0;
  CHECK_HR(pPresentationDesc->GetStreamDescriptorCount(&streamCount), "Failed to GetStreamDescriptorCount.");
  for (int i = 0; i < (int)streamCount; i++)
  {
    BOOL isSelected = FALSE;
    CHECK_HR(pPresentationDesc->GetStreamDescriptorByIndex(i, &isSelected, &pStreamDesc), "Failed to GetStreamDescriptorByIndex.");
    CHECK_HR(pStreamDesc->GetMediaTypeHandler(&pMediaTypeHandler), "Failed to GetMediaTypeHandler.");
    GUID majorType;
    CHECK_HR(pMediaTypeHandler->GetMajorType(&majorType), "Failed to GetMajorType.");
    if (majorType == MFMediaType_Video)
    {
      CHECK_HR(pMediaTypeHandler->GetCurrentMediaType(&pMediaType), "Failed to get current media type.");
      CHECK_HR(MFGetAttributeSize(pMediaType.Get(), MF_MT_FRAME_SIZE, &width, &height), "Failed to get frame size.");
      CHECK_HR(pMediaType->GetUINT32(MF_MT_AVG_BITRATE, &bitrate), "Failed to get bitrate.");
      CHECK_HR(MFGetAttributeRatio(pMediaType.Get(), MF_MT_PIXEL_ASPECT_RATIO, (UINT32*)&par.Numerator, (UINT32*)&par.Denominator), "Failed to get attribute ratio.");
      CHECK_HR(MFCreateSourceReaderFromURL(wfilename, nullptr, &pSourceReader), "Failed to MFCreateSourceReaderFromURL.");
      CHECK_HR(this->getDuration(&duration, pSourceReader.Get()), "Failed to get duration.");
      break;
    }
    else if (majorType == MFMediaType_Audio)
    {
      channelCount = 0;
      samplesPerSec = 0;
      bitsPerSample = 0;
      CHECK_HR(pMediaTypeHandler->GetCurrentMediaType(&pMediaType), "Failed to get current media type.");
      channelCount = MFGetAttributeUINT32(pMediaType.Get(), MF_MT_AUDIO_NUM_CHANNELS, 0);
      samplesPerSec = MFGetAttributeUINT32(pMediaType.Get(), MF_MT_AUDIO_SAMPLES_PER_SECOND, 0);
      bitsPerSample = MFGetAttributeUINT32(pMediaType.Get(), MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
    }
  }

done:
  return hr;
}

bool OutputThumbnail::createAPI()
{
  CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
  HRESULT hr = MFStartup(MF_VERSION);
  return(hr == S_OK);
}

void OutputThumbnail::destroyAPI()
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
RECT OutputThumbnail::correctAspectRatio(const RECT& src, const MFRatio& srcPAR)
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

HRESULT OutputThumbnail::canSeek(BOOL* pbCanSeek, IMFSourceReader* reader)
{
  HRESULT hr = S_OK;

  ULONG flags = 0;

  PROPVARIANT var;
  PropVariantInit(&var);

  if (reader == NULL)
  {
    return MF_E_NOT_INITIALIZED;
  }

  *pbCanSeek = FALSE;

  hr = reader->GetPresentationAttribute(
    (DWORD)MF_SOURCE_READER_MEDIASOURCE,
    MF_SOURCE_READER_MEDIASOURCE_CHARACTERISTICS,
    &var
  );

  if (SUCCEEDED(hr))
  {
    hr = PropVariantToUInt32(var, &flags);
  }

  if (SUCCEEDED(hr))
  {
    // If the source has slow seeking, we will treat it as
    // not supporting seeking. 

    if ((flags & MFMEDIASOURCE_CAN_SEEK) &&
      !(flags & MFMEDIASOURCE_HAS_SLOW_SEEK))
    {
      *pbCanSeek = TRUE;
    }
  }

  return hr;
}

HRESULT OutputThumbnail::getDuration(LONGLONG* phnsDuration, IMFSourceReader* reader)
{
  PROPVARIANT var;
  PropVariantInit(&var);

  HRESULT hr = S_OK;

  if (reader == NULL)
  {
    return MF_E_NOT_INITIALIZED;
  }

  hr = reader->GetPresentationAttribute(
    (DWORD)MF_SOURCE_READER_MEDIASOURCE,
    MF_PD_DURATION,
    &var
  );

  if (SUCCEEDED(hr))
  {
    assert(var.vt == VT_UI8);
    *phnsDuration = var.hVal.QuadPart;
  }

  PropVariantClear(&var);

  return hr;
}
