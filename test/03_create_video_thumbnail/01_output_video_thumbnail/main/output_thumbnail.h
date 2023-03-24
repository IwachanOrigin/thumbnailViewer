
#ifndef OUTPUT_THUMBNAIL_H_
#define OUTPUT_THUMBNAIL_H_

#include <string>

#include <mfapi.h>
#include <mfplay.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <wmcodecdsp.h>

class OutputThumnail
{
public:
  explicit OutputThumnail();
  ~OutputThumnail();

  int open(const std::wstring inputFilename, const std::wstring outputFilename);

  void initializeAPI();
  void releaseAPI();
};

#endif // OUTPUT_THUMBNAIL_H_
