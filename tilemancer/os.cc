#include "tilemancer/os.h"

const char OS_SEPARATOR_CHAR =
#ifdef TILEMANCER_OS_WINDOWS
    '\\'
#else
    '/'
#endif
    ;

#include <dirent.h>
#include <unistd.h>

const std::string OS_SEPARATOR_STRING = std::string(1, OS_SEPARATOR_CHAR);

const std::string executable_path() {
  const uint32_t size = 1024;
  char cwd[size] = {
      0,
  };
#ifdef TILEMANCER_OS_WINDOWS
  GetModuleFileName(NULL, cwd, size);
#elif defined(TILEMANCER_OS_OSX)
  _NSGetExecutablePath(cwd, (uint32_t*)&size);
#else
  ssize_t readSize = readlink("/proc/self/exe", cwd, size);
  if (readSize <= 0 || readSize == size) {
    printf("Could not determine executable path!\n");
    exit(1);
  }
  cwd[readSize] = 0;
#endif
  return std::string(cwd);
}

std::string GetFolder(const std::string& folder) {
  std::string cwd2 = executable_path();
  cwd2.erase(cwd2.rfind(OS_SEPARATOR_CHAR));
#ifdef TILEMANCER_OS_OSX
  cwd2.erase(cwd2.rfind('/'));
  cwd2.erase(cwd2.rfind('/'));
  cwd2.erase(cwd2.rfind('/'));
#endif
  cwd2 += OS_SEPARATOR_STRING + folder;
  return cwd2;
}

#ifdef TILEMANCER_WINDOWS
std::vector<std::string> FilesInFolder(const std::string& folder,
                                       const char* ext) {
  std::vector<std::string> files;

  DIR* dirr;
  struct dirent* ent;
  std::vector<BrowserFile*> temp;
  if ((dirr = opendir(folder.c_str())) != NULL) {
    while ((ent = readdir(dirr)) != NULL) {
      if (ent->d_name[0] != '.') {
        const std::string fn = ent->d_name;
        if (ext) {
          if (fn.substr(fn.find_last_of(".") + 1) != "lua") {
            continue;
          }
        }
        files.push_back(fn);
      }
    }
  }

  closedir(dirr);
  return files;
}
#endif

#ifdef TILEMANCER_OS_UNIX
std::vector<std::string> FilesInFolder(const std::string& folder,
                                       const char* ext) {
  int num_entries;
  struct dirent** entries = NULL;
  num_entries = scandir(folder.c_str(), &entries, NULL, NULL);

  std::vector<std::string> files;
  files.reserve(num_entries);

  for (int i = 0; i < num_entries; i++) {
    if (entries[i]->d_name[0] != '.') {
      const std::string& fn = entries[i]->d_name;
      if (ext) {
        if (fn.substr(fn.find_last_of(".") + 1) != "lua") {
          continue;
        }
      }
      files.push_back(fn);
    }
  }

  return files;
}
#endif