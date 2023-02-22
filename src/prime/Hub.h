#pragma once

enum class SectionID {
  Alliance_Contribution                    = -2035860712, // 0x86A73B18
  Alliance_Diplomacy_List                  = -1998741427, // 0x88DDA04D
  Navigation_Combat_Debug                  = -1993831053, // 0x89288D73
  SlideShowDialog                          = -1974142186, // 0x8A54FB16
  Tournament_Leaderboard                   = -1944855001, // 0x8C13DE27
  Research_LandingPage                     = -1936877406, // 0x8C8D98A2
  Bookmarks_Main                           = -1863601413, // 0x90EBB2FB
  Leaderboard_Alliances                    = -1853908838, // 0x917F989A
  Missions_AcceptedList                    = -1833888323, // 0x92B115BD
  ShipCosmetics_Preview                    = -1783421199, // 0x95B326F1
  OfficerAbilities                         = -1770476846, // 0x9678AAD2
  GameNews                                 = -1704696315, // 0x9A646605
  Missions_Achievements                    = -1663314948, // 0x9CDBD3FC
  Shop_AllianceChests                      = -1658878702, // 0x9D1F8512
  Combat_Scan                              = -1617308440, // 0x9F99D4E8
  Starbase_Armada_Building                 = -1582567019, // 0xA1ABF195
  ZDummyTerminator                         = -1582567018, // 0xA1ABF196
  OfficerReveal                            = -1560454013, // 0xA2FD5C83
  Reentry                                  = -1545897473, // 0xA3DB79FF
  Starbase_Interior                        = -1518342580, // 0xA57FEE4C
  OfficerGacha                             = -1446106612, // 0xA9CE2A0C
  Alliance_Permission                      = -1437368617, // 0xAA537ED7
  Tournament_Selection                     = -1398895946, // 0xAC9E8AB6
  Alliance_Create                          = -1339482620, // 0xB0291E04
  Navigation_Galaxy                        = -1300757605, // 0xB278039B
  Alliance_Armadas_Threats                 = -1240094941, // 0xB615A723
  Chat_Private_Message                     = -1232447932, // 0xB68A5644
  Alliance_Invite                          = -1170887663, // 0xBA35AC11
  Alliance_EditProfile                     = -1168827337, // 0xBA551C37
  TerritoryCapture_JoinTakeover            = -1155055432, // 0xBB2740B8
  OfficerInventory                         = -1154490138, // 0xBB2FE0E6
  PlayerProfile                            = -1152456856, // 0xBB4EE768
  BattlePass_Main                          = -1079277745, // 0xBFAB874F
  Bookmarks_Details                        = -1075632704, // 0xBFE325C0
  Navigation_Planet                        = -1033252317, // 0xC269D223
  Alliance_Help                            = -935334327,  // 0xC83FEE49
  Alliance_Join                            = -935265230,  // 0xC840FC32
  Alliance_Main                            = -935189311,  // 0xC84224C1
  Navigation_System                        = -934817094,  // 0xC847D2BA
  Missions_Rewards                         = -904826036,  // 0xCA11734C
  AwayTeamAssignments                      = -791072837,  // 0xD0D92FBB
  ShipManagement_Details                   = -763928158,  // 0xD27761A2
  InventoryList                            = -726704134,  // 0xD4AF5FFA
  Alliance_SendInvite                      = -726475335,  // 0xD4B2DDB9
  OfficerShowcase                          = -726094109,  // 0xD4B8AEE3
  Chat_Main                                = -672065984,  // 0xD7F11640
  Shop_Showcase                            = -653955370,  // 0xD9056ED6
  Shop_MainFactions                        = -653267623,  // 0xD90FED59
  Missions_DailyGoals                      = -644919297,  // 0xD98F4FFF
  OfficerConfirmSale                       = -548222927,  // 0xDF52C831
  Inbox_Main                               = -461074414,  // 0xE4849012
  OnScreen_Fleet                           = -412514502,  // 0xE769873A
  Bookmarks_Search_Coordinates             = -393757818,  // 0xE887BB86
  Alliance_Choose_Emblem                   = -342315942,  // 0xEB98AC5A
  Alliance_Armadas                         = -341922839,  // 0xEB9EABE9
  Chat_Private_List                        = -329495679,  // 0xEC5C4B81
  Starbase_Level_UpgradeWarning            = -328717641,  // 0xEC682AB7
  Shop_List                                = -100955065,  // 0xF9FB8C47
  Tournament_Detail                        = -72573081,   // 0xFBAC9F67
  Leaderboard_General                      = -15226394,   // 0xFF17A9E6
  AR_Ship                                  = -3902038,    // 0xFFC475AA
  AppInit                                  = 0,
  Shop                                     = 2576150,    // 0x00274F16
  Navigation_Default                       = 72648470,   // 0x04548716
  Login                                    = 73596745,   // 0x0462FF49
  Chat_Settings                            = 120009002,  // 0x0727312A
  DebugEmpty                               = 129029850,  // 0x07B0D6DA
  TerritoryCapture_HUB                     = 198032386,  // 0x0BCDBC02
  AvatarSelection                          = 228326419,  // 0x0D9BFC13
  OfficerGachaSelection                    = 253555936,  // 0x0F1CF4E0
  ServerTransferProgress                   = 301530299,  // 0x11F8FCBB
  Starbase_Exterior                        = 385452890,  // 0x16F98B5A
  ServerTransfer                           = 449193070,  // 0x1AC6246E
  Chat_Alliance                            = 492322110,  // 0x1D583D3E
  Combat_Report                            = 518419679,  // 0x1EE674DF
  Missions_Archived                        = 538435770,  // 0x2017E0BA
  ShipManagement_Selection                 = 561547500,  // 0x217888EC
  Shop_Refining_List                       = 620333674,  // 0x24F98A6A
  Chat_Block                               = 630965478,  // 0x259BC4E6
  ShipConstruction_HullSelection           = 658582829,  // 0x27412D2D
  TerritoryCapture_ContributionLeaderboard = 665740250,  // 0x27AE63DA
  Interstitial                             = 769047372,  // 0x2DD6BB4C
  ShipConstruction_Details                 = 883531056,  // 0x34A99D30
  Missions_AwayTeamsList                   = 963848782,  // 0x39732A4E
  Shop_Refining_Showcase                   = 985768825,  // 0x3AC1A379
  SpecialRewardScreen                      = 1014999988, // 0x3C7FABB4
  Starbase_ModuleDetails                   = 1100274458, // 0x4194DB1A
  TerritoryCapture_Embassy                 = 1121865311, // 0x42DE4E5F
  Shop_Summary                             = 1180221277, // 0x4658BF5D
  SlideShow                                = 1207462958, // 0x47F86C2E
  OnScreen_Self                            = 1233999840, // 0x498D57E0
  Research_TreeView                        = 1240388999, // 0x49EED587
  Alliance_Members                         = 1346039057, // 0x503AED11
  ShipCosmetics_Management                 = 1381234810, // 0x5253F87A
  ShipConstruction_Reveal                  = 1399203615, // 0x5366271F
  ShipCosmetics_Research                   = 1471406994, // 0x57B3E392
  Officer_Assignment                       = 1547574710, // 0x5C3E1DB6
  Alliance_Management                      = 1575235915, // 0x5DE4314B
  Leaderboard_Details                      = 1622643520, // 0x60B79340
  Missions_AvailableList                   = 1719419183, // 0x667C412F
  ShipManagement_Upgrade                   = 1742145916, // 0x67D7097C
  GameSettings                             = 1754181205, // 0x688EAE55
  ShipScrapping_List                       = 1844785206, // 0x6DF53036
  OnScreen_Station                         = 1883947040, // 0x704AC020
  RewardScreen                             = 1890775195, // 0x70B2F09B
  Shop_Reveal                              = 1934589238, // 0x734F7D36
  PrepareForReentry                        = 1937864029, // 0x7381755D
  Missions_Dialogue                        = 2020901552, // 0x787482B0
  OfficerRevealSummary                     = 2134661539, // 0x7F3C59A3
  Tournament_Group_Selection               = 2138751318, // 0x7F7AC156
};

struct SectionManager {
public:
  __declspec(property(get = __get_CurrentSection)) SectionID CurrentSection;

  void TriggerSectionChange(SectionID nextSectionID, void* args, bool forcedSectionChange = false,
                            bool isGoBackStep = false, bool allowSameSection = false)
  {
    static auto trigger =
        get_class_helper().GetMethod<void(void*, SectionID, void*, bool, bool isGoBackStep, bool allowSameSection)>(
            "TriggerSectionChange");
    trigger(this, nextSectionID, args, forcedSectionChange, isGoBackStep, allowSameSection);
  }

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Client.Sections", "SectionManager");
    return class_helper;
  }

public:
  SectionID __get_CurrentSection()
  {
    static auto field = get_class_helper().GetProperty("CurrentSection");
    return *field.Get<SectionID>(this);
  }
};

struct Hub {
  static SectionManager* get_SectionManager()
  {
    static auto field = get_class_helper().GetProperty("SectionManager");
    return field.GetRaw<SectionManager>(nullptr);
  }

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Client.Core", "Hub");
    return class_helper;
  }

public:
};