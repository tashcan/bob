#pragma once
#if (defined(_WIN32) && defined(_M_X64)) || defined(__APPLE__)
#include "interop.h"
#include "settings/page_catalog.h"
namespace mod_settings::native
{
void RegisterSettingsSearch(PageCatalog& catalog);
void InstallSettingsSearch();
void OpenSettingsSearch(Il2CppObject* controller);
void CloseSettingsSearch();
bool FilterSettingsSearch(const PageCatalog::Page& page, std::string_view id);
void NavigateToSearchResult(std::string_view page, std::string_view item);
} // namespace mod_settings::native
#endif
