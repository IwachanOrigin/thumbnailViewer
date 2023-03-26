
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
#include <d3d11.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <d2d1helper.h>
#include <dxgi1_2.h>

#include "sprite.h"

using namespace Microsoft::WRL;

class OutputThumbnail
{
public:
  explicit OutputThumbnail();
  ~OutputThumbnail();

  int open(const std::string inputFilename);

  bool createAPI();
  void destroyAPI();

private:
  RECT correctAspectRatio(const RECT& src, const MFRatio& srcPAR);
  HRESULT createDrawindResources(HWND hwnd, ComPtr<ID2D1HwndRenderTarget>& rt);
  HRESULT canSeek(BOOL* pbCanSeek, IMFSourceReader* reader);
  HRESULT getDuration(LONGLONG* phnsDuration, IMFSourceReader* reader);
  HRESULT createD3D11Device();

  ComPtr<ID3D11Device> m_device;
  ComPtr<ID3D11DeviceContext> m_context;
  D3D_FEATURE_LEVEL m_featureLevel;
  ComPtr<ID2D1Factory1> m_d2dFactory1;
  ComPtr<ID2D1Device> m_d2dDevice;
  ComPtr<ID2D1DeviceContext> m_d2dContext;
  ComPtr<IDXGISwapChain1> m_swapChain;

  HWND m_hwnd;
};

#endif // OUTPUT_THUMBNAIL_H_
