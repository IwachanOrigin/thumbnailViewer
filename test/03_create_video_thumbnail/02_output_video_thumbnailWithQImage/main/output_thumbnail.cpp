
#include <windows.h>

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
const DWORD MAX_SPRITES = 4;

#define SAMPLE_COUNT 100

OutputThumbnail::OutputThumbnail()
{
}

OutputThumbnail::~OutputThumbnail()
{
}

int OutputThumbnail::open(const std::string inputFilename)
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

  // Output bitmaps
  ComPtr<IWICImagingFactory2> pWICFactory = nullptr;
  ComPtr<IWICBitmapEncoder> pEncoder = nullptr;
  ComPtr<IWICBitmapFrameEncode> pFrameEncode = nullptr;
  ComPtr<IWICStream> pStream = nullptr;
  ComPtr<IWICImageEncoder> imageEncoder = nullptr;

  // Init videoFormat
  FormatInfo videoFormat{};
  IMFMediaType* pType = NULL;
  GUID subtype = { 0 };

  wchar_t wfilename[256]{};
  mbstowcs(wfilename, inputFilename.c_str(), sizeof(wfilename) / sizeof(wchar_t));

  CHECK_HR(MFCreateAttributes(&pVideoReaderAttributes, 1), "Failed to create attributes object for video reader.");
  CHECK_HR(pVideoReaderAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE), "Failed to set MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING.");

  CHECK_HR(MFCreateSourceReaderFromURL(wfilename, pVideoReaderAttributes.Get(), &pSourceReader), "Failed to MFCreateSourceReaderFromURL.");

  MFCreateMediaType(&pMediaType);
  CHECK_HR(pMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video), "Failed to set media mejor type.");
  CHECK_HR(pMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32), "Failed to set media sub type.");
  CHECK_HR(pSourceReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, pMediaType.Get())
    , "Failed to set current media type.");
  CHECK_HR(pSourceReader->SetStreamSelection((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, true), "Failed to set stream selection.");

  CHECK_HR(pSourceReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, &pType), "Failed to get current media type.");
  CHECK_HR(pType->GetGUID(MF_MT_SUBTYPE, &subtype), "");

  CHECK_HR(MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &width, &height), "Faile to get frame size.");
  lStride = (LONG)MFGetAttributeUINT32(pType, MF_MT_DEFAULT_STRIDE, 1);
  videoFormat.bTopDown = (lStride > 0);
  CHECK_HR(MFGetAttributeRatio(pType, MF_MT_PIXEL_ASPECT_RATIO, (UINT32*)&par.Numerator, (UINT32*)&par.Denominator)
    , "Failed to get attribute ratio.");
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
  pType->Release();
  pType = nullptr;

  // Create bitmaps
  hr = this->canSeek(&bCanSeek, pSourceReader.Get());
  if (FAILED(hr)) { return hr; }

  if (bCanSeek)
  {
    hr = this->getDuration(&hnsDuration, pSourceReader.Get());

    if (FAILED(hr)) { return hr; }

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
          IMFSample* pSampleTmp = NULL;

          hr = pSourceReader->ReadSample(
            (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM
            , 0
            , nullptr
            , &dwFlags
            , nullptr
            , &pSampleTmp
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
            IMFMediaType* pType = NULL;
            GUID subtype = { 0 };
            CHECK_HR(pSourceReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, &pType), "Failed to get current media type.");
            CHECK_HR(pType->GetGUID(MF_MT_SUBTYPE, &subtype), "");

            CHECK_HR(MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &width, &height), "Faile to get frame size.");
            lStride = (LONG)MFGetAttributeUINT32(pType, MF_MT_DEFAULT_STRIDE, 1);
            videoFormat.bTopDown = (lStride > 0);
            CHECK_HR(MFGetAttributeRatio(pType, MF_MT_PIXEL_ASPECT_RATIO, (UINT32*)&par.Numerator, (UINT32*)&par.Denominator)
              , "Failed to get attribute ratio.");
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
            pType->Release();
            pType = nullptr;
          }

          if (pSampleTmp == NULL)
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

          hr = pSample->ConvertToContiguousBuffer(&pBuffer);

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
          QGraphicsScene* scene = new QGraphicsScene();
          scene->setSceneRect(0, 0, 2, 2);
          QImage img(scene->sceneRect().size().toSize(), QImage::Format_RGB32);
          QPainter painter(&img);
          scene->render(&painter);

          bool b = img.save("test.png");
          
          if (!b)
          {
            if (pBitmapData)
            {
              pBuffer->Unlock();
            }
          }

          if (FAILED(hr))
          {
            if (pBitmapData)
            {
              pBuffer->Unlock();
            }
          }
        }
        else
        {
          hr = MF_E_END_OF_STREAM;
        }
      }
    }
  }

done:
  if (pBitmapData)
  {
    pBuffer->Unlock();
  }

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
