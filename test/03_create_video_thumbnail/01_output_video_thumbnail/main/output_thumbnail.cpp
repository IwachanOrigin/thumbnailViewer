
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
  IMFSourceResolver* pSourceResolver = NULL;
  IUnknown* uSource = NULL;
  IMFMediaSource* mediaFileSource = NULL;
  IMFAttributes* pVideoReaderAttributes = NULL;
  IMFSourceReader* pSourceReader = NULL;
  IMFMediaType* pFileVideoMediaType = NULL;
  IUnknown* spDecTransformUnk = NULL;
  IMFTransform* pDecoderTransform = NULL; // This is H264 Decoder MFT.
  IMFMediaType* pDecInputMediaType = NULL, * pDecOutputMediaType = NULL;
  MF_OBJECT_TYPE ObjectType = MF_OBJECT_INVALID;
  DWORD mftStatus = 0;

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
