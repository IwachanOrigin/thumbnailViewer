
#include <cstdio>
#include <iostream>
#include "output_thumbnail.h"

#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfplay.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "wmcodecdspuuid.lib")
#pragma comment(lib, "d2d1.lib")

void usage()
{
  std::wcout << "output_video_thumbnail.exe <input file name> <output file name>" << std::endl;
}

int main(int argc, char* argv[])
{
  if (argc != 2)
  {
    usage();
    return -1;
  }

  OutputThumnail ot;

  // Create API
  if (ot.createAPI())
  {
    // Output buffer
    ot.open(argv[1]);

    // Release
    ot.destroyAPI();
  }
  else
  {
    return -1;
  }

  return 0;
}
