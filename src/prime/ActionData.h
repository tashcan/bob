#pragma once

#include <il2cpp/il2cpp_helper.h>

using IActionData = void*;

enum class ActionType : int32_t {
  Invalid  = -1,
  Upgrade  = 1433481724,
  Purchase = 1807968545,
  Manage   = -1997567611,
  Harvest  = -1933286583,
  Repair   = -1850668115,
  View     = 2666181,
  Recall   = -1851055311,
  Promote  = 1355442080,
  Demote   = 2043415476,
  Move     = 2404337,
  Attack   = 1971575400,
  Assign   = 1970629903,
  Add      = 65665,
  Remove   = -1850743644,
  Claim    = 65189916,
  Chat     = 2099064,
  Help     = 2245473,
  Invite   = -2099832023,
  Report   = -1850654380,
  Copy     = 2106261,
  Share    = 79847359
};

enum ActionBehaviour {
  Default             = 1,
  Instant             = 2,
  Speedup             = 4,
  Collect             = 8,
  AskHelp             = 16,
  RequestConfirmation = 32,
};