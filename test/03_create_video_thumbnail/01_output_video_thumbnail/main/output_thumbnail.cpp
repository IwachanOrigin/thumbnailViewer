
#include "output_thumbnail.h"
#include "MFUtility.h"

#include <mfidl.h>
#include <wrl/client.h>

using namespace Microsoft::WRL;

OutputThumnail::OutputThumnail()
{
}

OutputThumnail::~OutputThumnail()
{
}

int OutputThumnail::open(const std::wstring inputFilename, const std::wstring outputFilename)
{
  HRESULT hr = S_OK;
  ComPtr<IMFSourceResolver> pSourceResolver = nullptr;
  ComPtr<IUnknown> uSource = nullptr;
  ComPtr<IMFMediaSource> mediaFileSource = nullptr;
  ComPtr<IMFAttributes> pVideoReaderAttributes = nullptr;
  ComPtr<IMFSourceReader> pSourceReader = nullptr;
  ComPtr<IMFMediaType> pFileVideoMediaType = nullptr;
  ComPtr<IUnknown> spDecTransformUnk = nullptr;
  ComPtr<IMFTransform> pDecoderTransform = nullptr; // This is H264 Decoder MFT.
  ComPtr<IMFMediaType> pDecInputMediaType = nullptr;
  ComPtr<IMFMediaType> pDecOutputMediaType = nullptr;
  MF_OBJECT_TYPE ObjectType = MF_OBJECT_INVALID;
  DWORD mftStatus = 0;

  std::wofstream outputBuffer(outputFilename, std::ios::out | std::ios::binary);

  // Set up the reader for the file.
  CHECK_HR(MFCreateSourceResolver(pSourceResolver.Get()), "MFCreateSourceResolver failed.");
  CHECK_HR(pSourceResolver->CreateObjectFromURL(
    SOURCE_FILENAME,		        // URL of the source.
    MF_RESOLUTION_MEDIASOURCE,  // Create a source object.
    NULL,                       // Optional property store.
    &ObjectType,				        // Receives the created object type. 
    &uSource					          // Receives a pointer to the media source.
  ),
    "Failed to create media source resolver for file.");

  CHECK_HR(uSource->QueryInterface(IID_PPV_ARGS(mediaFileSource.Get())), "Failed to create media file source.");
  CHECK_HR(MFCreateAttributes(&pVideoReaderAttributes, 2), "Failed to create attributes object for video reader.");

  CHECK_HR(pVideoReaderAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID)
           , "Failed to set dev source attribute type for reader config.");

  CHECK_HR(pVideoReaderAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, 1)
           , "Failed to set enable video processing attribute type for reader config.");

  CHECK_HR(MFCreateSourceReaderFromMediaSource(mediaFileSource, pVideoReaderAttributes, &pSourceReader)
           , "Error creating media source reader.");

  CHECK_HR(pSourceReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, &pFileVideoMediaType)
           , "Error retrieving current media type from first video stream.");

  // Create H.264 decoder.
  CHECK_HR(CoCreateInstance(CLSID_CMSH264DecoderMFT, NULL, CLSCTX_INPROC_SERVER , IID_IUnknown, (void**)&spDecTransformUnk)
           , "Failed to create H264 decoder MFT.");

  CHECK_HR(spDecTransformUnk->QueryInterface(IID_PPV_ARGS(&pDecoderTransform)), "Failed to get IMFTransform interface from H264 decoder MFT object.");

  MFCreateMediaType(pDecInputMediaType.Get());
  CHECK_HR(pFileVideoMediaType->CopyAllItems(pDecInputMediaType), "Error copying media type attributes to decoder input media type.");
  CHECK_HR(pDecoderTransform->SetInputType(0, pDecInputMediaType, 0), "Failed to set input media type on H.264 decoder MFT.");

  MFCreateMediaType(pDecOutputMediaType.Get());
  CHECK_HR(pFileVideoMediaType->CopyAllItems(pDecOutputMediaType), "Error copying media type attributes to decoder input media type.");
  CHECK_HR(pDecOutputMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_IYUV), "Failed to set media sub type.");

  CHECK_HR(pDecoderTransform->SetOutputType(0, pDecOutputMediaType, 0), "Failed to set output media type on H.264 decoder MFT.");

  CHECK_HR(pDecoderTransform->GetInputStatus(0, &mftStatus), "Failed to get input status from H.264 decoder MFT.");
  if (MFT_INPUT_STATUS_ACCEPT_DATA != mftStatus)
  {
    printf("H.264 decoder MFT is not accepting data.\n");
    goto done;
  }

  std::cout << "H264 decoder output media type: " << GetMediaTypeDescription(pDecOutputMediaType) << std::endl << std::endl;

  CHECK_HR(pDecoderTransform->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, NULL), "Failed to process FLUSH command on H.264 decoder MFT.");
  CHECK_HR(pDecoderTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, NULL), "Failed to process BEGIN_STREAMING command on H.264 decoder MFT.");
  CHECK_HR(pDecoderTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, NULL), "Failed to process START_OF_STREAM command on H.264 decoder MFT.");

  // Start processing frames.
  ComPtr<IMFSample> pVideoSample = nullptr;
  ComPtr<IMFSample> pCopyVideoSample = nullptr;
  ComPtr<IMFSample> pH264DecodeOutSample = nullptr;
  DWORD streamIndex = 0, flags = 0;
  LONGLONG llVideoTimeStamp = 0, llSampleDuration = 0;
  int sampleCount = 0;
  DWORD sampleFlags = 0;
  BOOL h264DecodeTransformFlushed = FALSE;

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
      CHECK_HR(CreateAndCopySingleBufferIMFSample(pVideoSample, &pCopyVideoSample)
               , "Failed to copy single buffer IMF sample.");

      // Apply the H264 decoder transform
      CHECK_HR(pDecoderTransform->ProcessInput(0, pCopyVideoSample, 0)
               , "The H264 decoder ProcessInput call failed.");

      HRESULT getOutputResult = S_OK;
      while (getOutputResult == S_OK)
      {
        getOutputResult = GetTransformOutput(pDecoderTransform, &pH264DecodeOutSample, &h264DecodeTransformFlushed);
        if (getOutputResult != S_OK && getOutputResult != MF_E_TRANSFORM_NEED_MORE_INPUT)
        {
          printf("Error getting H264 decoder transform output, error code %.2X.\n", getOutputResult);
          goto done;
        }
        if (h264DecodeTransformFlushed == TRUE)
        {
          // H264 decoder format changed. Clear the capture file and start again.
          outputBuffer.close();
          outputBuffer.open(CAPTURE_FILENAME, std::ios::out | std::ios::binary);
        }
        else if (pH264DecodeOutSample != NULL)
        {
          // Write decoded sample to capture file.
          CHECK_HR(WriteSampleToFile(pH264DecodeOutSample, &outputBuffer),
            "Failed to write sample to file.");
        }
      }
      sampleCount++;
    }
  }

  outputBuffer.close();

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
