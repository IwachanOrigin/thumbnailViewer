
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

  int open(const std::string inputFilename);

  bool createAPI();
  void destroyAPI();

private:
  RECT correctAspectRatio(const RECT& src, const MFRatio& srcPAR);
};

#endif // OUTPUT_THUMBNAIL_H_
