#include <cstdio>
#include <librsync.h>

extern "C" {
int rsync_apply(const char* source_file, const char* patch_file, const char* target_file)
{
  rs_stats_t stats;
  auto       basis_file = fopen(source_file, "rb");
  auto       delta_file = fopen(patch_file, "rb");
  auto       new_file   = fopen(target_file, "wb");

  auto result = rs_patch_file(basis_file, delta_file, new_file, &stats);
  fclose(basis_file);
  fclose(delta_file);
  fclose(new_file);
  return result;
}
}
