#include "il2cpp-functions.h"

#include <cstdio>

#if !_WIN32
#include <dlfcn.h>
#include <libgen.h>
#include <mach-o/dyld.h>

#define PATH_MAX 1024
#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus
#define DO_API(r, n, p) n##_t n;
#define DO_API_NO_RETURN(r, n, p) DO_API(r, n, p)
#include "il2cpp-api-functions.h"
#undef DO_API
#undef DO_API_NORETURN
#if defined(__cplusplus)
}
#endif // __cplusplus
#endif

void init_il2cpp_pointers()
{
#if !_WIN32
  char     buf[PATH_MAX];
  uint32_t bufsize = PATH_MAX;
  _NSGetExecutablePath(buf, &bufsize);

  char assembly_path[PATH_MAX];
  snprintf(assembly_path, sizeof(assembly_path), "%s/%s", dirname(buf), "../Frameworks/GameAssembly.dylib");
  auto assembly = dlopen(assembly_path, RTLD_LAZY | RTLD_GLOBAL);
#define DO_API(r, n, p) n = (n##_t)dlsym(assembly, #n);
#define DO_API_NO_RETURN(r, n, p) DO_API(r, n, p)
#include "il2cpp-api-functions.h"
#undef DO_API
#undef DO_API_NORETURN
#endif
}
