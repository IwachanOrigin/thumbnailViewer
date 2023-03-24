
#include "output_thumbnail.h"
#include "MFUtility.h"

OutputThumnail::OutputThumnail()
{
}

OutputThumnail::~OutputThumnail()
{
}

int OutputThumnail::open(const std::wstring inputFilename, const std::wstring outputFilename)
{
  HRESULT hr = S_OK;

  return hr;
}

void OutputThumnail::initializeAPI()
{
  CHECK_HR(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE), "COM initialisation failed.");
  CHECK_HR(MFStartup(MF_VERSION), "Media Foundation initialisation failed.");
}

void OutputThumnail::releaseAPI()
{
  
}
