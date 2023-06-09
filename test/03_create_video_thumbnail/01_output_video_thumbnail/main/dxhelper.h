
#ifndef DX_HELPER_H_
#define DX_HELPER_H_

#include <cstdint>
#include <d3d11.h>
#include <wrl/client.h>
#include <shellapi.h>
#include <stdexcept>

using Microsoft::WRL::ComPtr;

inline std::string HrToString(HRESULT hr)
{
  char s_str[64] = {};
  sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
  return std::string(s_str);
}

class HrException : public std::runtime_error
{
public:
  HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
  HRESULT Error() const { return m_hr; }
private:
  const HRESULT m_hr;
};

#define SAFE_RELEASE(p) if (p) (p)->Release()

inline void ThrowIfFailed(HRESULT hr)
{
  if (FAILED(hr))
  {
    throw HrException(hr);
  }
}
#endif // DX_HELPER_H_
