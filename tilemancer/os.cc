#include "tilemancer/os.h"

const char OS_SEPARATOR_CHAR =
#ifdef TILEMANCER_OS_WINDOWS
  '\\'
#else
  '/'
#endif
;

const std::string OS_SEPARATOR_STRING = std::string(1, OS_SEPARATOR_CHAR);
