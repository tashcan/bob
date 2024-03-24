#pragma once

#include <cstdint>

enum Faction
{
    None = -1,
    Undefined = 0,
    Federation = 1,
    Klingon = 2,
    Romulan = 3,
};

struct HullSpec;

class SpecManager;
using GetHull_t = HullSpec *(*)(SpecManager *_this, uint64_t, void *);

#pragma pack(push, 1)
struct SpecManagerVtbl
{
    char pad0[0x1C8];
    GetHull_t GetHull;
    void *TypeDefinition;
};

// static_assert(offsetof(SpecManagerVtbl, GetHull) == 0x1C8);

class SpecManager
{
public:
    SpecManagerVtbl vtbl;
};
// static_assert(offsetof(SpecManager, vtbl) == 0x0);
#pragma pack(pop)


enum BuffCondition
{
    Invalid = -1,
    SelfExplorer = 0,
    SelfInterceptor = 1,
    SelfBattleship = 2,
    SelfSurveyor = 3,
    EnemyExplorer = 4,
    EnemyInterceptor = 5,
    EnemyBattleship = 6,
    EnemySentinel = 8,
    CondCargoFull = 9,
    CondCargoEmpty = 10,
    CondSelfMining = 12,
    SelfOrSentinelAttacked = 14,
    // TODO(alexander): Find a better name
    EnemyStronger = 16,
    CondEnemyHullFaction = 17,
    CondEnemyHostile = 27,
    CondEnemyPlayer = 28,
    CondSelfHasMorale = 33,
    CondSelfHasHullBreach = 34,
    CondSelfIsBurning = 35,
    CondEnemyHullBreach = 37,
    CondEnemyBurning = 38,
    CondModuleEnergy = 39,
    CondModuleKinetic = 40,
    CondHitEnemyWithEnergy = 41,
    CondHitEnemyWithKinetic = 42,
    CondHullHealthBelow = 43,
    CondHullHealthBelowStartOfCombat = 44,
    // I wonder what could be here...maybe some server only things? Who knows
    CondSelfShipGrade = 102,
    CondSelfHullFaction = 113,
    CondSelfAtStation = 114,
    SelfSentinel = 116,
    CondEnemySentinel = 117, // TODO(alexander): What wait???? We have the before aswell, why is this a thing again??? see condition 8
    CondResourceId = 118,
    CondSelfModuleId = 119,
    CondSelfShipgrade1 = 120,
    CondSelfShipgrade2 = 121,
    CondSelfShipgrade3 = 122,
    CondSelfShipgrade4 = 123,
    CondSelfIsFederation = 124, // Not sure why they haven't used CondSelfHullFaction for this...but hey whatever
    CondSelfIsRomulan = 125,
    CondSelfIsKlingon = 126,
    CondSelfFranklin = 127,
    CondSelfIsArmada = 129,
    CondSelfIsNotArmada = 131,
    CondTargetShipGrade1 = 132,
    CondTargetShipGrade2 = 133,
    CondTargetShipGrade3 = 134,
    CondTargetShipGrade4 = 135,
    CondSelfHullMudd = 150,
    CondSelfHullBotanyBay = 151,
    CondSelfHullDiscovery = 152,
    CondSelfHullSarcophagus = 153,
    CondSelfHullVidar = 154,
    CondSelfHullDvor = 155,
    CondSelfHullFranklin = 156,
    CondSelfHullBlackJellyfish = 157,
    CondSelfHullFranklin2 = 158,
    CondSelfHullEnterprise = 159,
    CondSelfHullEnterpriseA = 160,
    CondSelfHullD4 = 161,
    CondSelfHullHeghta = 162,
    CondSelfHullAugur = 163,
    CondSelfHullTribune = 164,
    CondSelfHullSaladin = 165,
    CondSelfHullCenturion = 166,
    CondSelfHullBortas = 167,
    CondSelfIsSynciate30 = 203,
    CondSelfIsMantis = 207,
};