
#include <windows.h>

#include "MFUtility.h"

#include <vector>

#include <propvarutil.h>
#include <assert.h>
#include <dwrite.h>
#include <wincodec.h>

#include "dxhelper.h"
#include "output_thumbnail.h"

const LONGLONG SEEK_TOLERANCE = 10000000;
const LONGLONG MAX_FRAMES_TO_SKIP = 10;
const DWORD MAX_SPRITES = 4;

#define SAMPLE_COUNT 100

OutputThumbnail::OutputThumbnail()
  : m_hwnd(nullptr)
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
  ComPtr<ID2D1HwndRenderTarget> g_pRT = nullptr;

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
  ComPtr<ID2D1Bitmap> pBitmap = nullptr;
  DWORD dwFlags = 0;

  // Output bitmaps
  ComPtr<IWICImagingFactory2> pWICFactory = nullptr;
  ComPtr<IWICBitmapEncoder> pEncoder = nullptr;
  ComPtr<IWICBitmapFrameEncode> pFrameEncode = nullptr;
  ComPtr<IWICStream> pStream = nullptr;
  ComPtr<IWICImageEncoder> imageEncoder = nullptr;

  // Init sprites
  std::vector<Sprite> sprites;
  sprites.resize(MAX_SPRITES);

  // Init videoFormat
  FormatInfo videoFormat{};

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


  // Create the Direct2D resources.
  m_hwnd = ::CreateWindow(L"STATIC"
    , L"dummy"
    , WS_DISABLED
    , 0, 0, 100, 100
    , nullptr
    , nullptr
    , nullptr
    , nullptr);
  ::SetWindowText(m_hwnd, L"Dummy Window!");
  CHECK_HR(this->createDrawindResources(m_hwnd, g_pRT), "Failed to create ID2D1HwndRenderTarget.");

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

          hr = g_pRT->CreateBitmap(
            D2D1::SizeU(videoFormat.imageWidthPels, videoFormat.imageHeightPels),
            pBitmapData,
            pitch,
            D2D1::BitmapProperties(
              // Format = RGB32
              D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)
            ),
            &pBitmap
          );

          if (FAILED(hr))
          {
            if (pBitmapData)
            {
              pBuffer->Unlock();
            }
          }

          sprites[i].SetBitmap(pBitmap.Get(), videoFormat);
        }
        else
        {
          hr = MF_E_END_OF_STREAM;
        }
      }
    }

    // Output Bitmaps
    hr = CoCreateInstance(
      CLSID_WICImagingFactory,
      NULL,
      CLSCTX_INPROC_SERVER,
      IID_IWICImagingFactory,
      reinterpret_cast<void**>(pWICFactory.GetAddressOf())
    );

    if (SUCCEEDED(hr))
    {
      hr = pWICFactory->CreateStream(&pStream);
    }

    WICPixelFormatGUID format = GUID_WICPixelFormatDontCare;
    if (SUCCEEDED(hr))
    {
      static const WCHAR filename[] = L"output.png";
      hr = pStream->InitializeFromFilename(filename, GENERIC_WRITE);
    }

    if (SUCCEEDED(hr))
    {
      hr = pWICFactory->CreateEncoder(GUID_ContainerFormatPng, NULL, &pEncoder);
    }

    if (SUCCEEDED(hr))
    {
      hr = pEncoder->Initialize(pStream.Get(), WICBitmapEncoderNoCache);
    }
    if (SUCCEEDED(hr))
    {
      hr = pEncoder->CreateNewFrame(&pFrameEncode, nullptr);
    }
    if (SUCCEEDED(hr))
    {
      hr = pFrameEncode->Initialize(nullptr);
    }
#if 0
    ComPtr<ID2D1DeviceContext> dc = nullptr;
    ComPtr<ID2D1Device> d2dDevice = nullptr;
    hr = g_pRT->QueryInterface(dc.GetAddressOf());
    if (SUCCEEDED(hr))
    {
      dc->GetDevice(&d2dDevice);
    }
#endif
    
    // Create d3d device
    this->createD3D11Device();
    ComPtr<ID2D1Device> d2dDevice;
    m_d2dContext->GetDevice(&d2dDevice);
    if (d2dDevice)
    {
      hr = pWICFactory->CreateImageEncoder(d2dDevice.Get(), &imageEncoder);
    }
    if (SUCCEEDED(hr))
    {
      hr = imageEncoder->WriteFrame(sprites[0].getBitmap(), pFrameEncode.Get(), nullptr);
    }

    if (SUCCEEDED(hr))
    {
      hr = pFrameEncode->Commit();
    }
    if (SUCCEEDED(hr))
    {
      hr = pEncoder->Commit();
    }
    if (SUCCEEDED(hr))
    {
      hr = pStream->Commit(STGC_DEFAULT);
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

HRESULT OutputThumbnail::createDrawindResources(HWND hwnd, ComPtr<ID2D1HwndRenderTarget>& rt)
{
  HRESULT hr = S_OK;
  RECT rcClient = { 0 };

  ComPtr<ID2D1Factory> pFactory = nullptr;
  ComPtr<ID2D1HwndRenderTarget> pRenderTarget = nullptr;

  GetClientRect(hwnd, &rcClient);

  hr = D2D1CreateFactory(
    D2D1_FACTORY_TYPE_SINGLE_THREADED
    , pFactory.GetAddressOf()
  );


  if (SUCCEEDED(hr))
  {
    hr = pFactory->CreateHwndRenderTarget(
      D2D1::RenderTargetProperties(),
      D2D1::HwndRenderTargetProperties(
        hwnd,
        D2D1::SizeU(rcClient.right, rcClient.bottom)
      ),
      &rt
    );
  }

  if (SUCCEEDED(hr))
  {
    rt->AddRef();
  }

  return hr;
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

HRESULT OutputThumbnail::createD3D11Device()
{
  HRESULT hr = S_OK;
  UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

  D3D_FEATURE_LEVEL featureLevels[] =
  {
      D3D_FEATURE_LEVEL_11_1,
      D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_10_1,
      D3D_FEATURE_LEVEL_10_0,
      D3D_FEATURE_LEVEL_9_3,
      D3D_FEATURE_LEVEL_9_2,
      D3D_FEATURE_LEVEL_9_1
  };

  ThrowIfFailed(
    D3D11CreateDevice(
      nullptr,                    // specify null to use the default adapter
      D3D_DRIVER_TYPE_HARDWARE,
      0,
      creationFlags,              // optionally set debug and Direct2D compatibility flags
      featureLevels,              // list of feature levels this app can support
      ARRAYSIZE(featureLevels),   // number of possible feature levels
      D3D11_SDK_VERSION,
      &m_device,                    // returns the Direct3D device created
      &m_featureLevel,            // returns feature level of device created
      &m_context                    // returns the device immediate context
    )
  );

  ComPtr<IDXGIDevice1> dxgiDevice;
  // Obtain the underlying DXGI device of the Direct3D11 device.
  ThrowIfFailed(m_device.As(&dxgiDevice));

  ThrowIfFailed(
    D2D1CreateFactory(
      D2D1_FACTORY_TYPE_SINGLE_THREADED
      , m_d2dFactory1.GetAddressOf()
    )
  );

  // Obtain the Direct2D device for 2-D rendering.
  ThrowIfFailed(
    m_d2dFactory1->CreateDevice(dxgiDevice.Get(), &m_d2dDevice)
  );

  // Get Direct2D device's corresponding device context object.
  ThrowIfFailed(
    m_d2dDevice->CreateDeviceContext(
      D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
      &m_d2dContext
    )
  );

  return hr;
}