#include "folder_manager.h"

#include <string>

#include <cstdio>
#include <dlfcn.h>
#include <errno.h>
#include <libgen.h>
#include <mach-o/dyld.h>
#include <unistd.h>

#include <CoreServices/CoreServices.h>

#define LD_LIBRARY_PATH_ENV "DYLD_LIBRARY_PATH"
#define LD_PRELOAD_ENV "DYLD_INSERT_LIBRARIES"

static char **appArgs = NULL;

static void die(const char *op)
{
  fprintf(stderr, "cliloader Error: %s\n", op);
  exit(1);
}

int main()
{
  auto AppPath =
      R"(Star Trek Fleet Command/Games/Star Trek Fleet Command/Star Trek Fleet Command/default/game/Star Trek Fleet Command.app/Contents/MacOS/Star Trek Fleet Command)";

  auto ApplicationSupportPath = (char *)fm::FolderManager::pathForDirectory(fm::NSApplicationSupportDirectory, fm::NSUserDomainMask);
  char GameExecutablePath[PATH_MAX];
  snprintf(GameExecutablePath, PATH_MAX, "%s/%s", ApplicationSupportPath, AppPath);

  char     buf[PATH_MAX];
  uint32_t bufsize = PATH_MAX;
  _NSGetExecutablePath(buf, &bufsize);

  std::string path = dirname(buf);

#define SETENV(_name, _value) setenv(_name, _value, 1)
  std::string ld_library_path     = path;
  const char *old_ld_library_path = getenv(LD_LIBRARY_PATH_ENV);
  if (old_ld_library_path) {
    ld_library_path += ":";
    ld_library_path += old_ld_library_path;
  }
  std::string ld_preload     = path + "/libstfc-community-patch.dylib";
  const char *old_ld_preload = getenv(LD_PRELOAD_ENV);
  if (old_ld_preload) {
    ld_preload += ":";
    ld_preload += old_ld_preload;
  }

  SETENV(LD_PRELOAD_ENV, ld_preload.c_str());
  SETENV(LD_LIBRARY_PATH_ENV, ld_library_path.c_str());

  char *const argument_list[] = {(char *)GameExecutablePath, NULL};

  if (execvp(argument_list[0], argument_list) == -1) {
    printf("Failed to launch app %d %s\n", errno, GameExecutablePath);
    return -1;
  }

  return 0;
}
