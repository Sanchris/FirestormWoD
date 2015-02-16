/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _PLAYER_H
#define _PLAYER_H

#include "AchievementMgr.h"
#include "ArchaeologyMgr.h"
#include "Arena.h"
#include "Battleground.h"
#include "BattlePetMgr.h"
#include "Bag.h"
#include "Common.h"
#include "DatabaseEnv.h"
#include "DBCEnums.h"
#include "GroupReference.h"
#include "ItemPrototype.h"
#include "Item.h"
#include "MapReference.h"
#include "NPCHandler.h"
#include "Pet.h"
#include "QuestDef.h"
#include "ReputationMgr.h"
#include "Unit.h"
#include "Util.h"                                           // for Tokenizer typedef
#include "WorldSession.h"
#include "PhaseMgr.h"
#include "CUFProfiles.h"
#include "CinematicPathMgr.h"
#include "VignetteMgr.hpp"
#include "BitSet.hpp"

// for template
#include "SpellMgr.h"

#include <string>
#include <vector>
#include <mutex>
#include <ace/Stack_Trace.h>

struct Mail;
struct ItemExtendedCostEntry;
class Channel;
class Creature;
class DynamicObject;
class Group;
class Guild;
class OutdoorPvP;
class Pet;
class PlayerMenu;
class PlayerSocial;
class SpellCastTargets;
class UpdateMask;
class PhaseMgr;
class SceneObject;

typedef std::deque<Mail*> PlayerMails;
typedef std::set<uint32> DailyLootsCooldowns;

#define PLAYER_MAX_SKILLS           128
#define DEFAULT_MAX_PRIMARY_TRADE_SKILL 2
#define PLAYER_EXPLORED_ZONES_SIZE  200

/// 6.0.3 19116
enum ToastTypes
{
    TOAST_TYPE_MONEY        = 0,
    TOAST_TYPE_NEW_CURRENCY = 1,
    TOAST_TYPE_UNK1         = 2,
    TOAST_TYPE_NEW_ITEM     = 3,
};
/// 6.0.3 19116
enum DisplayToastMethod
{
    DISPLAY_TOAST_METHOD_UNK1                               = 0x0,
    DISPLAY_TOAST_METHOD_LOOT                               = 0x1,
    DISPLAY_TOAST_METHOD_PET_BATTLE_LOOT                    = 0x2,
    DISPLAY_TOAST_METHOD_CURRENCY_OR_GOLD                   = 0x3,
    DISPLAY_TOAST_METHOD_GARRISON_MISSION_BONUS_ROLL_LOOT_1 = 0x4,
    DISPLAY_TOAST_METHOD_LOOT_TOAST_UPGRADE_1               = 0x5,
    DISPLAY_TOAST_METHOD_LOOT_TOAST_UPGRADE_2               = 0x6,
    DISPLAY_TOAST_METHOD_UNK3                               = 0x7,
    DISPLAY_TOAST_METHOD_GARRISON_MISSION_BONUS_ROLL_LOOT_2 = 0x8,
    DISPLAY_TOAST_METHOD_PVP_FACTION_LOOT_TOAST             = 0x9,
    DISPLAY_TOAST_METHOD_GARRISON_CACHE                     = 0xA,
};

// Note: SPELLMOD_* values is aura types in fact
enum SpellModType
{
    SPELLMOD_FLAT         = 107,                            // SPELL_AURA_ADD_FLAT_MODIFIER
    SPELLMOD_PCT          = 108                             // SPELL_AURA_ADD_PCT_MODIFIER
};

// 2^n values, Player::m_isunderwater is a bitmask. These are Trinity internal values, they are never send to any client
enum PlayerUnderwaterState
{
    UNDERWATER_NONE                     = 0x00,
    UNDERWATER_INWATER                  = 0x01,             // terrain type is water and player is afflicted by it
    UNDERWATER_INLAVA                   = 0x02,             // terrain type is lava and player is afflicted by it
    UNDERWATER_INSLIME                  = 0x04,             // terrain type is lava and player is afflicted by it
    UNDERWARER_INDARKWATER              = 0x08,             // terrain type is dark water and player is afflicted by it

    UNDERWATER_EXIST_TIMERS             = 0x10
};

enum BuyBankSlotResult
{
    ERR_BANKSLOT_FAILED_TOO_MANY    = 0,
    ERR_BANKSLOT_INSUFFICIENT_FUNDS = 1,
    ERR_BANKSLOT_NOTBANKER          = 2,
    ERR_BANKSLOT_OK                 = 3
};

enum PlayerSpellState
{
    PLAYERSPELL_UNCHANGED = 0,
    PLAYERSPELL_CHANGED   = 1,
    PLAYERSPELL_NEW       = 2,
    PLAYERSPELL_REMOVED   = 3,
    PLAYERSPELL_TEMPORARY = 4
};

struct PlayerSpell
{
    PlayerSpellState state : 8;
    bool active            : 1;                             // show in spellbook
    bool dependent         : 1;                             // learned as result another spell learn, skill grow, quest reward, etc
    bool disabled          : 1;                             // first rank has been learned in result talent learn but currently talent unlearned, save max learned ranks
    bool IsMountFavorite   : 1;                             // Is flagged as favorite mount spell
};

struct PlayerTalent
{
    PlayerSpellState state : 8;
    uint8 spec             : 8;
};

enum TalentTree // talent tabs
{
    TALENT_TREE_WARRIOR_ARMS         = 746,
    TALENT_TREE_WARRIOR_FURY         = 815,
    TALENT_TREE_WARRIOR_PROTECTION   = 845,
    TALENT_TREE_PALADIN_HOLY         = 831,
    TALENT_TREE_PALADIN_PROTECTION   = 839,
    TALENT_TREE_PALADIN_RETRIBUTION  = 855,
    TALENT_TREE_HUNTER_BEAST_MASTERY = 811,
    TALENT_TREE_HUNTER_MARKSMANSHIP  = 807,
    TALENT_TREE_HUNTER_SURVIVAL      = 809,
    TALENT_TREE_ROGUE_ASSASSINATION  = 182,
    TALENT_TREE_ROGUE_COMBAT         = 181,
    TALENT_TREE_ROGUE_SUBTLETY       = 183,
    TALENT_TREE_PRIEST_DISCIPLINE    = 760,
    TALENT_TREE_PRIEST_HOLY          = 813,
    TALENT_TREE_PRIEST_SHADOW        = 795,
    TALENT_TREE_DEATH_KNIGHT_BLOOD   = 398,
    TALENT_TREE_DEATH_KNIGHT_FROST   = 399,
    TALENT_TREE_DEATH_KNIGHT_UNHOLY  = 400,
    TALENT_TREE_SHAMAN_ELEMENTAL     = 261,
    TALENT_TREE_SHAMAN_ENHANCEMENT   = 263,
    TALENT_TREE_SHAMAN_RESTORATION   = 262,
    TALENT_TREE_MAGE_ARCANE          = 799,
    TALENT_TREE_MAGE_FIRE            = 851,
    TALENT_TREE_MAGE_FROST           = 823,
    TALENT_TREE_WARLOCK_AFFLICTION   = 871,
    TALENT_TREE_WARLOCK_DEMONOLOGY   = 867,
    TALENT_TREE_WARLOCK_DESTRUCTION  = 865,
    TALENT_TREE_DRUID_BALANCE        = 752,
    TALENT_TREE_DRUID_FERAL_COMBAT   = 750,
    TALENT_TREE_DRUID_RESTORATION    = 748
};

// Spell modifier (used for modify other spells)
struct SpellModifier
{
    SpellModifier(AuraPtr _ownerAura = NULLAURA) : op(SPELLMOD_DAMAGE), type(SPELLMOD_FLAT), charges(0), value(0), mask(), spellId(0), ownerAura(_ownerAura) {}
    SpellModOp   op   : 8;
    SpellModType type : 8;
    int16 charges     : 16;
    int32 value;
    flag128 mask;
    uint32 spellId;
    constAuraPtr ownerAura;
};

enum PlayerCurrencyState
{
   PLAYERCURRENCY_UNCHANGED = 0,
   PLAYERCURRENCY_CHANGED   = 1,
   PLAYERCURRENCY_NEW       = 2,
   PLAYERCURRENCY_REMOVED   = 3     //not removed just set count == 0
};

struct PlayerCurrency
{
   PlayerCurrencyState state;
   uint32 totalCount;
   uint32 weekCount;
   uint32 seasonTotal;
   uint32 weekCap;
   uint8 flags;
   bool needResetCap;
};

typedef ACE_Based::LockedMap<uint32, PlayerTalent*> PlayerTalentMap;
typedef ACE_Based::LockedMap<uint32, PlayerSpell*> PlayerSpellMap;
typedef std::list<SpellModifier*> SpellModList;
typedef ACE_Based::LockedMap<uint32, PlayerCurrency> PlayerCurrenciesMap;

typedef std::list<uint64> WhisperListContainer;

struct SpellCooldown
{
    uint64 end;
    uint16 itemid;
};

typedef std::map<uint32, SpellCooldown> SpellCooldowns;
typedef ACE_Based::LockedMap<uint32 /*instanceId*/, time_t/*releaseTime*/> InstanceTimeMap;

enum TrainerSpellState
{
    TRAINER_SPELL_GRAY           = 0,
    TRAINER_SPELL_GREEN          = 1,
    TRAINER_SPELL_RED            = 2,
    TRAINER_SPELL_GREEN_DISABLED = 10                       // custom value, not send to client: formally green but learn not allowed
};

enum ActionButtonUpdateState
{
    ACTIONBUTTON_UNCHANGED = 0,
    ACTIONBUTTON_CHANGED = 1,
    ACTIONBUTTON_NEW = 2,
    ACTIONBUTTON_DELETED = 3
};

enum ActionButtonType
{
    ACTION_BUTTON_SPELL = 0x00,
    ACTION_BUTTON_C = 0x01,                         // click?
    ACTION_BUTTON_EQSET = 0x20,
    ACTION_BUTTON_DROPDOWN = 0x30,
    ACTION_BUTTON_MACRO = 0x40,
    ACTION_BUTTON_CMACRO = ACTION_BUTTON_C | ACTION_BUTTON_MACRO,
    ACTION_BUTTON_ITEM = 0x80
};

#define ACTION_BUTTON_ACTION(X) (uint64(X) & 0x00000000FFFFFFFF)
#define ACTION_BUTTON_TYPE(X)   ((uint64(X) & 0xFF00000000000000) >> 56)
#define MAX_ACTION_BUTTON_ACTION_VALUE (0x0000000000FFFFFF)

struct ActionButtonPacket
{
    uint32 id;
    uint32 type;
};

struct ActionButton
{
    ActionButton() : packedData(0), uState(ACTIONBUTTON_NEW) {}

    uint64 packedData;
    ActionButtonUpdateState uState;

    // helpers
    ActionButtonType GetType() const {
        return ActionButtonType(ACTION_BUTTON_TYPE(packedData));
    }
    uint32 GetAction() const {
        return ACTION_BUTTON_ACTION(packedData);
    }
    void SetActionAndType(uint32 action, ActionButtonType type)
    {
        uint64 newData = uint64(action) | (uint64(type) << 56);
        if (newData != packedData || uState == ACTIONBUTTON_DELETED)
        {
            packedData = newData;
            if (uState != ACTIONBUTTON_NEW)
                uState = ACTIONBUTTON_CHANGED;
        }
    }
};

#define  MAX_ACTION_BUTTONS 132                             //checked in 6.0.1

typedef std::map<uint8, ActionButton> ActionButtonList;

struct PlayerCreateInfoItem
{
    PlayerCreateInfoItem(uint32 id, uint32 amount) : item_id(id), item_amount(amount) {}

    uint32 item_id;
    uint32 item_amount;
};

typedef std::list<PlayerCreateInfoItem> PlayerCreateInfoItems;

struct PlayerLevelInfo
{
    PlayerLevelInfo() { for (uint8 i=0; i < MAX_STATS; ++i) stats[i] = 0; }

    uint16 stats[MAX_STATS];
};

typedef std::list<uint32> PlayerCreateInfoSpells;

struct PlayerCreateInfoAction
{
    PlayerCreateInfoAction() : button(0), type(0), action(0) {}
    PlayerCreateInfoAction(uint8 _button, uint32 _action, uint8 _type) : button(_button), type(_type), action(_action) {}

    uint8 button;
    uint8 type;
    uint32 action;
};

typedef std::list<PlayerCreateInfoAction> PlayerCreateInfoActions;

struct PlayerInfo
{
                                                            // existence checked by displayId != 0
    PlayerInfo() : displayId_m(0), displayId_f(0), levelInfo(NULL)
    {
    }

    uint32 mapId;
    uint32 areaId;
    float positionX;
    float positionY;
    float positionZ;
    float orientation;
    uint16 displayId_m;
    uint16 displayId_f;
    PlayerCreateInfoItems item;
    PlayerCreateInfoSpells spell;
    PlayerCreateInfoActions action;

    PlayerLevelInfo* levelInfo;                             //[level-1] 0..MaxPlayerLevel-1
};

struct PvPInfo
{
    PvPInfo() : inHostileArea(false), inNoPvPArea(false), inFFAPvPArea(false), endTimer(0) {}

    bool inHostileArea;
    bool inNoPvPArea;
    bool inFFAPvPArea;
    time_t endTimer;
};

struct DuelInfo
{
    DuelInfo() : initiator(NULL), opponent(NULL), startTimer(0), startTime(0), outOfBound(0), isMounted(false) {}

    Player* initiator;
    Player* opponent;
    time_t startTimer;
    time_t startTime;
    time_t outOfBound;
    bool isMounted;
    bool started;
};

struct Areas
{
    uint32 areaID;
    uint32 areaFlag;
    float x1;
    float x2;
    float y1;
    float y2;
};

#define MAX_RUNES       6

enum RuneCooldowns
{
    RUNE_BASE_COOLDOWN  = 10000,
    RUNE_MISS_COOLDOWN  = 1500,     // cooldown applied on runes when the spell misses
};

enum RuneType
{
    RUNE_BLOOD      = 0,
    RUNE_UNHOLY     = 1,
    RUNE_FROST      = 2,
    RUNE_DEATH      = 3,
    NUM_RUNE_TYPES  = 4
};

struct RuneInfo
{
    uint8 BaseRune;
    uint8 CurrentRune;
    uint32 Cooldown;
    uint32 spell_id;
    bool DeathUsed;
    bool Permanently;

    RuneInfo()
    {
        BaseRune = 0;
        CurrentRune = 0;
        Cooldown = 0;
        spell_id = 0;
        DeathUsed = false;
        Permanently = false;
    }
};

struct Runes
{
    RuneInfo runes[MAX_RUNES];
    uint8 runeState;                                        // mask of available runes

    void SetRuneState(uint8 index, bool set = true)
    {
        if (set)
            runeState |= (1 << index);                      // usable
        else
            runeState &= ~(1 << index);                     // on cooldown
    }
};

struct EnchantDuration
{
    EnchantDuration() : item(NULL), slot(MAX_ENCHANTMENT_SLOT), leftduration(0) {};
    EnchantDuration(Item* _item, EnchantmentSlot _slot, uint32 _leftduration) : item(_item), slot(_slot),
        leftduration(_leftduration){ ASSERT(item); };

    Item* item;
    EnchantmentSlot slot;
    uint32 leftduration;
};

typedef std::list<EnchantDuration> EnchantDurationList;
typedef std::list<Item*> ItemDurationList;

enum DrunkenState
{
    DRUNKEN_SOBER   = 0,
    DRUNKEN_TIPSY   = 1,
    DRUNKEN_DRUNK   = 2,
    DRUNKEN_SMASHED = 3
};

#define MAX_DRUNKEN   4

enum PlayerFlags
{
    PLAYER_FLAGS_GROUP_LEADER           = 0x00000001,
    PLAYER_FLAGS_AFK                    = 0x00000002,
    PLAYER_FLAGS_DND                    = 0x00000004,
    PLAYER_FLAGS_GM                     = 0x00000008,
    PLAYER_FLAGS_GHOST                  = 0x00000010,
    PLAYER_FLAGS_RESTING                = 0x00000020,
    PLAYER_FLAGS_UNK6                   = 0x00000040,
    PLAYER_FLAGS_UNK7                   = 0x00000080,       // pre-3.0.3 PLAYER_FIELD_PLAYER_FLAGS_FFA_PVP flag for FFA PVP state
    PLAYER_FLAGS_CONTESTED_PVP          = 0x00000100,       // Player has been involved in a PvP combat and will be attacked by contested guards
    PLAYER_FLAGS_IN_PVP                 = 0x00000200,
    PLAYER_FLAGS_HIDE_HELM              = 0x00000400,
    PLAYER_FLAGS_HIDE_CLOAK             = 0x00000800,
    PLAYER_FLAGS_PLAYED_LONG_TIME       = 0x00001000,       // played long time
    PLAYER_FLAGS_PLAYED_TOO_LONG        = 0x00002000,       // played too long time
    PLAYER_FLAGS_IS_OUT_OF_BOUNDS       = 0x00004000,
    PLAYER_FLAGS_DEVELOPER              = 0x00008000,       // <Dev> prefix for something?
    PLAYER_FLAGS_UNK16                  = 0x00010000,       // pre-3.0.3 PLAYER_FIELD_PLAYER_FLAGS_SANCTUARY flag for player entered sanctuary
    PLAYER_FLAGS_TAXI_BENCHMARK         = 0x00020000,       // taxi benchmark mode (on/off) (2.0.1)
    PLAYER_FLAGS_PVP_TIMER              = 0x00040000,       // 3.0.2, pvp timer active (after you disable pvp manually)
    PLAYER_FLAGS_UBER                   = 0x00080000,
    PLAYER_FLAGS_UNK20                  = 0x00100000,
    PLAYER_FLAGS_UNK21                  = 0x00200000,
    PLAYER_FLAGS_COMMENTATOR2           = 0x00400000,
    PLAYER_ALLOW_ONLY_ABILITY           = 0x00800000,       // used by bladestorm and killing spree, allowed only spells with SPELL_ATTR0_REQ_AMMO, SPELL_EFFECT_ATTACK, checked only for active player
    PLAYER_FLAGS_HAS_BATTLE_PET_TRAINING= 0x01000000,       // allowed to use battle pet combat system
    PLAYER_FLAGS_NO_XP_GAIN             = 0x02000000,
    PLAYER_FLAGS_UNK26                  = 0x04000000,
    PLAYER_FLAGS_AUTO_DECLINE_GUILD     = 0x08000000,       // Automatically declines guild invites
    PLAYER_FLAGS_GUILD_LEVEL_ENABLED    = 0x10000000,       // Lua_GetGuildLevelEnabled() - enables guild leveling related UI
    PLAYER_FLAGS_VOID_UNLOCKED          = 0x20000000,       // void storage
    PLAYER_FLAGS_UNK30                  = 0x40000000,
    PLAYER_FLAGS_UNK31                  = 0x80000000,
};

enum PlayerFlagsEx
{
    PLAYER_FLAGS_EX_REAGENT_BANK_UNLOCKED = 0x00000001
};

// used for PLAYER_FIELD_KNOWN_TITLES field (uint64), (1<<bit_index) without (-1)
// can't use enum for uint64 values
#define PLAYER_TITLE_DISABLED              UI64LIT(0x0000000000000000)
#define PLAYER_TITLE_NONE                  UI64LIT(0x0000000000000001)
#define PLAYER_TITLE_PRIVATE               UI64LIT(0x0000000000000002) // 1
#define PLAYER_TITLE_CORPORAL              UI64LIT(0x0000000000000004) // 2
#define PLAYER_TITLE_SERGEANT_A            UI64LIT(0x0000000000000008) // 3
#define PLAYER_TITLE_MASTER_SERGEANT       UI64LIT(0x0000000000000010) // 4
#define PLAYER_TITLE_SERGEANT_MAJOR        UI64LIT(0x0000000000000020) // 5
#define PLAYER_TITLE_KNIGHT                UI64LIT(0x0000000000000040) // 6
#define PLAYER_TITLE_KNIGHT_LIEUTENANT     UI64LIT(0x0000000000000080) // 7
#define PLAYER_TITLE_KNIGHT_CAPTAIN        UI64LIT(0x0000000000000100) // 8
#define PLAYER_TITLE_KNIGHT_CHAMPION       UI64LIT(0x0000000000000200) // 9
#define PLAYER_TITLE_LIEUTENANT_COMMANDER  UI64LIT(0x0000000000000400) // 10
#define PLAYER_TITLE_COMMANDER             UI64LIT(0x0000000000000800) // 11
#define PLAYER_TITLE_MARSHAL               UI64LIT(0x0000000000001000) // 12
#define PLAYER_TITLE_FIELD_MARSHAL         UI64LIT(0x0000000000002000) // 13
#define PLAYER_TITLE_GRAND_MARSHAL         UI64LIT(0x0000000000004000) // 14
#define PLAYER_TITLE_SCOUT                 UI64LIT(0x0000000000008000) // 15
#define PLAYER_TITLE_GRUNT                 UI64LIT(0x0000000000010000) // 16
#define PLAYER_TITLE_SERGEANT_H            UI64LIT(0x0000000000020000) // 17
#define PLAYER_TITLE_SENIOR_SERGEANT       UI64LIT(0x0000000000040000) // 18
#define PLAYER_TITLE_FIRST_SERGEANT        UI64LIT(0x0000000000080000) // 19
#define PLAYER_TITLE_STONE_GUARD           UI64LIT(0x0000000000100000) // 20
#define PLAYER_TITLE_BLOOD_GUARD           UI64LIT(0x0000000000200000) // 21
#define PLAYER_TITLE_LEGIONNAIRE           UI64LIT(0x0000000000400000) // 22
#define PLAYER_TITLE_CENTURION             UI64LIT(0x0000000000800000) // 23
#define PLAYER_TITLE_CHAMPION              UI64LIT(0x0000000001000000) // 24
#define PLAYER_TITLE_LIEUTENANT_GENERAL    UI64LIT(0x0000000002000000) // 25
#define PLAYER_TITLE_GENERAL               UI64LIT(0x0000000004000000) // 26
#define PLAYER_TITLE_WARLORD               UI64LIT(0x0000000008000000) // 27
#define PLAYER_TITLE_HIGH_WARLORD          UI64LIT(0x0000000010000000) // 28
#define PLAYER_TITLE_GLADIATOR             UI64LIT(0x0000000020000000) // 29
#define PLAYER_TITLE_DUELIST               UI64LIT(0x0000000040000000) // 30
#define PLAYER_TITLE_RIVAL                 UI64LIT(0x0000000080000000) // 31
#define PLAYER_TITLE_CHALLENGER            UI64LIT(0x0000000100000000) // 32
#define PLAYER_TITLE_SCARAB_LORD           UI64LIT(0x0000000200000000) // 33
#define PLAYER_TITLE_CONQUEROR             UI64LIT(0x0000000400000000) // 34
#define PLAYER_TITLE_JUSTICAR              UI64LIT(0x0000000800000000) // 35
#define PLAYER_TITLE_CHAMPION_OF_THE_NAARU UI64LIT(0x0000001000000000) // 36
#define PLAYER_TITLE_MERCILESS_GLADIATOR   UI64LIT(0x0000002000000000) // 37
#define PLAYER_TITLE_OF_THE_SHATTERED_SUN  UI64LIT(0x0000004000000000) // 38
#define PLAYER_TITLE_HAND_OF_ADAL          UI64LIT(0x0000008000000000) // 39
#define PLAYER_TITLE_VENGEFUL_GLADIATOR    UI64LIT(0x0000010000000000) // 40

#define KNOWN_TITLES_SIZE   10
#define MAX_TITLE_INDEX     (KNOWN_TITLES_SIZE*64)          // 5 uint64 fields

#define ECLIPSE_FULL_CYCLE_DURATION 40

// used in PLAYER_FIELD_BYTES values
enum PlayerFieldByteFlags
{
    PLAYER_FIELD_BYTE_TRACK_STEALTHED   = 0x00000002,
    PLAYER_FIELD_BYTE_RELEASE_TIMER     = 0x00000008,       // Display time till auto release spirit
    PLAYER_FIELD_BYTE_NO_RELEASE_WINDOW = 0x00000010        // Display no "release spirit" window at all
};

// used in PLAYER_FIELD_BYTES2 values
enum PlayerFieldByte2Flags
{
    PLAYER_FIELD_BYTE2_NONE                 = 0x00,
    PLAYER_FIELD_BYTE2_STEALTH              = 0x20,
    PLAYER_FIELD_BYTE2_INVISIBILITY_GLOW    = 0x40
};

enum MirrorTimerType
{
    FATIGUE_TIMER      = 0,
    BREATH_TIMER       = 1,
    FIRE_TIMER         = 2 // feign death
};
#define MAX_TIMERS      3
#define DISABLED_MIRROR_TIMER   -1

// 2^n values
enum PlayerExtraFlags
{
    // gm abilities
    PLAYER_EXTRA_GM_ON              = 0x0001,
    PLAYER_EXTRA_ACCEPT_WHISPERS    = 0x0004,
    PLAYER_EXTRA_TAXICHEAT          = 0x0008,
    PLAYER_EXTRA_GM_INVISIBLE       = 0x0010,
    PLAYER_EXTRA_GM_CHAT            = 0x0020,               // Show GM badge in chat messages

    // other states
    PLAYER_EXTRA_PVP_DEATH          = 0x0100                // store PvP death status until corpse creating.
};

// 2^n values
enum AtLoginFlags
{
    AT_LOGIN_NONE                   = 0x000,
    AT_LOGIN_RENAME                 = 0x001,
    AT_LOGIN_RESET_SPELLS           = 0x002,
    AT_LOGIN_RESET_TALENTS          = 0x004,
    AT_LOGIN_CUSTOMIZE              = 0x008,
    AT_LOGIN_RESET_PET_TALENTS      = 0x010,
    AT_LOGIN_FIRST                  = 0x020,
    AT_LOGIN_CHANGE_FACTION         = 0x040,
    AT_LOGIN_CHANGE_RACE            = 0x080,
    AT_LOGIN_UNLOCK                 = 0x100,
    AT_LOGIN_LOCKED_FOR_TRANSFER    = 0x200
};

typedef std::map<uint32, QuestStatusData> QuestStatusMap;
typedef std::map<uint32, uint32> QuestObjectiveStatusMap;
typedef std::set<uint32> RewardedQuestSet;

//               quest,  keep
typedef std::map<uint32, bool> QuestStatusSaveMap;

// Size (in bytes) of client completed quests bit map
#define QUESTS_COMPLETED_BITS_SIZE 2500

enum QuestSlotOffsets
{
    QUEST_ID_OFFSET     = 0,
    QUEST_STATE_OFFSET  = 1,
    QUEST_COUNTS_OFFSET = 2,
    QUEST_TIME_OFFSET   = 4
};

#define MAX_QUEST_OFFSET 15

enum QuestSlotStateMask
{
    QUEST_STATE_NONE            = 0x00000,
    QUEST_STATE_COMPLETE        = 0x00001,
    QUEST_STATE_FAIL            = 0x00002,
    QUEST_STATE_OBJ_0_COMPLETE  = 0x00100,
    QUEST_STATE_OBJ_1_COMPLETE  = 0x00200,
    QUEST_STATE_OBJ_2_COMPLETE  = 0x00400,
    QUEST_STATE_OBJ_3_COMPLETE  = 0x00800,
    QUEST_STATE_OBJ_4_COMPLETE  = 0x01000,
    QUEST_STATE_OBJ_5_COMPLETE  = 0x02000,
    QUEST_STATE_OBJ_6_COMPLETE  = 0x04000,
    QUEST_STATE_OBJ_7_COMPLETE  = 0x08000,
    QUEST_STATE_OBJ_8_COMPLETE  = 0x10000,
    QUEST_STATE_OBJ_9_COMPLETE  = 0x20000,
    QUEST_STATE_OBJ_10_COMPLETE = 0x40000,
};

enum SkillUpdateState
{
    SKILL_UNCHANGED     = 0,
    SKILL_CHANGED       = 1,
    SKILL_NEW           = 2,
    SKILL_DELETED       = 3
};

struct SkillStatusData
{
    SkillStatusData(uint8 _pos, SkillUpdateState _uState) : pos(_pos), uState(_uState)
    {
    }
    uint8 pos;
    SkillUpdateState uState;
};

typedef ACE_Based::LockedMap<uint32, SkillStatusData> SkillStatusMap;

class Quest;
class Spell;
class Item;
class WorldSession;
class BattlePet;

enum PlayerSlots
{
    // first slot for item stored (in any way in player m_items data)
    PLAYER_SLOT_START           = 0,
    // last+1 slot for item stored (in any way in player m_items data)
    PLAYER_SLOT_END             = 184,
    PLAYER_SLOTS_COUNT          = (PLAYER_SLOT_END - PLAYER_SLOT_START)
};

#define INVENTORY_SLOT_BAG_0    255

enum EquipmentSlots                                         // 19 slots
{
    EQUIPMENT_SLOT_START        = 0,
    EQUIPMENT_SLOT_HEAD         = 0,
    EQUIPMENT_SLOT_NECK         = 1,
    EQUIPMENT_SLOT_SHOULDERS    = 2,
    EQUIPMENT_SLOT_BODY         = 3,
    EQUIPMENT_SLOT_CHEST        = 4,
    EQUIPMENT_SLOT_WAIST        = 5,
    EQUIPMENT_SLOT_LEGS         = 6,
    EQUIPMENT_SLOT_FEET         = 7,
    EQUIPMENT_SLOT_WRISTS       = 8,
    EQUIPMENT_SLOT_HANDS        = 9,
    EQUIPMENT_SLOT_FINGER1      = 10,
    EQUIPMENT_SLOT_FINGER2      = 11,
    EQUIPMENT_SLOT_TRINKET1     = 12,
    EQUIPMENT_SLOT_TRINKET2     = 13,
    EQUIPMENT_SLOT_BACK         = 14,
    EQUIPMENT_SLOT_MAINHAND     = 15,
    EQUIPMENT_SLOT_OFFHAND      = 16,
    EQUIPMENT_SLOT_RANGED       = 17,
    EQUIPMENT_SLOT_TABARD       = 18,
    EQUIPMENT_SLOT_END          = 19
};

enum InventorySlots                                         // 4 slots
{
    INVENTORY_SLOT_BAG_START    = 19,
    INVENTORY_SLOT_BAG_END      = 23
};

enum InventoryPackSlots                                     // 16 slots
{
    INVENTORY_SLOT_ITEM_START   = 23,
    INVENTORY_SLOT_ITEM_END     = 39
};

enum BankItemSlots                                          // 28 slots
{
    BANK_SLOT_ITEM_START        = 39,
    BANK_SLOT_ITEM_END          = 67
};

enum BankBagSlots                                           // 7 slots
{
    BANK_SLOT_BAG_START         = 67,
    BANK_SLOT_BAG_END           = 74
};

enum BuyBackSlots                                           // 98 slots
{
    BUYBACK_SLOT_START          = 74,
    BUYBACK_SLOT_END            = 86
};

enum ReagentBankBagSlots
{
    REAGENT_BANK_SLOT_BAG_START = 86,
    REAGENT_BANK_SLOT_BAG_END = 184
};

enum EquipmentSetUpdateState
{
    EQUIPMENT_SET_UNCHANGED = 0,
    EQUIPMENT_SET_CHANGED   = 1,
    EQUIPMENT_SET_NEW       = 2,
    EQUIPMENT_SET_DELETED   = 3
};

struct EquipmentSet
{
    EquipmentSet() : Guid(0), IgnoreMask(0), state(EQUIPMENT_SET_NEW)
    {
        for (uint8 i = 0; i < EQUIPMENT_SLOT_END; ++i)
            Items[i] = 0;
    }

    uint64 Guid;
    std::string Name;
    std::string IconName;
    uint32 IgnoreMask;
    uint32 Items[EQUIPMENT_SLOT_END];
    EquipmentSetUpdateState state;
};

#define MAX_EQUIPMENT_SET_INDEX 10                          // client limit

typedef std::map<uint32, EquipmentSet> EquipmentSets;

struct ItemPosCount
{
    ItemPosCount(uint16 _pos, uint32 _count) : pos(_pos), count(_count) {}
    bool isContainedIn(std::vector<ItemPosCount> const& vec) const;
    uint16 pos;
    uint32 count;
};
typedef std::vector<ItemPosCount> ItemPosCountVec;

enum TradeSlots
{
    TRADE_SLOT_TRADED_COUNT     = 6,
    TRADE_SLOT_COUNT            = 7,
    TRADE_SLOT_NONTRADED        = 10,
    TRADE_SLOT_INVALID          = -1
};

enum TransferAbortReason
{
    TRANSFER_ABORT_NONE                          = 0,
    TRANSFER_ABORT_TOO_MANY_REALM_INSTANCES      = 1,   // Additional instances cannot be launched, please try again later.
    TRANSFER_ABORT_DIFFICULTY                    = 3,   // <Normal, Heroic, Epic> difficulty mode is not available for %s.
    TRANSFER_ABORT_INSUF_EXPAN_LVL               = 8,   // You must have <TBC, WotLK> expansion installed to access this area.
    TRANSFER_ABORT_NOT_FOUND                     = 10,  // Transfer Aborted: instance not found
    TRANSFER_ABORT_TOO_MANY_INSTANCES            = 11,  // You have entered too many instances recently.
    TRANSFER_ABORT_MAX_PLAYERS                   = 12,  // Transfer Aborted: instance is full
    TRANSFER_ABORT_XREALM_ZONE_DOWN              = 14,  // Transfer Aborted: cross-realm zone is down
    TRANSFER_ABORT_NOT_FOUND_2                   = 15,  // Transfer Aborted: instance not found
    TRANSFER_ABORT_DIFFICULTY_NOT_FOUND          = 16,  // client writes to console "Unable to resolve requested difficultyID %u to actual difficulty for map %d"
    TRANSFER_ABORT_NOT_FOUND_3                   = 17,  // Transfer Aborted: instance not found
    TRANSFER_ABORT_NOT_FOUND_4                   = 18,  // Transfer Aborted: instance not found
    TRANSFER_ABORT_ZONE_IN_COMBAT                = 19,  // Unable to zone in while an encounter is in progress.
    TRANSFER_ABORT_ALREADY_COMPLETED_ENCOUNTER   = 20,  // You are ineligible to participate in at least one encounter in this instance because you are already locked to an instance in which it has been defeated.
    TRANSFER_ABORT_LOCKED_TO_DIFFERENT_INSTANCE  = 24,  // You are already locked to %s
    TRANSFER_ABORT_REALM_ONLY                    = 25,  // All players in the party must be from the same realm to enter %s.
    TRANSFER_ABORT_MAP_NOT_ALLOWED               = 27,  // Map cannot be entered at this time.
    TRANSFER_ABORT_SOLO_PLAYER_SWITCH_DIFFICULTY = 28,  // This instance is already in progress. You may only switch difficulties from inside the instance.
    TRANSFER_ABORT_NEED_GROUP                    = 29,  // Transfer Aborted: you must be in a raid group to enter this instance
    TRANSFER_ABORT_UNIQUE_MESSAGE                = 30,  // Until you've escaped TLK's grasp, you cannot leave this place!
    TRANSFER_ABORT_ERROR                         = 31
    /*
    // Unknown values - not used by the client to display any error
    TRANSFER_ABORT_MANY_REALM_INSTANCES
    TRANSFER_ABORT_AREA_NOT_ZONED
    TRANSFER_ABORT_TIMEOUT
    TRANSFER_ABORT_SHUTTING_DOWN
    TRANSFER_ABORT_PLAYER_CONDITION
    TRANSFER_ABORT_BUSY
    TRANSFER_ABORT_DISCONNECTED
    TRANSFER_ABORT_LOGGING_OUT
    TRANSFER_ABORT_NEED_SERVER
    */
};

enum InstanceResetWarningType
{
    RAID_INSTANCE_WARNING_HOURS     = 1,                    // WARNING! %s is scheduled to reset in %d hour(s).
    RAID_INSTANCE_WARNING_MIN       = 2,                    // WARNING! %s is scheduled to reset in %d minute(s)!
    RAID_INSTANCE_WARNING_MIN_SOON  = 3,                    // WARNING! %s is scheduled to reset in %d minute(s). Please exit the zone or you will be returned to your bind location!
    RAID_INSTANCE_WELCOME           = 4,                    // Welcome to %s. This raid instance is scheduled to reset in %s.
    RAID_INSTANCE_EXPIRED           = 5
};

// PLAYER_FIELD_ARENA_TEAM_INFO_1_1 offsets
enum ArenaTeamInfoType
{
    ARENA_TEAM_ID                = 0,
    ARENA_TEAM_TYPE              = 1,                       // new in 3.2 - team type?
    ARENA_TEAM_MEMBER            = 2,                       // 0 - captain, 1 - member
    ARENA_TEAM_GAMES_WEEK        = 3,
    ARENA_TEAM_GAMES_SEASON      = 4,
    ARENA_TEAM_WINS_SEASON       = 5,
    ARENA_TEAM_PERSONAL_RATING   = 6,
    ARENA_TEAM_END               = 7
};

class InstanceSave;

enum RestType
{
    REST_TYPE_NO        = 0,
    REST_TYPE_IN_TAVERN = 1,
    REST_TYPE_IN_CITY   = 2
};

enum DuelCompleteType
{
    DUEL_INTERRUPTED = 0,
    DUEL_WON         = 1,
    DUEL_FLED        = 2
};

enum TeleportToOptions
{
    TELE_TO_GM_MODE             = 0x01,
    TELE_TO_NOT_LEAVE_TRANSPORT = 0x02,
    TELE_TO_NOT_LEAVE_COMBAT    = 0x04,
    TELE_TO_NOT_UNSUMMON_PET    = 0x08,
    TELE_TO_SPELL               = 0x10,
};

/// Type of environmental damages
enum EnviromentalDamage
{
    DAMAGE_EXHAUSTED = 0,
    DAMAGE_DROWNING  = 1,
    DAMAGE_FALL      = 2,
    DAMAGE_LAVA      = 3,
    DAMAGE_SLIME     = 4,
    DAMAGE_FIRE      = 5,
    DAMAGE_FALL_TO_VOID = 6                                 // custom case for fall without durability loss
};

enum PlayerChatTag
{
    CHAT_TAG_NONE       = 0x00,
    CHAT_TAG_AFK        = 0x01,
    CHAT_TAG_DND        = 0x02,
    CHAT_TAG_GM         = 0x04,
    CHAT_TAG_COM        = 0x08,                             // Commentator
    CHAT_TAG_DEV        = 0x10,
};

enum PlayedTimeIndex
{
    PLAYED_TIME_TOTAL = 0,
    PLAYED_TIME_LEVEL = 1
};

#define MAX_PLAYED_TIME_INDEX 2

// used at player loading query list preparing, and later result selection
enum PlayerLoginQueryIndex
{
    PLAYER_LOGIN_QUERY_LOADFROM                     = 0,
    PLAYER_LOGIN_QUERY_LOADGROUP                    = 1,
    PLAYER_LOGIN_QUERY_LOADBOUNDINSTANCES           = 2,
    PLAYER_LOGIN_QUERY_LOADAURAS                    = 3,
    PLAYER_LOGIN_QUERY_LOADAURAS_EFFECTS            = 4,
    PLAYER_LOGIN_QUERY_LOGIN_LOADSPELLS             = 5,
    PLAYER_LOGIN_QUERY_CHAR_LOADSPELLS              = 6,
    PLAYER_LOGIN_QUERY_LOADQUESTSTATUS              = 7,
    PLAYER_LOGIN_QUERY_LOADDAILYQUESTSTATUS         = 8,
    PLAYER_LOGIN_QUERY_LOADREPUTATION               = 9,
    PLAYER_LOGIN_QUERY_LOADINVENTORY                = 10,
    PLAYER_LOGIN_QUERY_LOADACTIONS                  = 11,
    PLAYER_LOGIN_QUERY_LOADMAILCOUNT                = 12,
    PLAYER_LOGIN_QUERY_LOADMAILDATE                 = 13,
    PLAYER_LOGIN_QUERY_LOADSOCIALLIST               = 14,
    PLAYER_LOGIN_QUERY_LOADHOMEBIND                 = 15,
    PLAYER_LOGIN_QUERY_LOADSPELLCOOLDOWNS           = 16,
    PLAYER_LOGIN_QUERY_LOADDECLINEDNAMES            = 17,
    PLAYER_LOGIN_QUERY_LOADGUILD                    = 18,
    PLAYER_LOGIN_QUERY_LOADACHIEVEMENTS             = 19,
    PLAYER_LOGIN_QUERY_LOADACCOUNTACHIEVEMENTS      = 20,
    PLAYER_LOGIN_QUERY_LOADCRITERIAPROGRESS         = 21,
    PLAYER_LOGIN_QUERY_LOADACCOUNTCRITERIAPROGRESS  = 22,
    PLAYER_LOGIN_QUERY_LOADEQUIPMENTSETS            = 23,
    PLAYER_LOGIN_QUERY_LOADARENADATA                = 24,
    PLAYER_LOGIN_QUERY_LOADBGDATA                   = 25,
    PLAYER_LOGIN_QUERY_LOADGLYPHS                   = 26,
    PLAYER_LOGIN_QUERY_LOADTALENTS                  = 27,
    PLAYER_LOGIN_QUERY_LOADACCOUNTDATA              = 28,
    PLAYER_LOGIN_QUERY_LOADSKILLS                   = 29,
    PLAYER_LOGIN_QUERY_LOADWEEKLYQUESTSTATUS        = 30,
    PLAYER_LOGIN_QUERY_LOADRANDOMBG                 = 31,
    PLAYER_LOGIN_QUERY_LOADBANNED                   = 32,
    PLAYER_LOGIN_QUERY_LOADQUESTSTATUSREW           = 33,
    PLAYER_LOGIN_QUERY_LOADINSTANCELOCKTIMES        = 34,
    PLAYER_LOGIN_QUERY_LOADSEASONALQUESTSTATUS      = 35,
    PLAYER_LOGIN_QUERY_LOAD_MONTHLY_QUEST_STATUS    = 36,
    PLAYER_LOGIN_QUERY_LOADVOIDSTORAGE              = 37,
    PLAYER_LOGIN_QUERY_LOADCURRENCY                 = 38,
    PLAYER_LOGIN_QUERY_LOAD_CUF_PROFILES            = 39,
    PLAYER_LOGIN_QUERY_LOAD_ARCHAEOLOGY             = 40,
    PLAYER_LOGIN_QUERY_LOAD_ARCHAEOLOGY_PROJECTS    = 41,
    PLAYER_LOGIN_QUERY_LOAD_ARCHAEOLOGY_SITES       = 42,
    PLAYER_LOGIN_QUERY_LOAD_ACCOUNT_TOYS            = 43,
    PLAYER_LOGIN_QUERY_LOAD_QUEST_OBJECTIVE_STATUS  = 44,
    PLAYER_LOGIN_QUERY_LOAD_CHARGES_COOLDOWNS       = 45,
    PLAYER_LOGIN_QUERY_LOAD_COMPLETED_CHALLENGES    = 46,
    PLAYER_LOGIN_QUERY_GARRISON                     = 47,
    PLAYER_LOGIN_QUERY_GARRISON_MISSIONS            = 48,
    PLAYER_LOGIN_QUERY_GARRISON_FOLLOWERS           = 49,
    PLAYER_LOGIN_QUERY_GARRISON_BUILDINGS           = 50,
    PLAYER_LOGIN_QUERY_DAILY_LOOT_COOLDOWNS         = 51,
    MAX_PLAYER_LOGIN_QUERY
};

class PetLoginQueryHolder : public SQLQueryHolder
{
    private:
        uint32 m_guid;
        PreparedQueryResult m_petResult;
    public:
        PetLoginQueryHolder(uint32 guid, PreparedQueryResult result)
            : m_guid(guid), m_petResult(result) { }
        uint32 GetGuid() const { return m_guid; }
        PreparedQueryResult GetPetResult() const { return m_petResult; }
        bool Initialize();
};

enum PetLoginQueryIndex
{
    PET_LOGIN_QUERY_LOADAURA                        = 0,
    PET_LOGIN_QUERY_LOADAURAEFFECT                  = 1,
    PET_LOGIN_QUERY_LOADSPELL                       = 2,
    PET_LOGIN_QUERY_LOADSPELLCOOLDOWN               = 3,
    MAX_PET_LOGIN_QUERY                             = 4
};

enum PlayerDelayedOperations
{
    DELAYED_SAVE_PLAYER         = 0x01,
    DELAYED_RESURRECT_PLAYER    = 0x02,
    DELAYED_SPELL_CAST_DESERTER = 0x04,
    DELAYED_BG_MOUNT_RESTORE    = 0x08,                     ///< Flag to restore mount state after teleport from BG
    DELAYED_BG_TAXI_RESTORE     = 0x10,                     ///< Flag to restore taxi state after teleport from BG
    DELAYED_BG_GROUP_RESTORE    = 0x20,                     ///< Flag to restore group state after teleport from BG
    DELAYED_END
};

// Player summoning auto-decline time (in secs)
#define MAX_PLAYER_SUMMON_DELAY                   (2*MINUTE)
#define MAX_MONEY_AMOUNT               (UI64LIT(9999999999)) // @TODO: Move this restriction to worldserver.conf, default to this value, hardcap at uint64.max

struct InstancePlayerBind
{
    InstanceSave* save;
    bool perm;
    /* permanent PlayerInstanceBinds are created in Raid/Heroic instances for players
       that aren't already permanently bound when they are inside when a boss is killed
       or when they enter an instance that the group leader is permanently bound to. */
    InstancePlayerBind() : save(NULL), perm(false) {}
};

enum DungeonStatusFlag
{
    DUNGEON_STATUSFLAG_NORMAL = 0x01,
    DUNGEON_STATUSFLAG_HEROIC = 0x02,

    RAID_STATUSFLAG_10MAN_NORMAL = 0x01,
    RAID_STATUSFLAG_25MAN_NORMAL = 0x02,
    RAID_STATUSFLAG_10MAN_HEROIC = 0x04,
    RAID_STATUSFLAG_25MAN_HEROIC = 0x08
};

struct AccessRequirement
{
    uint8  levelMin;
    uint8  levelMax;
    uint32 item;
    uint32 item2;
    uint32 quest_A;
    uint32 quest_H;
    uint32 achievement;
    uint32 leader_achievement;
    uint32 itemlevelMin;
    uint32 itemlevelMax;
    std::string questFailedText;
};

enum CharDeleteMethod
{
    CHAR_DELETE_REMOVE = 0,                      // Completely remove from the database
    CHAR_DELETE_UNLINK = 1                       // The character gets unlinked from the account,
                                                 // the name gets freed up and appears as deleted ingame
};

enum ReferAFriendError
{
    ERR_REFER_A_FRIEND_NONE                          = 0x00,
    ERR_REFER_A_FRIEND_NOT_REFERRED_BY               = 0x01,
    ERR_REFER_A_FRIEND_TARGET_TOO_HIGH               = 0x02,
    ERR_REFER_A_FRIEND_INSUFFICIENT_GRANTABLE_LEVELS = 0x03,
    ERR_REFER_A_FRIEND_TOO_FAR                       = 0x04,
    ERR_REFER_A_FRIEND_DIFFERENT_FACTION             = 0x05,
    ERR_REFER_A_FRIEND_NOT_NOW                       = 0x06,
    ERR_REFER_A_FRIEND_GRANT_LEVEL_MAX_I             = 0x07,
    ERR_REFER_A_FRIEND_NO_TARGET                     = 0x08,
    ERR_REFER_A_FRIEND_NOT_IN_GROUP                  = 0x09,
    ERR_REFER_A_FRIEND_SUMMON_LEVEL_MAX_I            = 0x0A,
    ERR_REFER_A_FRIEND_SUMMON_COOLDOWN               = 0x0B,
    ERR_REFER_A_FRIEND_INSUF_EXPAN_LVL               = 0x0C,
    ERR_REFER_A_FRIEND_SUMMON_OFFLINE_S              = 0x0D
};

enum PlayerRestState
{
    REST_STATE_RESTED                                = 0x01,
    REST_STATE_NOT_RAF_LINKED                        = 0x02,
    REST_STATE_RAF_LINKED                            = 0x06
};

enum PlayerCommandStates
{
    CHEAT_NONE          = 0x00,
    CHEAT_GOD           = 0x01,
    CHEAT_CASTTIME      = 0x02,
    CHEAT_COOLDOWN      = 0x04,
    CHEAT_POWER         = 0x08,
    CHEAT_WATERWALK     = 0x10,
    CHEAT_ALL_SPELLS    = 0x20,
};

enum AttackSwingError
{
    ATTACKSWINGERR_CANT_ATTACK  = 0,
    ATTACKSWINGERR_BAD_FACING   = 1,
    ATTACKSWINGERR_NOT_IN_RANGE = 2,
    ATTACKSWINGERR_DEAD_TARGET  = 3,
};

struct auraEffectData
{
    auraEffectData(uint8 slot, uint8 effect, uint32 amount, uint32 baseamount)
        : _slot(slot), _effect(effect), _amount(amount), _baseamount(baseamount)
    {
    }

    uint8 _slot;
    uint8 _effect;
    uint32 _amount;
    uint32 _baseamount;
};

class PlayerTaxi
{
    public:
        PlayerTaxi();
        ~PlayerTaxi() {}
        // Nodes
        void InitTaxiNodesForLevel(uint32 race, uint32 chrClass, uint8 level);
        void LoadTaxiMask(std::string const& data);

        bool IsTaximaskNodeKnown(uint32 nodeidx) const
        {
            uint8  field   = uint8((nodeidx - 1) / 8);
            uint32 submask = 1 << ((nodeidx-1) % 8);
            return (m_taximask[field] & submask) == submask;
        }
        bool SetTaximaskNode(uint32 nodeidx)
        {
            uint8  field   = uint8((nodeidx - 1) / 8);
            uint32 submask = 1 << ((nodeidx-1) % 8);
            if ((m_taximask[field] & submask) != submask)
            {
                m_taximask[field] |= submask;
                return true;
            }
            else
                return false;
        }
        void AppendTaximaskTo(ByteBuffer& data, bool all);

        // Destinations
        bool LoadTaxiDestinationsFromString(const std::string& values, uint32 team);
        std::string SaveTaxiDestinationsToString();

        void ClearTaxiDestinations() { m_TaxiDestinations.clear(); }
        void AddTaxiDestination(uint32 dest) { m_TaxiDestinations.push_back(dest); }
        uint32 GetTaxiSource() const { return m_TaxiDestinations.empty() ? 0 : m_TaxiDestinations.front(); }
        uint32 GetTaxiDestination() const { return m_TaxiDestinations.size() < 2 ? 0 : m_TaxiDestinations[1]; }
        uint32 GetCurrentTaxiPath() const;
        uint32 NextTaxiDestination()
        {
            m_TaxiDestinations.pop_front();
            return GetTaxiDestination();
        }
        bool empty() const { return m_TaxiDestinations.empty(); }

        friend std::ostringstream& operator<< (std::ostringstream& ss, PlayerTaxi const& taxi);
    private:
        TaxiMask m_taximask;
        std::deque<uint32> m_TaxiDestinations;
};

std::ostringstream& operator<< (std::ostringstream& ss, PlayerTaxi const& taxi);

class Player;

/// Holder for Battleground data
struct BGData
{
    BGData() :
        bgInstanceID(0),
        bgTypeID(BATTLEGROUND_TYPE_NONE),
        bgAfkReportedCount(0),
        bgAfkReportedTimer(0),
        bgTeam(0),
        mountSpell(0),
        m_LastActiveSpec(0)
    {
        bgQueuesJoinedTime.clear();
        ClearTaxiPath();
    }

    uint32 bgInstanceID;                    ///< This variable is set to bg->m_InstanceID,
                                            ///  when player is teleported to BG - (it is battleground's GUID)
    BattlegroundTypeId bgTypeID;

    std::map<MS::Battlegrounds::BattlegroundType::Type, uint32> bgQueuesJoinedTime;

    std::set<uint32>   bgAfkReporter;
    uint8              bgAfkReportedCount;
    time_t             bgAfkReportedTimer;

    uint32 bgTeam;                          ///< What side the player will be added to

    uint32 mountSpell;
    uint32 taxiPath[2];

    WorldLocation joinPos;                  ///< From where player entered BG

    uint8 m_LastActiveSpec;

    void ClearTaxiPath()     { taxiPath[0] = taxiPath[1] = 0; }
    bool HasTaxiPath() const { return taxiPath[0] && taxiPath[1]; }
};

struct VoidStorageItem
{
    VoidStorageItem()
    {
        ItemId = 0;
        ItemEntry = 0;
        CreatorGuid = 0;
        ItemRandomPropertyId = 0;
        ItemSuffixFactor = 0;
    }

    VoidStorageItem(uint64 id, uint32 entry, uint32 creator, uint32 randomPropertyId, uint32 suffixFactor)
    {
        ItemId = id;
        ItemEntry = entry;
        CreatorGuid = creator;
        ItemRandomPropertyId = randomPropertyId;
        ItemSuffixFactor = suffixFactor;
    }

    uint64 ItemId;
    uint32 ItemEntry;
    uint32 CreatorGuid;
    uint32 ItemRandomPropertyId;
    uint32 ItemSuffixFactor;
};

class TradeData
{
    public:                                                 // constructors
        TradeData(Player* player, Player* trader) :
            m_player(player),  m_trader(trader), m_accepted(false), m_acceptProccess(false),
            m_money(0), m_spell(0), m_spellCastItem(0)
        {
            memset(m_items, 0, TRADE_SLOT_COUNT * sizeof(uint64));
        }

        Player* GetTrader() const { return m_trader; }
        TradeData* GetTraderData() const;

        Item* GetItem(TradeSlots slot) const;
        bool HasItem(uint64 itemGuid) const;
        TradeSlots GetTradeSlotForItem(uint64 itemGuid) const;
        void SetItem(TradeSlots slot, Item* item);

        uint32 GetSpell() const { return m_spell; }
        void SetSpell(uint32 spell_id, Item* castItem = NULL);

        Item*  GetSpellCastItem() const;
        bool HasSpellCastItem() const { return m_spellCastItem != 0; }

        uint64 GetMoney() const { return m_money; }
        void SetMoney(uint64 money);

        bool IsAccepted() const { return m_accepted; }
        void SetAccepted(bool state, bool crosssend = false);

        bool IsInAcceptProcess() const { return m_acceptProccess; }
        void SetInAcceptProcess(bool state) { m_acceptProccess = state; }

    private:                                                // internal functions

        void Update(bool for_trader = true);

    private:                                                // fields

        Player*    m_player;                                // Player who own of this TradeData
        Player*    m_trader;                                // Player who trade with m_player

        bool       m_accepted;                              // m_player press accept for trade list
        bool       m_acceptProccess;                        // one from player/trader press accept and this processed

        uint64     m_money;                                 // m_player place money to trade

        uint32     m_spell;                                 // m_player apply spell to non-traded slot item
        uint64     m_spellCastItem;                         // applied spell casted by item use

        uint64     m_items[TRADE_SLOT_COUNT];               // traded items from m_player side including non-traded slot
};

struct ResurrectionData
{
    uint64 GUID;
    WorldLocation Location;
    uint32 Health;
    uint32 Mana;
    uint32 Aura;
};

class KillRewarder
{
public:
    KillRewarder(Player* killer, Unit* victim, bool isBattleGround);

    void Reward();

private:
    void _InitXP(Player* player);
    void _InitGroupData();

    void _RewardHonor(Player* player);
    void _RewardXP(Player* player, float rate);
    void _RewardReputation(Player* player, float rate);
    void _RewardKillCredit(Player* player);
    void _RewardPlayer(Player* player, bool isDungeon);
    void _RewardGroup();

    Player* _killer;
    Unit* _victim;
    Group* _group;
    float _groupRate;
    Player* _maxNotGrayMember;
    uint32 _count;
    uint32 _sumLevel;
    uint32 _xp;
    bool _isFullXP;
    uint8 _maxLevel;
    bool _isBattleGround;
    bool _isPvP;
};

struct PlayerTalentInfo
{
    PlayerTalentInfo() :
        FreeTalentPoints(0), UsedTalentCount(0), QuestRewardedTalentCount(0),
        ResetTalentsCost(0), ResetTalentsTime(0), ResetSpecializationCost(0),
        ResetSpecializationTime(0), ActiveSpec(0), SpecsCount(1)
    {
        for (uint8 i = 0; i < MAX_TALENT_SPECS; ++i)
        {
            SpecInfo[i].Talents = new PlayerTalentMap();
            memset(SpecInfo[i].Glyphs, 0, MAX_GLYPH_SLOT_INDEX * sizeof(uint32));
            SpecInfo[i].TalentTree = 0;
            SpecInfo[i].SpecializationId = 0;
        }
    }

    ~PlayerTalentInfo()
    {
        for (uint8 i = 0; i < MAX_TALENT_SPECS; ++i)
        {
            for (PlayerTalentMap::const_iterator itr = SpecInfo[i].Talents->begin(); itr != SpecInfo[i].Talents->end(); ++itr)
                delete itr->second;
            delete SpecInfo[i].Talents;
        }
    }

    struct TalentSpecInfo
    {
        PlayerTalentMap* Talents;
        uint32 Glyphs[MAX_GLYPH_SLOT_INDEX];
        uint32 TalentTree;
        uint32 SpecializationId;
    } SpecInfo[MAX_TALENT_SPECS];

    uint32 FreeTalentPoints;
    uint32 UsedTalentCount;
    uint32 QuestRewardedTalentCount;
    uint32 ResetTalentsCost;
    time_t ResetTalentsTime;
    uint32 ResetSpecializationCost;
    time_t ResetSpecializationTime;
    uint8 ActiveSpec;
    uint8 SpecsCount;

private:
    PlayerTalentInfo(PlayerTalentInfo const&);
};

struct PlayerToy
{
    PlayerToy()
    {
        memset(this, 0, sizeof (PlayerToy));
    }

    PlayerToy(uint32 p_Item, bool p_Favorite)
    {
        m_ItemID = p_Item;
        m_IsFavorite = p_Favorite;
    }

    uint32 m_ItemID;
    bool m_IsFavorite;
};

typedef std::map<uint32, PlayerToy> PlayerToys;

struct ChargesData
{
    ChargesData()
    {
        m_MaxCharges = 0;
        m_ConsumedCharges = 0;
        m_Changed = false;
    }

    ChargesData(uint32 p_MaxCharges, uint64 p_Cooldown, uint32 p_Charges = 1)
    {
        m_MaxCharges = p_MaxCharges;

        // Called in ConsumeCharge, so one charge has gone
        m_ConsumedCharges = p_Charges;

        m_ChargesCooldown.push_back(p_Cooldown);
        m_Changed = true;
    }

    std::vector<uint64> GetChargesCooldown() const { return m_ChargesCooldown; }
    void DecreaseCooldown(uint8 p_Charge, uint32 p_Time)
    {
        if (p_Charge >= m_MaxCharges)
            return;

        m_ChargesCooldown[p_Charge] -= p_Time;
    }

    uint32 m_MaxCharges;
    uint32 m_ConsumedCharges;
    std::vector<uint64> m_ChargesCooldown;
    bool m_Changed;
};

///<            SpellID
typedef std::map<uint32, ChargesData> SpellChargesMap;

struct CompletedChallenge
{
    CompletedChallenge()
    {
        m_BestTime = 0;
        m_LastTime = 0;
        m_BestMedal = 0;
        m_BestMedalDate = 0;
    }

    uint32 m_BestTime;
    uint32 m_LastTime;
    uint8 m_BestMedal; ///< 0 - None, 1 - Bronze, 2 - Silver, 3 - Gold
    uint32 m_BestMedalDate;
};

///<             MapID
typedef std::map<uint32, CompletedChallenge> CompletedChallengesMap;

enum BattlegroundTimerTypes
{
    PVP_TIMER,
    CHALLENGE_TIMER
};

namespace MS { namespace Garrison 
{
    class Manager;
}   ///< namespace Garrison
}   ///< namespace MS

class Player : public Unit, public GridObject<Player>
{
    friend class WorldSession;
    friend void Item::AddToUpdateQueueOf(Player* player);
    friend void Item::RemoveFromUpdateQueueOf(Player* player);
    public:
        explicit Player (WorldSession* session);
        ~Player();

        //AnticheatData anticheatData;

        void CleanupsBeforeDelete(bool finalCleanup = true);

        void AddToWorld();
        void RemoveFromWorld();

        bool TeleportTo(uint32 mapid, float x, float y, float z, float orientation, uint32 options = 0);
        bool TeleportTo(WorldLocation const &loc, uint32 options = 0)
        {
            return TeleportTo(loc.GetMapId(), loc.GetPositionX(), loc.GetPositionY(), loc.GetPositionZ(), loc.GetOrientation(), options);
        }
        bool TeleportToBGEntryPoint();
        void SwitchToPhasedMap(uint32 p_MapID);

        void SetSummonPoint(uint32 mapid, float x, float y, float z)
        {
            m_summon_expire = time(NULL) + MAX_PLAYER_SUMMON_DELAY;
            m_summon_mapid = mapid;
            m_summon_x = x;
            m_summon_y = y;
            m_summon_z = z;
        }
        void SummonIfPossible(bool agree);

        bool Create(uint32 guidlow, CharacterCreateInfo* createInfo);

        void Update(uint32 time);

        static bool BuildEnumData(PreparedQueryResult p_Result, ByteBuffer * p_Data);

        void SetInWater(bool apply);

        bool IsInWater() const { return m_isInWater; }
        bool IsUnderWater() const;
        bool IsFalling() { return GetPositionZ() < m_lastFallZ; }

        void SendInitialPacketsBeforeAddToMap();
        void SendInitialPacketsAfterAddToMap();
        void SendTransferAborted(uint32 mapid, TransferAbortReason reason, uint8 arg = 0);
        void SendRaidInstanceMessage(uint32 mapid, Difficulty difficulty, uint32 time);

        bool CanInteractWithQuestGiver(Object* questGiver);
        Creature* GetNPCIfCanInteractWith(uint64 guid, uint32 npcflagmask);
        Creature* GetNPCIfCanInteractWithFlag2(uint64 guid, uint32 npcflagmask);
        GameObject* GetGameObjectIfCanInteractWith(uint64 guid, GameobjectTypes type) const;

        bool ToggleAFK();
        bool ToggleDND();
        bool isAFK() const { return HasFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_AFK); }
        bool isDND() const { return HasFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_DND); }
        uint8 GetChatTag() const;
        std::string afkMsg;
        std::string dndMsg;

        MS::Garrison::Manager * GetGarrison();
        void CreateGarrison();
        bool IsInGarrison();
        void DeleteGarrison();

        uint32 GetBarberShopCost(uint8 newhairstyle, uint8 newhaircolor, uint8 newfacialhair, BarberShopStyleEntry const* newSkin=NULL);

        PlayerSocial *GetSocial() { return m_social; }

        PlayerTaxi m_taxi;
        void InitTaxiNodesForLevel() { m_taxi.InitTaxiNodesForLevel(getRace(), getClass(), getLevel()); }
        bool ActivateTaxiPathTo(std::vector<uint32> const& nodes, Creature* npc = NULL, uint32 spellid = 0, bool p_Triggered = false);
        bool ActivateTaxiPathTo(uint32 taxi_path_id, uint32 spellid = 0, bool p_Triggered = false);
        void CleanupAfterTaxiFlight();
        void ContinueTaxiFlight();
                                                            // mount_id can be used in scripting calls
        bool IsAcceptWhispers() const { return m_ExtraFlags & PLAYER_EXTRA_ACCEPT_WHISPERS; }
        void SetAcceptWhispers(bool on) { if (on) m_ExtraFlags |= PLAYER_EXTRA_ACCEPT_WHISPERS; else m_ExtraFlags &= ~PLAYER_EXTRA_ACCEPT_WHISPERS; }
        bool isGameMaster() const { return m_ExtraFlags & PLAYER_EXTRA_GM_ON; }
        void SetGameMaster(bool on);
        bool isGMChat() const { return m_ExtraFlags & PLAYER_EXTRA_GM_CHAT; }
        void SetGMChat(bool on) { if (on) m_ExtraFlags |= PLAYER_EXTRA_GM_CHAT; else m_ExtraFlags &= ~PLAYER_EXTRA_GM_CHAT; }
        bool isTaxiCheater() const { return m_ExtraFlags & PLAYER_EXTRA_TAXICHEAT; }
        void SetTaxiCheater(bool on) { if (on) m_ExtraFlags |= PLAYER_EXTRA_TAXICHEAT; else m_ExtraFlags &= ~PLAYER_EXTRA_TAXICHEAT; }
        bool isGMVisible() const { return !(m_ExtraFlags & PLAYER_EXTRA_GM_INVISIBLE); }
        void SetGMVisible(bool on);
        void SetPvPDeath(bool on) { if (on) m_ExtraFlags |= PLAYER_EXTRA_PVP_DEATH; else m_ExtraFlags &= ~PLAYER_EXTRA_PVP_DEATH; }

        void GiveXP(uint32 xp, Unit* victim, float group_rate=1.0f);
        void GiveGatheringXP();
        void GiveLevel(uint8 level);

        void InitStatsForLevel(bool reapplyMods = false);


        // .cheat command related
        bool GetCommandStatus(uint32 command) const { return _activeCheats & command; }
        void SetCommandStatusOn(uint32 command) { _activeCheats |= command; }
        void SetCommandStatusOff(uint32 command) { _activeCheats &= ~command; }

        // Played Time Stuff
        time_t m_logintime;
        time_t m_Last_tick;
        uint32 m_Played_time[MAX_PLAYED_TIME_INDEX];
        uint32 GetTotalPlayedTime() { return m_Played_time[PLAYED_TIME_TOTAL]; }
        uint32 GetLevelPlayedTime() { return m_Played_time[PLAYED_TIME_LEVEL]; }

        void setDeathState(DeathState s);                   // overwrite Unit::setDeathState

        void InnEnter (time_t time, uint32 mapid, float x, float y, float z)
        {
            inn_pos_mapid = mapid;
            inn_pos_x = x;
            inn_pos_y = y;
            inn_pos_z = z;
            time_inn_enter = time;
        }

        float GetRestBonus() const { return m_rest_bonus; }
        void SetRestBonus(float rest_bonus_new);

        RestType GetRestType() const { return rest_type; }
        void SetRestType(RestType n_r_type) { rest_type = n_r_type; }

        uint32 GetInnPosMapId() const { return inn_pos_mapid; }
        float GetInnPosX() const { return inn_pos_x; }
        float GetInnPosY() const { return inn_pos_y; }
        float GetInnPosZ() const { return inn_pos_z; }

        time_t GetTimeInnEnter() const { return time_inn_enter; }
        void UpdateInnerTime (time_t time) { time_inn_enter = time; }

        Pet* GetPet() const;
        Pet* SummonPet(uint32 entry, float x, float y, float z, float ang, PetType petType, uint32 despwtime, PetSlot slotID = PET_SLOT_UNK_SLOT, bool stampeded = false);
        void RemovePet(Pet* pet, PetSlot mode, bool returnreagent = false, bool stampeded = false);

        PhaseMgr& GetPhaseMgr() { return phaseMgr; }

        void Say(const std::string& text, const uint32 language);
        void Yell(const std::string& text, const uint32 language);
        void TextEmote(const std::string& text);
        void Whisper(const std::string& text, const uint32 language, uint64 receiver);
        void WhisperAddon(const std::string& text, const std::string& prefix, Player* receiver);
        void BuildPlayerChat(WorldPacket* data, uint8 msgtype, const std::string& text, uint32 language, const char* addonPrefix = NULL, const std::string& channel = "") const;

        ArchaeologyMgr& GetArchaeologyMgr() { return m_archaeologyMgr; }

        /*********************************************************/
        /***                    STORAGE SYSTEM                 ***/
        /*********************************************************/

        void SetVirtualItemSlot(uint8 i, Item* item);
        void SetSheath(SheathState sheathed);             // overwrite Unit version
        uint8 FindEquipSlot(ItemTemplate const* proto, uint32 slot, bool swap) const;
        uint8 GetGuessedEquipSlot(ItemTemplate const* proto) const;
        uint32 GetItemCount(uint32 item, bool inBankAlso = false, Item* skipItem = NULL) const;
        uint32 GetItemCountWithLimitCategory(uint32 limitCategory, Item* skipItem = NULL) const;
        Item* GetItemByGuid(uint64 guid) const;
        Item* GetItemByEntry(uint32 entry) const;
        Item* GetItemByPos(uint16 pos) const;
        Item* GetItemByPos(uint8 bag, uint8 slot) const;
        Bag*  GetBagByPos(uint8 slot) const;
        inline Item* GetUseableItemByPos(uint8 bag, uint8 slot) const //Does additional check for disarmed weapons
        {
            if (!CanUseAttackType(GetAttackBySlot(slot)))
                return NULL;
            return GetItemByPos(bag, slot);
        }
        Item* GetWeaponForAttack(WeaponAttackType attackType, bool useable = false) const;
        Item* GetShield(bool useable = false) const;
        static uint8 GetAttackBySlot(uint8 slot);        // MAX_ATTACK if not weapon slot
        std::vector<Item*> &GetItemUpdateQueue() { return m_itemUpdateQueue; }
        static bool IsInventoryPos(uint16 pos) { return IsInventoryPos(pos >> 8, pos & 255); }
        static bool IsInventoryPos(uint8 bag, uint8 slot);
        static bool IsEquipmentPos(uint16 pos) { return IsEquipmentPos(pos >> 8, pos & 255); }
        static bool IsEquipmentPos(uint8 bag, uint8 slot);
        static bool IsBagPos(uint16 pos);
        static bool IsBankPos(uint16 pos) { return IsBankPos(pos >> 8, pos & 255); }
        static bool IsBankPos(uint8 bag, uint8 slot);
        static bool IsReagentBankPos(uint16 pos) { return IsReagentBankPos(pos >> 8, pos & 255); }
        static bool IsReagentBankPos(uint8 bag, uint8 slot);
        bool IsValidPos(uint16 pos, bool explicit_pos) { return IsValidPos(pos >> 8, pos & 255, explicit_pos); }
        bool IsValidPos(uint8 bag, uint8 slot, bool explicit_pos);
        uint8 GetBankBagSlotCount() const { return GetByteValue(PLAYER_FIELD_REST_STATE, 2); }
        void SetBankBagSlotCount(uint8 count) { SetByteValue(PLAYER_FIELD_REST_STATE, 2, count); }
        bool HasItemCount(uint32 item, uint32 count = 1, bool inBankAlso = false) const;
        bool HasItemFitToSpellRequirements(SpellInfo const* spellInfo, Item const* ignoreItem = NULL) const;
        bool CanNoReagentCast(SpellInfo const* spellInfo) const;
        bool HasItemOrGemWithIdEquipped(uint32 item, uint32 count, uint8 except_slot = NULL_SLOT) const;
        bool HasItemOrGemWithLimitCategoryEquipped(uint32 limitCategory, uint32 count, uint8 except_slot = NULL_SLOT) const;
        bool IsItemSupplies(ItemTemplate const* p_BagProto) const;
        InventoryResult CanTakeMoreSimilarItems(Item* pItem) const { return CanTakeMoreSimilarItems(pItem->GetEntry(), pItem->GetCount(), pItem); }
        InventoryResult CanTakeMoreSimilarItems(uint32 entry, uint32 count) const { return CanTakeMoreSimilarItems(entry, count, NULL); }
        InventoryResult CanStoreNewItem(uint8 bag, uint8 slot, ItemPosCountVec& dest, uint32 item, uint32 count, uint32* no_space_count = NULL) const
        {
            return CanStoreItem(bag, slot, dest, item, count, NULL, false, no_space_count);
        }
        InventoryResult CanStoreItem(uint8 bag, uint8 slot, ItemPosCountVec& dest, Item* pItem, bool swap = false) const
        {
            if (!pItem)
                return EQUIP_ERR_ITEM_NOT_FOUND;
            uint32 count = pItem->GetCount();
            return CanStoreItem(bag, slot, dest, pItem->GetEntry(), count, pItem, swap, NULL);

        }
        InventoryResult CanStoreItems(Item** pItem, int count) const;
        InventoryResult CanEquipNewItem(uint8 slot, uint16& dest, uint32 item, bool swap) const;
        InventoryResult CanEquipItem(uint8 slot, uint16& dest, Item* pItem, bool swap, bool not_loading = true) const;

        InventoryResult CanEquipUniqueItem(Item* pItem, uint8 except_slot = NULL_SLOT, uint32 limit_count = 1) const;
        InventoryResult CanEquipUniqueItem(ItemTemplate const* itemProto, uint8 except_slot = NULL_SLOT, uint32 limit_count = 1) const;
        InventoryResult CanUnequipItems(uint32 item, uint32 count) const;
        InventoryResult CanUnequipItem(uint16 src, bool swap) const;
        InventoryResult CanBankItem(uint8 bag, uint8 slot, ItemPosCountVec& dest, Item* pItem, bool swap, bool not_loading = true) const;
        InventoryResult CanReagentBankItem(uint8 bag, uint8 slot, ItemPosCountVec& dest, Item* pItem, bool swap) const;
        InventoryResult CanUseItem(Item* pItem, bool not_loading = true) const;
        bool HasItemTotemCategory(uint32 TotemCategory) const;
        InventoryResult CanUseItem(ItemTemplate const* pItem) const;
        InventoryResult CanUseAmmo(uint32 item) const;
        InventoryResult CanRollForItemInLFG(ItemTemplate const* item, WorldObject const* lootedObject) const;
        Item* StoreNewItem(ItemPosCountVec const& pos, uint32 item, bool update, int32 randomPropertyId = 0);
        Item* StoreNewItem(ItemPosCountVec const& pos, uint32 item, bool update, int32 randomPropertyId, AllowedLooterSet &allowedLooters);
        Item* StoreItem(ItemPosCountVec const& pos, Item* pItem, bool update);
        Item* EquipNewItem(uint16 pos, uint32 item, bool update);
        Item* EquipItem(uint16 pos, Item* pItem, bool update);
        void AutoUnequipOffhandIfNeed(bool force = false);
        bool StoreNewItemInBestSlots(uint32 item_id, uint32 item_count);
        void AutoStoreLoot(uint8 bag, uint8 slot, uint32 loot_id, LootStore const& store, bool broadcast = false);
        void AutoStoreLoot(uint32 loot_id, LootStore const& store, bool broadcast = false) { AutoStoreLoot(NULL_BAG, NULL_SLOT, loot_id, store, broadcast); }
        void StoreLootItem(uint8 p_LootSlot, Loot* p_Loot, uint8 p_LinkedLootSlot = 255);
        void AddTrackingQuestIfNeeded(uint64 p_SourceGuid);

        /// Apply a function on every item found in the bags.
        /// @p_Function : A function that takes the owner of the items, the item to process, the slot bag of the item and the slot of the item.
        /// If p_Function returns false, then it will stop apply.
        void ApplyOnBagsItems(std::function<bool(Player*, Item*, uint8, uint8)>&& p_Function);

        /// Apply a function on every item found in the bank.
        /// @p_Function : A function that takes the owner of the items, the item to process, the slot bag of the item and the slot of the item.
        /// If p_Function returns false, then it will stop apply.
        void ApplyOnBankItems(std::function<bool(Player*, Item*, uint8, uint8)>&& p_Function);

        /// Apply a function on every item found in the reagent bank.
        /// @p_Function : A function that takes the owner of the items, the item to process, the slot bag of the item and the slot of the item.
        /// If p_Function returns false, then it will stop apply.
        void ApplyOnReagentBankItems(std::function<bool(Player*, Item*, uint8, uint8)>&& p_Function);

        InventoryResult CanTakeMoreSimilarItems(uint32 entry, uint32 count, Item* pItem, uint32* no_space_count = NULL) const;
        InventoryResult CanStoreItem(uint8 bag, uint8 slot, ItemPosCountVec& dest, uint32 entry, uint32 count, Item* pItem = NULL, bool swap = false, uint32* no_space_count = NULL) const;

        void AddRefundReference(uint32 it);
        void DeleteRefundReference(uint32 it);

        /// send initialization of new currency for client
        void SendNewCurrency(uint32 id);
        /// send full data about all currencies to client
        void ModifyCurrencyFlags(uint32 currencyId, uint8 flags);
        void SendCurrencies();
        void SendPvpRewards();
        /// return count of currency witch has plr
        uint32 GetCurrency(uint32 id, bool usePrecision) const;
        uint32 GetCurrencyOnWeek(uint32 id, bool usePrecision) const;
        uint32 GetCurrencyOnSeason(uint32 id, bool usePrecision) const;
        /// return presence related currency
        bool HasCurrency(uint32 id, uint32 count) const;
        /// @todo: not understand why it subtract from total count and for what it used. It should be remove and replaced by ModifyCurrency
        void SetCurrency(uint32 id, uint32 count, bool printLog = true);
        uint32 GetCurrencyWeekCap(uint32 id, bool usePrecision = false);
        void ResetCurrencyWeekCap();
        uint32 CalculateCurrencyWeekCap(uint32 id);
        uint32 GetCurrencyTotalCap(CurrencyTypesEntry const* currency) const;
        void UpdateConquestCurrencyCap(uint32 currency);

        bool HasUnlockedReagentBank();
        void UnlockReagentBank();
        uint32 GetFreeReagentBankSlot() const;

        /**
        * @name ModifyCurrency
        * @brief Change specific currency and send result to client

        * @param id currency entry from CurrencyTypes.dbc
        * @param count integer value for adding/removing current currency
        * @param printLog used on SMSG_UPDATE_CURRENCY
        * @param ignore gain multipliers
        */

        void ModifyCurrency(uint32 id, int32 count, bool printLog = true, bool ignoreMultipliers = false, bool ignoreLimit = false);

        void ApplyEquipCooldown(Item* pItem);
        void QuickEquipItem(uint16 pos, Item* pItem);
        void VisualizeItem(uint8 slot, Item* pItem);
        void SetVisibleItemSlot(uint8 slot, Item* pItem);
        Item* BankItem(ItemPosCountVec const& dest, Item* pItem, bool update)
        {
            return StoreItem(dest, pItem, update);
        }
        Item* BankItem(uint16 pos, Item* pItem, bool update);
        void RemoveItem(uint8 bag, uint8 slot, bool update);
        void MoveItemFromInventory(uint8 bag, uint8 slot, bool update);
                                                            // in trade, auction, guild bank, mail....
        void MoveItemToInventory(ItemPosCountVec const& dest, Item* pItem, bool update, bool in_characterInventoryDB = false);
                                                            // in trade, guild bank, mail....
        void RemoveItemDependentAurasAndCasts(Item* pItem);
        void DestroyItem(uint8 bag, uint8 slot, bool update);
        void DestroyItemCount(uint32 item, uint32 count, bool update, bool unequip_check = false);
        void DestroyItemCount(Item* item, uint32& count, bool update);
        void DestroyConjuredItems(bool update);
        void DestroyZoneLimitedItem(bool update, uint32 new_zone);
        void SplitItem(uint16 src, uint16 dst, uint32 count);
        void SwapItem(uint16 src, uint16 dst);
        void AddItemToBuyBackSlot(Item* pItem);
        Item* GetItemFromBuyBackSlot(uint32 slot);
        void RemoveItemFromBuyBackSlot(uint32 slot, bool del);
        void SendEquipError(InventoryResult msg, Item* pItem, Item* pItem2 = NULL, uint32 itemid = 0);
        void SendBuyError(BuyResult msg, Creature* creature, uint32 item, uint32 param);
        void SendSellError(SellResult msg, Creature* creature, uint64 guid);
        void AddWeaponProficiency(uint32 newflag) { m_WeaponProficiency |= newflag; }
        void AddArmorProficiency(uint32 newflag) { m_ArmorProficiency |= newflag; }
        uint32 GetWeaponProficiency() const { return m_WeaponProficiency; }
        uint32 GetArmorProficiency() const { return m_ArmorProficiency; }
        bool IsUseEquipedWeapon(bool mainhand) const
        {
            // disarm applied only to mainhand weapon
            return !IsInFeralForm() && (!mainhand || !HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISARMED));
        }
        bool IsTwoHandUsed() const
        {
            Item* mainItem = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);

            if (mainItem && mainItem->GetTemplate()->SubClass == ITEM_SUBCLASS_WEAPON_WAND)
                return false;

            return mainItem && ((mainItem->GetTemplate()->InventoryType == INVTYPE_2HWEAPON && !CanTitanGrip()) || mainItem->GetTemplate()->InventoryType == INVTYPE_RANGED || mainItem->GetTemplate()->InventoryType == INVTYPE_THROWN || mainItem->GetTemplate()->InventoryType == INVTYPE_RANGEDRIGHT);
        }
        void SendNewItem(Item* item, uint32 count, bool received, bool created, bool broadcast = false, std::vector<uint32> const& p_ItemBonus = {});
        bool BuyItemFromVendorSlot(uint64 vendorguid, uint32 vendorslot, uint32 item, uint8 count, uint8 bag, uint8 slot);
        bool BuyCurrencyFromVendorSlot(uint64 vendorGuid, uint32 vendorSlot, uint32 currency, uint32 count);
        bool _StoreOrEquipNewItem(uint32 vendorslot, uint32 item, uint8 count, uint8 bag, uint8 slot, int64 price, ItemTemplate const* pProto, Creature* pVendor, VendorItem const* crItem, bool bStore);

        float GetReputationPriceDiscount(Creature const* creature) const;

        Player* GetTrader() const { return m_trade ? m_trade->GetTrader() : NULL; }
        TradeData* GetTradeData() const { return m_trade; }
        void TradeCancel(bool sendback);
        uint32 GetClientStateIndex() const { return m_ClientStateIndex; }
        void IncreaseClientStateIndex() { ++m_ClientStateIndex; }
        void InitializeClientStateIndex() { m_ClientStateIndex = 1; }

        void UpdateEnchantTime(uint32 time);
        void UpdateSoulboundTradeItems();
        void AddTradeableItem(Item* item);
        void RemoveTradeableItem(Item* item);
        void UpdateItemDuration(uint32 time, bool realtimeonly = false);
        void AddEnchantmentDurations(Item* item);
        void RemoveEnchantmentDurations(Item* item);
        void RemoveArenaEnchantments(EnchantmentSlot slot);
        void AddEnchantmentDuration(Item* item, EnchantmentSlot slot, uint32 duration);
        void ApplyEnchantment(Item* item, EnchantmentSlot slot, bool apply, bool apply_dur = true, bool ignore_condition = false);
        void ApplyEnchantment(Item* item, bool apply);
        void UpdateSkillEnchantments(uint16 skill_id, uint16 curr_value, uint16 new_value);
        void SendEnchantmentDurations();
        void BuildEnchantmentsInfoData(WorldPacket* data);
        void AddItemDurations(Item* item);
        void RemoveItemDurations(Item* item);
        void SendItemDurations();
        void SendDisplayToast(uint32 p_Entry, uint32 p_Count, DisplayToastMethod p_Method, ToastTypes p_Type, bool p_BonusRoll, bool p_Mailed, std::vector<uint32> const& p_Bonus = {});
        void LoadCorpse();
        void LoadPet(PreparedQueryResult result);

        bool AddItem(uint32 itemId, uint32 count, uint32* noSpaceForCount = NULL);

        uint32 m_stableSlots;

        /*********************************************************/
        /***                    GOSSIP SYSTEM                  ***/
        /*********************************************************/

        void PrepareGossipMenu(WorldObject* source, uint32 menuId = 0, bool showQuests = false);
        void SendPreparedGossip(WorldObject* source);
        void OnGossipSelect(WorldObject* source, uint32 gossipListId, uint32 menuId);

        uint32 GetGossipTextId(uint32 menuId, WorldObject* source);
        uint32 GetGossipTextId(WorldObject* source);
        static uint32 GetDefaultGossipMenuForSource(WorldObject* source);

        /*********************************************************/
        /***                    QUEST SYSTEM                   ***/
        /*********************************************************/

        int32 GetQuestLevel(Quest const* quest) const { return quest && (quest->GetQuestLevel() > 0) ? quest->GetQuestLevel() : getLevel(); }

        void PrepareQuestMenu(uint64 guid);
        void SendPreparedQuest(uint64 guid);
        bool IsActiveQuest(uint32 quest_id) const;
        Quest const* GetNextQuest(uint64 guid, Quest const* quest);
        bool CanSeeStartQuest(Quest const* quest);
        bool CanTakeQuest(Quest const* quest, bool msg);
        bool CanAddQuest(Quest const* quest, bool msg);
        bool CanCompleteQuest(uint32 quest_id);
        bool CanCompleteRepeatableQuest(Quest const* quest);
        bool CanRewardQuest(Quest const* quest, bool msg);
        bool CanRewardQuest(Quest const* quest, uint32 reward, bool msg);
        void AddQuest(Quest const* quest, Object* questGiver);
        void CompleteQuest(uint32 quest_id);
        void IncompleteQuest(uint32 quest_id);
        void RewardQuest(Quest const* quest, uint32 reward, Object* questGiver, bool announce = true);
        void FailQuest(uint32 quest_id);
        bool SatisfyQuestSkill(Quest const* qInfo, bool msg) const;
        bool SatisfyQuestLevel(Quest const* qInfo, bool msg);
        bool SatisfyQuestLog(bool msg);
        bool SatisfyQuestPreviousQuest(Quest const* qInfo, bool msg);
        bool SatisfyQuestTeam(Quest const* qInfo);
        bool SatisfyQuestClass(Quest const* qInfo, bool msg) const;
        bool SatisfyQuestRace(Quest const* qInfo, bool msg);
        bool SatisfyQuestReputation(Quest const* qInfo, bool msg);
        bool SatisfyQuestStatus(Quest const* qInfo, bool msg);
        bool SatisfyQuestConditions(Quest const* qInfo, bool msg);
        bool SatisfyQuestTimed(Quest const* qInfo, bool msg);
        bool SatisfyQuestExclusiveGroup(Quest const* qInfo, bool msg);
        bool SatisfyQuestNextChain(Quest const* qInfo, bool msg);
        bool SatisfyQuestPrevChain(Quest const* qInfo, bool msg);
        bool SatisfyQuestDay(Quest const* qInfo);
        bool SatisfyQuestWeek(Quest const* qInfo, bool msg);
        bool SatisfyQuestMonth(Quest const* qInfo, bool msg);
        bool SatisfyQuestSeasonal(Quest const* qInfo, bool msg);
        bool GiveQuestSourceItem(Quest const* quest);
        bool TakeQuestSourceItem(uint32 questId, bool msg);
        bool GetQuestRewardStatus(uint32 quest_id) const;
        QuestStatus GetQuestStatus(uint32 quest_id) const;
        void SetQuestStatus(uint32 quest_id, QuestStatus status);
        void RemoveActiveQuest(uint32 quest_id);
        void RemoveRewardedQuest(uint32 quest_id);

        void SetDailyQuestStatus(uint32 quest_id);
        void SetWeeklyQuestStatus(uint32 quest_id);
        void SetMonthlyQuestStatus(uint32 quest_id);
        void SetSeasonalQuestStatus(uint32 quest_id);
        void ResetDailyQuestStatus();
        void ResetWeeklyQuestStatus();
        void ResetMonthlyQuestStatus();
        void ResetSeasonalQuestStatus(uint16 event_id);

        uint16 FindQuestSlot(uint32 quest_id) const;
        uint32 GetQuestSlotQuestId(uint16 slot) const { return GetUInt32Value(PLAYER_FIELD_QUEST_LOG + slot * MAX_QUEST_OFFSET + QUEST_ID_OFFSET); }
        uint32 GetQuestSlotState(uint16 slot)   const { return GetUInt32Value(PLAYER_FIELD_QUEST_LOG + slot * MAX_QUEST_OFFSET + QUEST_STATE_OFFSET); }
        uint16 GetQuestSlotCounter(uint16 slot, uint8 counter) const { return (uint16)(GetUInt64Value(PLAYER_FIELD_QUEST_LOG + slot * MAX_QUEST_OFFSET + QUEST_COUNTS_OFFSET) >> (counter * 16)); }
        uint32 GetQuestSlotTime(uint16 slot)    const { return GetUInt32Value(PLAYER_FIELD_QUEST_LOG + slot * MAX_QUEST_OFFSET + QUEST_TIME_OFFSET); }
        void SetQuestSlot(uint16 slot, uint32 quest_id, uint32 timer = 0)
        {
            SetUInt32Value(PLAYER_FIELD_QUEST_LOG + slot * MAX_QUEST_OFFSET + QUEST_ID_OFFSET, quest_id);
            SetUInt32Value(PLAYER_FIELD_QUEST_LOG + slot * MAX_QUEST_OFFSET + QUEST_STATE_OFFSET, 0);
            SetUInt32Value(PLAYER_FIELD_QUEST_LOG + slot * MAX_QUEST_OFFSET + QUEST_COUNTS_OFFSET, 0);
            SetUInt32Value(PLAYER_FIELD_QUEST_LOG + slot * MAX_QUEST_OFFSET + QUEST_TIME_OFFSET, timer);
        }
        void SetQuestSlotCounter(uint16 slot, uint8 counter, uint16 count)
        {
            uint64 val = GetUInt64Value(PLAYER_FIELD_QUEST_LOG + slot * MAX_QUEST_OFFSET + QUEST_COUNTS_OFFSET);
            val &= ~((uint64)0xFFFF << (counter * 16));
            val |= ((uint64)count << (counter * 16));
            SetUInt64Value(PLAYER_FIELD_QUEST_LOG + slot * MAX_QUEST_OFFSET + QUEST_COUNTS_OFFSET, val);
        }
        void SetQuestSlotState(uint16 slot, uint32 state) { SetFlag(PLAYER_FIELD_QUEST_LOG + slot * MAX_QUEST_OFFSET + QUEST_STATE_OFFSET, state); }
        void RemoveQuestSlotState(uint16 slot, uint32 state) { RemoveFlag(PLAYER_FIELD_QUEST_LOG + slot * MAX_QUEST_OFFSET + QUEST_STATE_OFFSET, state); }
        void SetQuestSlotTimer(uint16 slot, uint32 timer) { SetUInt32Value(PLAYER_FIELD_QUEST_LOG + slot * MAX_QUEST_OFFSET + QUEST_TIME_OFFSET, timer); }
        void SwapQuestSlot(uint16 slot1, uint16 slot2)
        {
            for (int i = 0; i < MAX_QUEST_OFFSET; ++i)
            {
                uint32 temp1 = GetUInt32Value(PLAYER_FIELD_QUEST_LOG + MAX_QUEST_OFFSET * slot1 + i);
                uint32 temp2 = GetUInt32Value(PLAYER_FIELD_QUEST_LOG + MAX_QUEST_OFFSET * slot2 + i);

                SetUInt32Value(PLAYER_FIELD_QUEST_LOG + MAX_QUEST_OFFSET * slot1 + i, temp2);
                SetUInt32Value(PLAYER_FIELD_QUEST_LOG + MAX_QUEST_OFFSET * slot2 + i, temp1);
            }
        }
        uint16 GetReqKillOrCastCurrentCount(uint32 quest_id, int32 entry);
        void AreaExploredOrEventHappens(uint32 questId);
        void GroupEventHappens(uint32 questId, WorldObject const* pEventObject);
        void ItemAddedQuestCheck(uint32 entry, uint32 count);
        void ItemRemovedQuestCheck(int32 entry, uint32 count);
        void KilledMonster(CreatureTemplate const* cInfo, uint64 guid);
        void KilledMonsterCredit(uint32 entry, uint64 guid = 0);
        void KilledPlayerCredit();
        void CastedCreatureOrGO(uint32 entry, uint64 guid, uint32 spell_id);
        void TalkedToCreature(uint32 entry, uint64 guid);
        void MoneyChanged(uint32 value);
        void ReputationChanged(FactionEntry const* factionEntry);
        void ReputationChanged2(FactionEntry const* factionEntry);
        void ReputationChangedQuestCheck(FactionEntry const* factionEntry);
        bool HasQuestForItem(uint32 itemid) const;
        bool HasQuestForGO(uint32 GOId) const;
        bool HasQuest(uint32 p_QuestID) const;
        void UpdateForQuestWorldObjects();
        bool CanShareQuest(uint32 quest_id) const;
        void QuestObjectiveSatisfy(uint32 objectId, uint32 amount, uint8 type = 0u, uint64 guid = 0u);

        void SendQuestComplete(Quest const* quest);
        void SendQuestReward(Quest const* quest, uint32 XP, Object* questGiver);
        void SendQuestFailed(uint32 questId, InventoryResult reason = EQUIP_ERR_OK);
        void SendQuestTimerFailed(uint32 quest_id);
        void SendCanTakeQuestResponse(uint32 msg) const;
        void SendQuestConfirmAccept(Quest const* quest, Player* pReceiver);
        void SendPushToPartyResponse(Player* player, uint32 msg);
        void SendQuestUpdateAddCredit(Quest const* p_Quest, const QuestObjective & p_Objective, uint64 p_ObjGUID, uint16 p_OldCount, uint16 p_AddCount);
        void SendQuestUpdateAddPlayer(Quest const* p_Quest, const QuestObjective & p_Objective, uint16 p_OldCount, uint16 p_AddCount);

        uint64 GetDivider() { return m_divider; }
        void SetDivider(uint64 guid) { m_divider = guid; }

        uint32 GetInGameTime() { return m_ingametime; }

        void SetInGameTime(uint32 time) { m_ingametime = time; }

        void AddTimedQuest(uint32 quest_id) { m_timedquests.insert(quest_id); }
        void RemoveTimedQuest(uint32 quest_id) { m_timedquests.erase(quest_id); }

        MS::Utilities::BitSet const& GetCompletedQuests() const { return m_CompletedQuestBits; }

        /*********************************************************/
        /***                   LOAD SYSTEM                     ***/
        /*********************************************************/

        bool LoadFromDB(uint32 guid, SQLQueryHolder *holder, PreparedQueryResult accountResult);
        bool isBeingLoaded() const { return GetSession()->PlayerLoading();}

        void Initialize(uint32 guid);
        static uint32 GetUInt32ValueFromArray(Tokenizer const& data, uint16 index);
        static float  GetFloatValueFromArray(Tokenizer const& data, uint16 index);
        static uint32 GetZoneIdFromDB(uint64 guid);
        static uint32 GetLevelFromDB(uint64 guid);
        static bool   LoadPositionFromDB(uint32& mapid, float& x, float& y, float& z, float& o, bool& in_flight, uint64 guid);

        static bool IsValidGender(uint8 Gender) { return Gender <= GENDER_FEMALE; }
        static bool IsValidClass(uint8 Class) { return (1 << (Class - 1)) & CLASSMASK_ALL_PLAYABLE; }
        static bool IsValidRace(uint8 Race) { return (1 << (Race - 1)) & RACEMASK_ALL_PLAYABLE; }

        /*********************************************************/
        /***                   SAVE SYSTEM                     ***/
        /*********************************************************/

        void SaveToDB(bool create = false);
        void SaveInventoryAndGoldToDB(SQLTransaction& trans);                    // fast save function for item/money cheating preventing
        void SaveGoldToDB(SQLTransaction& trans);

        static void SetUInt32ValueInArray(Tokenizer& data, uint16 index, uint32 value);
        static void SetFloatValueInArray(Tokenizer& data, uint16 index, float value);
        static void Customize(uint64 guid, uint8 gender, uint8 skin, uint8 face, uint8 hairStyle, uint8 hairColor, uint8 facialHair);
        static void SavePositionInDB(uint32 mapid, float x, float y, float z, float o, uint32 zone, uint64 guid);

        static void DeleteFromDB(uint64 playerguid, uint32 accountId, bool updateRealmChars = true, bool deleteFinally = false);
        static void DeleteOldCharacters();
        static void DeleteOldCharacters(uint32 keepDays);

        bool m_mailsLoaded;
        bool m_mailsUpdated;

        void SetBindPoint(uint64 guid);
        void SendTalentWipeConfirm(uint64 guid, bool specializaion);
        void CalcRage(uint32 damage, bool attacker);
        void CalculateMonkMeleeAttacks(float &p_Low, float &p_High);
        void RegenerateAll();
        void Regenerate(Powers power);
        void RegenerateHealth();
        void setRegenTimerCount(uint32 time) {m_regenTimerCount = time;}
        void setWeaponChangeTimer(uint32 time) {m_weaponChangeTimer = time;}

        uint64 GetMoney() const { return GetUInt64Value(PLAYER_FIELD_COINAGE); }
        void ModifyMoney(int64 d);
        bool HasEnoughMoney(uint64 amount) const { return GetMoney() >= amount; }
        bool HasEnoughMoney(int64 amount) const
        {
            if (amount > 0)
                return (GetMoney() >= (uint64)amount);
            return true;
        }

        void SetMoney(uint64 value)
        {
            SetUInt64Value(PLAYER_FIELD_COINAGE, value);
            MoneyChanged(value);
            UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_GOLD_VALUE_OWNED);
        }

        RewardedQuestSet const& getRewardedQuests() const { return m_RewardedQuests; }
        QuestStatusMap& getQuestStatusMap() { return m_QuestStatus; }

        size_t GetRewardedQuestCount() const { return m_RewardedQuests.size(); }
        bool IsQuestRewarded(uint32 quest_id) const
        {
            return m_RewardedQuests.find(quest_id) != m_RewardedQuests.end();
        }

        uint64 GetSelection() const { return m_curSelection; }
        Unit* GetSelectedUnit() const;
        Player* GetSelectedPlayer() const;
        void SetSelection(uint64 guid) { m_curSelection = guid; SetGuidValue(UNIT_FIELD_TARGET, guid); }

        void SendMailResult(uint32 mailId, MailResponseType mailAction, MailResponseResult mailError, uint32 equipError = 0, uint32 item_guid = 0, uint32 item_count = 0);
        void SendNewMail();
        void UpdateNextMailTimeAndUnreads();
        void AddNewMailDeliverTime(time_t deliver_time);
        bool IsMailsLoaded() const { return m_mailsLoaded; }

        void RemoveMail(uint32 id);

        void AddMail(Mail* mail) { m_mail.push_front(mail);}// for call from WorldSession::SendMailTo
        uint32 GetMailSize() { return m_mail.size();}
        Mail* GetMail(uint32 id);

        PlayerMails::iterator GetMailBegin() { return m_mail.begin();}
        PlayerMails::iterator GetMailEnd() { return m_mail.end();}

        /*********************************************************/
        /*** MAILED ITEMS SYSTEM ***/
        /*********************************************************/

        uint8 unReadMails;
        time_t m_nextMailDelivereTime;

        typedef UNORDERED_MAP<uint32, Item*> ItemMap;

        ItemMap mMitems;                                    //template defined in objectmgr.cpp

        Item* GetMItem(uint32 id)
        {
            ItemMap::const_iterator itr = mMitems.find(id);
            return itr != mMitems.end() ? itr->second : NULL;
        }

        void AddMItem(Item* it)
        {
            ASSERT(it);
            //ASSERT deleted, because items can be added before loading
            mMitems[it->GetGUIDLow()] = it;
        }

        bool RemoveMItem(uint32 id)
        {
            return mMitems.erase(id) ? true : false;
        }

        void PetSpellInitialize();
        void CharmSpellInitialize();
        void PossessSpellInitialize();
        void VehicleSpellInitialize();
        void SendRemoveControlBar();
        void SendKnownSpells();
        bool HasSpell(uint32 spell) const;
        bool HasActiveSpell(uint32 spell) const;            // show in spellbook
        TrainerSpellState GetTrainerSpellState(TrainerSpell const* trainer_spell) const;
        bool IsSpellFitByClassAndRace(uint32 spell_id) const;
        bool IsNeedCastPassiveSpellAtLearn(SpellInfo const* spellInfo) const;

        void SendProficiency(ItemClass itemClass, uint32 itemSubclassMask);
        bool addSpell(uint32 spellId, bool active, bool learning, bool dependent, bool disabled, bool loading = false, bool p_IsMountFavorite = false);
        void learnSpell(uint32 spell_id, bool dependent);
        void removeSpell(uint32 spell_id, bool disabled = false, bool learn_low_rank = true);
        void resetSpells(bool myClassOnly = false);
        void learnDefaultSpells();
        void learnQuestRewardedSpells();
        void learnQuestRewardedSpells(Quest const* quest);
        void learnSpellHighRank(uint32 spellid);
        void AddTemporarySpell(uint32 spellId);
        void RemoveTemporarySpell(uint32 spellId);
        void SetReputation(uint32 factionentry, uint32 value);
        uint32 GetReputation(uint32 factionentry);
        std::string GetGuildName();

        void MountSetFavorite(uint32 p_SpellID, bool p_IsFavorite);

        // Talents
        uint32 GetFreeTalentPoints() const { return _talentMgr->FreeTalentPoints; }
        void SetFreeTalentPoints(uint32 points) { _talentMgr->FreeTalentPoints = points; }
        uint32 GetUsedTalentCount() const { return _talentMgr->UsedTalentCount; }
        void SetUsedTalentCount(uint32 talents) { _talentMgr->UsedTalentCount = talents; }
        uint32 GetQuestRewardedTalentCount() const { return _talentMgr->QuestRewardedTalentCount; }
        void AddQuestRewardedTalentCount(uint32 points) { _talentMgr->QuestRewardedTalentCount += points; }
        uint32 GetTalentResetCost() const { return _talentMgr->ResetTalentsCost; }
        void SetTalentResetCost(uint32 cost)  { _talentMgr->ResetTalentsCost = cost; }
        uint32 GetSpecializationResetCost() const { return _talentMgr->ResetSpecializationCost; }
        void SetSpecializationResetCost(uint32 cost) { _talentMgr->ResetSpecializationCost = cost; }
        uint32 GetSpecializationResetTime() const { return _talentMgr->ResetSpecializationTime; }
        void SetSpecializationResetTime(time_t time_) { _talentMgr->ResetSpecializationTime = time_; }
        uint32 GetTalentResetTime() const { return _talentMgr->ResetTalentsTime; }
        void SetTalentResetTime(time_t time_)  { _talentMgr->ResetTalentsTime = time_; }
        uint32 GetPrimaryTalentTree(uint8 spec) const { return _talentMgr->SpecInfo[spec].TalentTree; }
        void SetPrimaryTalentTree(uint8 spec, uint32 tree) { _talentMgr->SpecInfo[spec].TalentTree = tree; }
        uint8 GetActiveSpec() const { return _talentMgr->ActiveSpec; }
        void SetActiveSpec(uint8 spec){ _talentMgr->ActiveSpec = spec; }
        uint8 GetSpecsCount() const { return _talentMgr->SpecsCount; }
        void SetSpecsCount(uint8 count) { _talentMgr->SpecsCount = count; }
        void SetSpecializationId(uint8 spec, uint32 id, bool loading = false);
        uint32 GetSpecializationId(uint8 spec) const { return _talentMgr->SpecInfo[spec].SpecializationId; }
        uint32 GetRoleForGroup(uint32 specializationId = 0);
        Stats GetPrimaryStat() const;
        bool IsActiveSpecTankSpec() const;

        bool ResetTalents(bool no_cost = false);
        uint32 GetNextResetTalentsCost() const;
        uint32 GetNextResetSpecializationCost() const;
        void InitTalentForLevel();
        void InitSpellForLevel();
        void RemoveSpecializationSpells();
        void BuildPlayerTalentsInfoData(WorldPacket* data);
        void SendTalentsInfoData(bool pet);
        bool LearnTalent(uint32 talentId);
        bool AddTalent(uint32 spellId, uint8 spec, bool learning);
        bool HasTalent(uint32 spell_id, uint8 spec) const;
        uint32 CalculateTalentsPoints() const;
        void CastPassiveTalentSpell(uint32 spellId);
        void RemovePassiveTalentSpell(uint32 spellId);

        void ResetSpec(bool p_NoCost = false);

        // Dual Spec
        void UpdateSpecCount(uint8 count);
        void ActivateSpec(uint8 spec);

        void InitGlyphsForLevel();
        void SetGlyphSlot(uint8 slot, uint32 slottype) { SetUInt32Value(PLAYER_FIELD_GLYPH_SLOTS + slot, slottype); }
        uint32 GetGlyphSlot(uint8 slot) const { return GetUInt32Value(PLAYER_FIELD_GLYPH_SLOTS + slot); }
        void SetGlyph(uint8 slot, uint32 glyph)
        {
            _talentMgr->SpecInfo[GetActiveSpec()].Glyphs[slot] = glyph;
            SetUInt32Value(PLAYER_FIELD_GLYPHS + slot, glyph);
        }
        uint32 GetGlyph(uint8 spec, uint8 slot) const { return _talentMgr->SpecInfo[spec].Glyphs[slot]; }
        bool HasGlyph(uint32 spell_id);

        PlayerTalentMap const* GetTalentMap(uint8 spec) const { return _talentMgr->SpecInfo[spec].Talents; }
        PlayerTalentMap* GetTalentMap(uint8 spec) { return _talentMgr->SpecInfo[spec].Talents; }
        ActionButtonList const& GetActionButtons() const { return m_actionButtons; }

        uint32 GetFreePrimaryProfessionPoints() const { return GetUInt32Value(PLAYER_FIELD_CHARACTER_POINTS); }
        void SetFreePrimaryProfessions(uint16 profs) { SetUInt32Value(PLAYER_FIELD_CHARACTER_POINTS, profs); }
        void InitPrimaryProfessions();

        PlayerSpellMap const& GetSpellMap() const { return m_spells; }
        PlayerSpellMap      & GetSpellMap()       { return m_spells; }

        SpellCooldowns const& GetSpellCooldownMap() const { return m_spellCooldowns; }

        void AddSpellMod(SpellModifier* mod, bool apply);
        bool IsAffectedBySpellmod(SpellInfo const* spellInfo, SpellModifier* mod, Spell* spell = NULL);
        template <class T> T ApplySpellMod(uint32 spellId, SpellModOp op, T &basevalue, Spell* spell = NULL, bool removestacks = true);
        void RemoveSpellMods(Spell* spell);
        void RestoreSpellMods(Spell* spell, uint32 ownerAuraId = 0, AuraPtr aura = NULLAURA);
        void RestoreAllSpellMods(uint32 ownerAuraId = 0, AuraPtr aura = NULLAURA);
        void DropModCharge(SpellModifier* mod, Spell* spell);
        void SetSpellModTakingSpell(Spell* spell, bool apply);

        static uint64 const infinityCooldownDelay = uint64(2592000LL * 1000LL); // MONTH * IN_MILLISECONDS
        static uint64 const infinityCooldownDelayCheck = uint64(2592000LL/2 * 1000LL); // MONTH/2 * IN_MILLISECONDS

        bool HasSpellCooldown(uint32 spell_id) const { return GetSpellCooldownDelay(spell_id) != 0; }
        uint32 GetSpellCooldownDelay(uint32 spell_id) const
        {
            SpellCooldowns::const_iterator itr = m_spellCooldowns.find(spell_id);
            uint64 currTime = 0;
            ACE_OS::gettimeofday().msec(currTime);
            return uint32(itr != m_spellCooldowns.end() && itr->second.end > currTime ? itr->second.end - currTime : 0);
        }
        void AddSpellAndCategoryCooldowns(SpellInfo const* spellInfo, uint32 itemId, Spell* spell = NULL, bool infinityCooldown = false);
        void AddSpellCooldown(uint32 spell_id, uint32 itemid, uint64 end_time, bool send = false);
        void SendCategoryCooldown(uint32 categoryId, int32 cooldown);
        void SendCooldownEvent(const SpellInfo * p_SpellInfo, uint32 p_ItemID = 0, Spell * p_Spell = NULL, bool p_SetCooldown = true);
        void ProhibitSpellSchool(SpellSchoolMask idSchoolMask, uint32 unTimeMs);
        void ReduceSpellCooldown(uint32 spell_id, time_t modifyTime);
        void RemoveSpellCooldown(uint32 spell_id, bool update = false);
        void RemoveSpellCategoryCooldown(uint32 cat, bool update = false);
        void SendClearCooldown(uint32 p_SpellID, Unit * p_Target, bool p_ClearOnHold = false);

        GlobalCooldownMgr& GetGlobalCooldownMgr() { return m_GlobalCooldownMgr; }

        void RemoveCategoryCooldown(uint32 cat);
        void RemoveArenaSpellCooldowns(bool removeActivePetCooldowns = false);
        void RemoveAllSpellCooldown();
        void _LoadSpellCooldowns(PreparedQueryResult result);
        void _LoadChargesCooldowns(PreparedQueryResult p_Result);
        void _SaveSpellCooldowns(SQLTransaction& trans);
        void _SaveChargesCooldowns(SQLTransaction& p_Transaction);
        void SetLastPotionId(uint32 item_id) { m_lastPotionId = item_id; }
        void UpdatePotionCooldown(Spell* spell = NULL);

        void SetResurrectRequestData(Unit* caster, uint32 health, uint32 mana, uint32 appliedAura)
        {
            ASSERT(!IsRessurectRequested());
            _resurrectionData = new ResurrectionData();
            _resurrectionData->GUID = caster->GetGUID();
            _resurrectionData->Location.WorldRelocate(*caster);
            _resurrectionData->Health = health;
            _resurrectionData->Mana = mana;
            _resurrectionData->Aura = appliedAura;
        }

        void ClearResurrectRequestData()
        {
            delete _resurrectionData;
            _resurrectionData = NULL;
        }
        bool IsRessurectRequestedBy(uint64 guid) const
        {
            if (!IsRessurectRequested())
                return false;

            return _resurrectionData->GUID == guid;
        }

        bool IsRessurectRequested() const { return _resurrectionData != NULL; }

        void ResurectUsingRequestData();

        uint8 getCinematic()
        {
            return m_cinematic;
        }
        void setCinematic(uint8 cine)
        {
            m_cinematic = cine;
        }

        ActionButton* addActionButton(uint8 button, uint32 action, uint8 type);
        void removeActionButton(uint8 button);
        ActionButton const* GetActionButton(uint8 button);
        void SendInitialActionButtons() const { SendActionButtons(0); }
        void SendActionButtons(uint32 state) const;
        void BuildActionButtonsDatas(WorldPacket* data, uint8 buttons[MAX_ACTION_BUTTONS][8], uint8 order, bool byte = false) const;
        bool IsActionButtonDataValid(uint8 button, uint32 action, uint8 type);

        int8 GetFreeActionButton();

        PvPInfo pvpInfo;
        void UpdatePvPState(bool onlyFFA = false);
        void SetPvP(bool state)
        {
            Unit::SetPvP(state);
            for (ControlList::iterator itr = m_Controlled.begin(); itr != m_Controlled.end(); ++itr)
                (*itr)->SetPvP(state);
        }
        void UpdatePvP(bool state, bool override=false);
        void UpdateZone(uint32 newZone, uint32 newArea);
        void UpdateArea(uint32 newArea);

        void UpdateZoneDependentAuras(uint32 zone_id);    // zones
        void UpdateAreaDependentAuras(uint32 area_id);    // subzones

        void UpdateAfkReport(time_t currTime);
        void UpdatePvPFlag(time_t currTime);
        void UpdateContestedPvP(uint32 currTime);
        void SetContestedPvPTimer(uint32 newTime) {m_contestedPvPTimer = newTime;}
        void ResetContestedPvP()
        {
            ClearUnitState(UNIT_STATE_ATTACK_PLAYER);
            RemoveFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_CONTESTED_PVP);
            m_contestedPvPTimer = 0;
        }

        /** todo: -maybe move UpdateDuelFlag+DuelComplete to independent DuelHandler.. **/
        DuelInfo* m_Duel;
        void UpdateDuelFlag(time_t currTime);
        void CheckDuelDistance(time_t currTime);
        void DuelComplete(DuelCompleteType type);
        void SendDuelCountdown(uint32 counter);

        bool IsGroupVisibleFor(Player const* p) const;
        bool IsInSameGroupWith(Player const* p) const;
        bool IsInSameRaidWith(Player const* p) const { return p == this || (GetGroup() != NULL && GetGroup() == p->GetGroup()); }
        void UninviteFromGroup();
        static void RemoveFromGroup(Group* group, uint64 guid, RemoveMethod method = GROUP_REMOVEMETHOD_DEFAULT, uint64 kicker = 0, const char* reason = NULL);
        void RemoveFromGroup(RemoveMethod method = GROUP_REMOVEMETHOD_DEFAULT) { RemoveFromGroup(GetGroup(), GetGUID(), method); }
        void SendUpdateToOutOfRangeGroupMembers();

        void SetInGuild(uint32 guildId);
        void SetRank(uint8 rankId) { SetUInt32Value(PLAYER_FIELD_GUILD_RANK_ID, rankId); }
        uint32 GetRank() { return GetUInt32Value(PLAYER_FIELD_GUILD_RANK_ID); }
        void SetGuildLevel(uint32 level) { SetUInt32Value(PLAYER_FIELD_GUILD_LEVEL, level); }
        uint32 GetGuildLevel() { return GetUInt32Value(PLAYER_FIELD_GUILD_LEVEL); }
        void SetGuildIdInvited(uint32 GuildId) { m_GuildIdInvited = GuildId; }
        uint32 GetGuildId() const { return GetUInt32Value(OBJECT_FIELD_DATA); /* return only lower part */ }
        Guild* GetGuild();
        static uint32 GetGuildIdFromDB(uint64 guid);
        static uint8 GetRankFromDB(uint64 guid);
        int GetGuildIdInvited() { return m_GuildIdInvited; }
        static void RemovePetitionsAndSigns(uint64 guid, uint32 type);

        // Arena
        uint32 GetArenaPersonalRating(uint8 slot) const { ASSERT(slot < MAX_PVP_SLOT); return m_ArenaPersonalRating[slot]; }
        uint32 GetBestRatingOfWeek(uint8 slot) const { ASSERT(slot < MAX_PVP_SLOT); return m_BestRatingOfWeek[slot]; }
        uint32 GetBestRatingOfSeason(uint8 slot) const { ASSERT(slot < MAX_PVP_SLOT); return m_BestRatingOfSeason[slot]; }
        uint32 GetWeekWins(uint8 slot) const { ASSERT(slot < MAX_PVP_SLOT); return m_WeekWins[slot]; }
        uint32 GetPrevWeekWins(uint8 slot) const { ASSERT(slot < MAX_PVP_SLOT); return m_PrevWeekWins[slot]; }
        uint32 GetSeasonWins(uint8 slot) const { ASSERT(slot < MAX_PVP_SLOT); return m_SeasonWins[slot]; }
        uint32 GetWeekGames(uint8 slot) const { ASSERT(slot < MAX_PVP_SLOT); return m_WeekGames[slot]; }
        uint32 GetSeasonGames(uint8 slot) const { ASSERT(slot < MAX_PVP_SLOT); return m_SeasonGames[slot]; }
        uint32 GetArenaMatchMakerRating(uint8 slot) const { ASSERT(slot < MAX_PVP_SLOT); return m_ArenaMatchMakerRating[slot]; }

        uint32 GetMaxRating() const
        {
            uint32 max_value = 0;

            for (uint8 i = 0; i < MAX_PVP_SLOT; i++)
                if (max_value < GetArenaPersonalRating(i))
                    max_value = GetArenaPersonalRating(i);

            return max_value;
        }

        void SetArenaPersonalRating(uint8 slot, uint32 value)
        {
            if (slot >= MAX_PVP_SLOT)
                return;

            if (value > 3500)
            {
                ACE_Stack_Trace trace;
                sLog->outError(LOG_FILTER_GENERAL, "Suspiciously high personal rating. Rating: %u, Slot: %u, Player: %u, Trace log: %s", value, slot, GUID_LOPART(GetGUID()), trace.c_str());
            }

            m_ArenaPersonalRating[slot] = value;
            if (m_BestRatingOfWeek[slot] < value)
                m_BestRatingOfWeek[slot] = value;
            if (m_BestRatingOfSeason[slot] < value)
                m_BestRatingOfSeason[slot] = value;
        }

        void SetArenaMatchMakerRating(uint8 slot, uint32 value)
        {
            if (slot >= MAX_PVP_SLOT)
                return;

            if (value > 3500)
            {
                ACE_Stack_Trace trace;
                sLog->outError(LOG_FILTER_GENERAL, "Suspiciously high match maker rating. Rating: %u, Slot: %u, Player: %u, Trace log: %s", value, slot, GUID_LOPART(GetGUID()), trace.c_str());
            }

            m_ArenaMatchMakerRating[slot] = value;
        }
        void IncrementWeekGames(uint8 slot)
        {
            if (slot >= MAX_PVP_SLOT)
                return;

            ++m_WeekGames[slot];
        }
        void IncrementWeekWins(uint8 slot)
        {
            if (slot >= MAX_PVP_SLOT)
                return;

            ++m_WeekWins[slot];
        }
        void IncrementSeasonGames(uint8 slot)
        {
            if (slot >= MAX_PVP_SLOT)
                return;

            ++m_SeasonGames[slot];
        }
        void IncrementSeasonWins(uint8 slot)
        {
            if (slot >= MAX_PVP_SLOT)
                return;

            ++m_SeasonWins[slot];
        }
        void FinishWeek();

        void SendStartTimer(uint32 p_Time, uint32 p_MaxTime, uint8 p_Type);

        Difficulty GetDifficulty(bool isRaid) const { return isRaid ? m_LegacyRaidDifficulty : m_dungeonDifficulty; }
        Difficulty GetDungeonDifficulty() const { return m_dungeonDifficulty; }
        Difficulty GetRaidDifficulty() const { return m_raidDifficulty; }
        Difficulty GetLegacyRaidDifficulty() const { return m_LegacyRaidDifficulty; }
        Difficulty GetStoredRaidDifficulty() const { return m_raidMapDifficulty; } // only for use in difficulty packet after exiting to raid map
        void SetDungeonDifficulty(Difficulty dungeon_difficulty) { m_dungeonDifficulty = dungeon_difficulty; }
        void SetRaidDifficulty(Difficulty raid_difficulty) { m_raidDifficulty = raid_difficulty; }
        void SetLegacyRaidDifficulty(Difficulty p_LegacyRaidDifficulty) { m_LegacyRaidDifficulty = p_LegacyRaidDifficulty; }
        void StoreRaidMapDifficulty() { m_raidMapDifficulty = GetMap()->GetDifficulty(); }

        bool UpdateSkill(uint32 skill_id, uint32 step);
        bool UpdateSkillPro(uint16 SkillId, int32 Chance, uint32 step);

        bool UpdateCraftSkill(uint32 spellid);
        bool UpdateGatherSkill(uint32 SkillId, uint32 SkillValue, uint32 RedLevel, uint32 Multiplicator = 1);
        bool UpdateFishingSkill();

        uint32 GetSpellByProto(ItemTemplate* proto);

        float GetHealthBonusFromStamina();

        bool UpdateStats(Stats stat);
        bool UpdateAllStats();
        void UpdateResistances(uint32 school);
        void UpdateArmor();
        void UpdateMaxHealth();
        void UpdateMaxPower(Powers power);
        void UpdateAttackPowerAndDamage(bool ranged = false);
        void UpdateDamagePhysical(WeaponAttackType attType);
        void ApplySpellPowerBonus(int32 amount, bool apply);
        void UpdateSpellDamageAndHealingBonus();
        void ApplyRatingMod(CombatRating cr, int32 value, bool apply);
        void UpdateRating(CombatRating cr);
        void UpdateItemLevel();
        void UpdateAllRatings();

        void CalculateMinMaxDamage(WeaponAttackType attType, bool normalized, bool addTotalPct, float& min_damage, float& max_damage);

        inline void RecalculateRating(CombatRating cr) { ApplyRatingMod(cr, 0, true);}
        float OCTRegenMPPerSpirit();
        float GetRatingMultiplier(CombatRating cr) const;
        float GetRatingBonusValue(CombatRating cr) const;
        uint32 GetBaseSpellPowerBonus() const { return m_baseSpellPower; }
        int32 GetSpellPenetrationItemMod() const { return m_spellPenetrationItemMod; }

        float GetExpertiseDodgeOrParryReduction(WeaponAttackType attType) const;
        void UpdateBlockPercentage();
        void UpdateCritPercentage(WeaponAttackType attType);
        void UpdateAllCritPercentages();
        void UpdateParryPercentage();
        void UpdateDodgePercentage();
        void UpdateMasteryPercentage();
        void UpdatePvPPowerPercentage();
        void UpdateMultistrikePercentage();
        void UpdateLeechPercentage();
        void UpdateVersatilityPercentage();
        void UpdateAvoidancePercentage();
        void UpdateSpeedPercentage();

        void UpdateAllSpellCritChances();
        void UpdateSpellCritChance(uint32 school);
        void UpdateArmorPenetration(int32 amount);
        void ApplyManaRegenBonus(int32 amount, bool apply);
        void ApplyHealthRegenBonus(int32 amount, bool apply);
        void UpdateManaRegen();
        void UpdateEnergyRegen();
        void UpdateFocusRegen();
        void UpdateRuneRegen(RuneType rune);
        void UpdateAllRunesRegen();
        float GetRegenForPower(Powers p_Power);

        bool CanSwitch() const;
        bool IsInWorgenForm() const { return HasAuraType(SPELL_AURA_ALLOW_WORGEN_TRANSFORM); }
        void SwitchToHumanForm() { RemoveAurasByType(SPELL_AURA_ALLOW_WORGEN_TRANSFORM); }
        void SwitchToWorgenForm() { CastSpell(this, 97709, true); }
        void SwitchForm();

        uint64 GetLootGUID() const { return m_lootGuid; }
        void SetLootGUID(uint64 guid) { m_lootGuid = guid; }

        uint32 GetLootSpecId() const { return m_lootSpecId; }
        void SetLootSpecId(uint32 specId) { m_lootSpecId = specId; }

        uint32 GetBonusRollFails() const { return m_BonusRollFails; }
        void IncreaseBonusRollFails() { ++m_BonusRollFails; }
        void ResetBonusRollFails() { m_BonusRollFails = 0; }

        void RemovedInsignia(Player* looterPlr);

        WorldSession* GetSession() const { return m_session; }

        void BuildCreateUpdateBlockForPlayer(UpdateData* data, Player* target) const;
        void DestroyForPlayer(Player* target, bool onDeath = false) const;
        void SendLogXPGain(uint32 GivenXP, Unit* victim, uint32 BonusXP, bool recruitAFriend = false, float group_rate=1.0f);

        // notifiers
        void SendAttackSwingError(AttackSwingError p_Error);
        void SendAttackSwingCancelAttack();
        void SendAutoRepeatCancel(Unit* target);
        void SendExplorationExperience(uint32 Area, uint32 Experience);

        void SendDungeonDifficulty();
        void SendRaidDifficulty(int32 forcedDifficulty = -1);
        void ResetInstances(uint8 method, bool isRaid);
        void SendResetInstanceSuccess(uint32 MapId);
        void SendResetInstanceFailed(uint32 reason, uint32 MapId);
        void SendResetFailedNotify();

        virtual bool UpdatePosition(float x, float y, float z, float orientation, bool teleport = false);
        bool UpdatePosition(const Position &pos, bool teleport = false) { return UpdatePosition(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation(), teleport); }
        void UpdateUnderwaterState(Map* m, float x, float y, float z);

        void SendMessageToSet(WorldPacket* data, bool self) {SendMessageToSetInRange(data, GetVisibilityRange(), self); };// overwrite Object::SendMessageToSet
        void SendMessageToSetInRange(WorldPacket* data, float fist, bool self);// overwrite Object::SendMessageToSetInRange
        void SendMessageToSetInRange(WorldPacket* data, float dist, bool self, bool own_team_only);
        void SendMessageToSet(WorldPacket* data, Player const* skipped_rcvr);

        void SendTeleportPacket(Position &oldPos);

        Corpse* GetCorpse() const;
        void SpawnCorpseBones();
        void CreateCorpse();
        void KillPlayer();
        uint32 GetResurrectionSpellId();
        void ResurrectPlayer(float restore_percent, bool applySickness = false);
        void BuildPlayerRepop();
        void RepopAtGraveyard();
        void TeleportToClosestGrave(float p_X, float p_Y, float p_Z, float p_O, uint32 p_MapId);
        void TeleportToClosestGrave(WorldSafeLocsEntry const* p_WorldSafeLoc) { TeleportToClosestGrave(p_WorldSafeLoc->x, p_WorldSafeLoc->y, p_WorldSafeLoc->z, p_WorldSafeLoc->o, p_WorldSafeLoc->map_id);  }
        void SendCemeteryList(bool onMap);

        void DurabilityLossAll(double percent, bool inventory);
        void DurabilityLoss(Item* item, double percent);
        void DurabilityPointsLossAll(int32 points, bool inventory);
        void DurabilityPointsLoss(Item* item, int32 points);
        void DurabilityPointLossForEquipSlot(EquipmentSlots slot);
        uint32 DurabilityRepairAll(bool cost, float discountMod, bool guildBank);
        uint32 DurabilityRepair(uint16 pos, bool cost, float discountMod, bool guildBank);

        void UpdateMirrorTimers();
        void StopMirrorTimers()
        {
            StopMirrorTimer(FATIGUE_TIMER);
            StopMirrorTimer(BREATH_TIMER);
            StopMirrorTimer(FIRE_TIMER);
        }
        bool IsMirrorTimerActive(MirrorTimerType type) { return m_MirrorTimer[type] == getMaxTimer(type); }

        bool CanJoinConstantChannelInZone(ChatChannelsEntry const* channel, AreaTableEntry const* zone);

        void JoinedChannel(Channel* c);
        void LeftChannel(Channel* c);
        void CleanupChannels();
        void UpdateLocalChannels(uint32 newZone);
        void LeaveLFGChannel();

        void SetSkill(uint16 id, uint16 step, uint16 currVal, uint16 maxVal);
        uint16 GetMaxSkillValue(uint32 skill) const;        // max + perm. bonus + temp bonus
        uint16 GetPureMaxSkillValue(uint32 skill) const;    // max
        uint16 GetSkillValue(uint32 skill) const;           // skill value + perm. bonus + temp bonus
        uint16 GetBaseSkillValue(uint32 skill) const;       // skill value + perm. bonus
        uint16 GetPureSkillValue(uint32 skill) const;       // skill value
        int16 GetSkillPermBonusValue(uint32 skill) const;
        int16 GetSkillTempBonusValue(uint32 skill) const;
        uint16 GetSkillStep(uint16 skill) const;            // 0...6
        bool HasSkill(uint32 skill) const;
        void learnSkillRewardedSpells(uint32 id, uint32 value);

        WorldLocation& GetTeleportDest() { return m_teleport_dest; }
        bool IsBeingTeleported() const { return mSemaphoreTeleport_Near || mSemaphoreTeleport_Far; }
        bool IsBeingTeleportedNear() const { return mSemaphoreTeleport_Near; }
        bool IsBeingTeleportedFar() const { return mSemaphoreTeleport_Far; }
        void SetSemaphoreTeleportNear(bool semphsetting) { mSemaphoreTeleport_Near = semphsetting; }
        void SetSemaphoreTeleportFar(bool semphsetting) { mSemaphoreTeleport_Far = semphsetting; }
        void ProcessDelayedOperations();

        void CheckAreaExploreAndOutdoor(void);

        static uint32 TeamForRace(uint8 race);
        uint32 GetTeam() const { return m_team; }
        TeamId GetTeamId() const { if (m_team == ALLIANCE) return TEAM_ALLIANCE; if (m_team == HORDE) return TEAM_HORDE; return TEAM_NEUTRAL; }
        void setFactionForRace(uint8 race);

        void InitDisplayIds();

        bool IsAtGroupRewardDistance(WorldObject const* pRewardSource) const;
        bool IsAtRecruitAFriendDistance(WorldObject const* pOther) const;
        void RewardCurrencyAtKill(Unit* victim);
        void RewardPlayerAndGroupAtKill(Unit* victim, bool isBattleGround);
        void RewardPlayerAndGroupAtEvent(uint32 creature_id, WorldObject* pRewardSource);
        bool isHonorOrXPTarget(Unit* victim);

        bool GetsRecruitAFriendBonus(bool forXP);
        uint8 GetGrantableLevels() { return m_grantableLevels; }
        void SetGrantableLevels(uint8 val) { m_grantableLevels = val; }

        Expansion GetExpByLevel();

        ReputationMgr&       GetReputationMgr()       { return m_reputationMgr; }
        ReputationMgr const& GetReputationMgr() const { return m_reputationMgr; }
        ReputationRank GetReputationRank(uint32 faction_id) const;
        void RewardReputation(Unit* victim, float rate);
        void RewardReputation(Quest const* quest);
        void RewardGuildReputation(Quest const* quest);

        void ModifySkillBonus(uint32 skillid, int32 val, bool talent);

        /*********************************************************/
        /***                  PVP SYSTEM                       ***/
        /*********************************************************/
        // @TODO: Properly implement correncies as of Cataclysm
        void UpdateHonorFields();
        bool RewardHonor(Unit* victim, uint32 groupsize, int32 honor = -1, bool pvptoken = false);
        uint32 GetMaxPersonalArenaRatingRequirement(uint32 minarenaslot) const;

        //End of PvP System

        inline SpellCooldowns GetSpellCooldowns() const { return m_spellCooldowns; }

        void SetDrunkValue(uint8 newDrunkValue, uint32 itemId = 0);
        uint8 GetDrunkValue() const { return GetByteValue(PLAYER_FIELD_ARENA_FACTION, 1); }
        static DrunkenState GetDrunkenstateByValue(uint8 value);

        uint32 GetDeathTimer() const { return m_deathTimer; }
        uint32 GetCorpseReclaimDelay(bool pvp) const;
        void UpdateCorpseReclaimDelay();
        void SendCorpseReclaimDelay(bool load = false);

        uint32 GetBlockPercent() { return GetUInt32Value(PLAYER_FIELD_SHIELD_BLOCK); }
        bool CanParry() const { return m_canParry; }
        void SetCanParry(bool value);
        bool CanBlock() const { return m_canBlock; }
        void SetCanBlock(bool value);
        bool CanMastery() const { return HasAuraType(SPELL_AURA_MASTERY); }
        bool CanTitanGrip() const { return m_canTitanGrip; }
        void SetCanTitanGrip(bool value) { m_canTitanGrip = value; }
        bool CanTameExoticPets() const { return isGameMaster() || HasAuraType(SPELL_AURA_ALLOW_TAME_PET_TYPE); }

        void SetRegularAttackTime();
        void SetBaseModValue(BaseModGroup modGroup, BaseModType modType, float value) { m_auraBaseMod[modGroup][modType] = value; }
        void HandleBaseModValue(BaseModGroup modGroup, BaseModType modType, float amount, bool apply);
        float GetBaseModValue(BaseModGroup modGroup, BaseModType modType) const;
        float GetTotalBaseModValue(BaseModGroup modGroup) const;
        float GetTotalPercentageModValue(BaseModGroup modGroup) const { return m_auraBaseMod[modGroup][FLAT_MOD] + m_auraBaseMod[modGroup][PCT_MOD]; }
        void _ApplyAllStatBonuses();
        void _RemoveAllStatBonuses();

        void ResetAllPowers();

        void _ApplyWeaponDependentAuraMods(Item* item, WeaponAttackType attackType, bool apply);
        void _ApplyWeaponDependentAuraCritMod(Item* item, WeaponAttackType attackType, constAuraEffectPtr aura, bool apply);
        void _ApplyWeaponDependentAuraDamageMod(Item* item, WeaponAttackType attackType, constAuraEffectPtr aura, bool apply);
        void _ApplyWeaponDependentAuraSpellModifier(Item* item, WeaponAttackType attackType, bool apply);

        void _ApplyItemMods(Item* item, uint8 slot, bool apply);
        void _RemoveAllItemMods();
        void _ApplyAllItemMods();
        void _ApplyItemBonuses(Item const* proto, uint8 slot, bool apply, uint32 rescaleToItemLevel = 0);
        void _ApplyItemModification(Item const* p_Item, ItemBonusEntry const* p_ItemBonusEntry, uint8 p_Slot, bool p_Apply, uint32 p_RescaleToItemLevel = 0);
        void _ApplyItemModifications(Item const* p_Item, uint8 p_Slot, bool p_Apply, uint32 p_RescaleToItemLevel = 0);
        void _ApplyWeaponDamage(uint8 slot, Item const* item, bool apply, uint32 minDamage = 0, uint32 maxDamage = 0);
        bool EnchantmentFitsRequirements(uint32 enchantmentcondition, int8 slot);
        void ToggleMetaGemsActive(uint8 exceptslot, bool apply);
        void CorrectMetaGemEnchants(uint8 slot, bool apply);
        void InitDataForForm(bool reapplyMods = false);

        void ApplyItemEquipSpell(Item* item, bool apply, bool form_change = false);
        void ApplyEquipSpell(SpellInfo const* spellInfo, Item* item, bool apply, bool form_change = false);
        void UpdateEquipSpellsAtFormChange();
        void CastItemCombatSpell(Unit* target, WeaponAttackType attType, uint32 procVictim, uint32 procEx);
        void CastItemUseSpell(Item* item, SpellCastTargets const& targets, uint8 cast_count, uint32 glyphIndex);
        void CastItemCombatSpell(Unit* target, WeaponAttackType attType, uint32 procVictim, uint32 procEx, Item* item, ItemTemplate const* proto);

        void SendEquipmentSetList();
        void SetEquipmentSet(uint32 index, EquipmentSet eqset);
        void DeleteEquipmentSet(uint64 setGuid);

        void SetEmoteState(uint32 anim_id);
        uint32 GetEmoteState() { return m_emote; }

        void SendInitWorldStates(uint32 zone, uint32 area);
        void SendUpdateWorldState(uint32 Field, uint32 Value);
        void SendDirectMessage(WorldPacket* data);
        void SendBGWeekendWorldStates();

        void SendAurasForTarget(Unit* target);
        void SendCooldownAtLogin();

        PlayerMenu* PlayerTalkClass;
        std::vector<ItemSetEffect*> ItemSetEff;

        void SendLoot(uint64 guid, LootType loot_type, bool fetchLoot = false);
        void SendLootRelease(uint64 p_LootGuid);
        void SendNotifyLootItemRemoved(uint8 lootSlot);
        void SendNotifyLootMoneyRemoved();

        /*********************************************************/
        /***               BATTLEGROUND SYSTEM                 ***/
        /*********************************************************/

        bool InBattleground()       const                { return m_bgData.bgInstanceID != 0; }
        bool InArena()              const;
        bool InRatedBattleGround()  const;
        uint32 GetBattlegroundId()  const                { return m_bgData.bgInstanceID; }
        BattlegroundTypeId GetBattlegroundTypeId() const { return m_bgData.bgTypeID; }
        Battleground* GetBattleground() const;

        uint32 GetBattlegroundQueueJoinTime(MS::Battlegrounds::BattlegroundType::Type bgTypeId) const
        {
            auto itr = m_bgData.bgQueuesJoinedTime.find(bgTypeId);
            return itr != m_bgData.bgQueuesJoinedTime.end() ? itr->second : time(NULL);
        }

        void ChangeBattlegroundQueueJoinTimeKey(MS::Battlegrounds::BattlegroundType::Type p_BgTypeId, MS::Battlegrounds::BattlegroundType::Type p_OldBgTypeId)
        {
            auto l_Itr = m_bgData.bgQueuesJoinedTime.find(p_OldBgTypeId);
            if (l_Itr != std::end(m_bgData.bgQueuesJoinedTime))
            {
                m_bgData.bgQueuesJoinedTime.insert(std::make_pair(p_BgTypeId, l_Itr->second));
                m_bgData.bgQueuesJoinedTime.erase(l_Itr);
            }
        }

        void AddBattlegroundQueueJoinTime(MS::Battlegrounds::BattlegroundType::Type bgTypeId, uint32 joinTime)
        {
            m_bgData.bgQueuesJoinedTime[bgTypeId] = joinTime;
        }

        void RemoveBattlegroundQueueJoinTime(MS::Battlegrounds::BattlegroundType::Type bgTypeId)
        {
            auto l_Itr = m_bgData.bgQueuesJoinedTime.find(bgTypeId);
            if (l_Itr != m_bgData.bgQueuesJoinedTime.end())
                m_bgData.bgQueuesJoinedTime.erase(l_Itr);
        }
        
        /// Returns true if the player is in a battleground queue.
        bool InBattlegroundQueue() const
        {
            for (uint8 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
                if (m_bgBattlegroundQueueID[i].BgType <= MS::Battlegrounds::BattlegroundType::DeepwindGorge)
                    return true;
            return false;
        }

        /// Returns true if the player is in an arena queue.
        bool InArenaQueue() const
        {
            for (uint8 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
                if (m_bgBattlegroundQueueID[i].BgType >= MS::Battlegrounds::BattlegroundType::Arena2v2 && m_bgBattlegroundQueueID[i].BgType <= MS::Battlegrounds::BattlegroundType::ArenaSkirmish3v3)
                    return true;
            return false;
        }

        void SetBattlegroundQueueTypeId(uint32 p_Index, MS::Battlegrounds::BattlegroundType::Type p_Type) { m_bgBattlegroundQueueID[p_Index].BgType = p_Type; }
        MS::Battlegrounds::BattlegroundType::Type GetBattlegroundQueueTypeId(uint32 index) const { return m_bgBattlegroundQueueID[index].BgType; }

        uint32 GetBattlegroundQueueIndex(MS::Battlegrounds::BattlegroundType::Type bgQueueTypeId) const
        {
            for (uint8 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
                if (m_bgBattlegroundQueueID[i].BgType == bgQueueTypeId)
                    return i;
            return PLAYER_MAX_BATTLEGROUND_QUEUES;
        }
        bool IsInvitedForBattlegroundQueueType(MS::Battlegrounds::BattlegroundType::Type bgQueueTypeId) const
        {
            for (uint8 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
                if (m_bgBattlegroundQueueID[i].BgType == bgQueueTypeId)
                    return m_bgBattlegroundQueueID[i].invitedToInstance != 0;
            return false;
        }
        bool InBattlegroundQueueForBattlegroundQueueType(MS::Battlegrounds::BattlegroundType::Type bgQueueTypeId) const
        {
            return GetBattlegroundQueueIndex(bgQueueTypeId) < PLAYER_MAX_BATTLEGROUND_QUEUES;
        }

        void SetBattlegroundId(uint32 val, BattlegroundTypeId bgTypeId)
        {
            m_bgData.bgInstanceID = val;
            m_bgData.bgTypeID = bgTypeId;
        }

        
        uint32 AddBattlegroundQueueId(MS::Battlegrounds::BattlegroundType::Type val)
        {
            for (uint8 i=0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
            {
                if (m_bgBattlegroundQueueID[i].BgType == MS::Battlegrounds::BattlegroundType::None || m_bgBattlegroundQueueID[i].BgType == val)
                {
                    m_bgBattlegroundQueueID[i].BgType = val;
                    m_bgBattlegroundQueueID[i].invitedToInstance = 0;
                    return i;
                }
            }
            return PLAYER_MAX_BATTLEGROUND_QUEUES;
        }
        bool HasFreeBattlegroundQueueId()
        {
            for (uint8 i=0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
                if (m_bgBattlegroundQueueID[i].BgType == MS::Battlegrounds::BattlegroundType::None)
                    return true;
            return false;
        }
        void RemoveBattlegroundQueueId(MS::Battlegrounds::BattlegroundType::Type val)
        {
            for (uint8 i=0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
            {
                if (m_bgBattlegroundQueueID[i].BgType == val)
                {
                    m_bgBattlegroundQueueID[i].BgType = MS::Battlegrounds::BattlegroundType::None;
                    m_bgBattlegroundQueueID[i].invitedToInstance = 0;
                    return;
                }
            }
        }
        void SetInviteForBattlegroundQueueType(MS::Battlegrounds::BattlegroundType::Type bgQueueTypeId, uint32 instanceId)
        {
            for (uint8 i=0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
                if (m_bgBattlegroundQueueID[i].BgType == bgQueueTypeId)
                    m_bgBattlegroundQueueID[i].invitedToInstance = instanceId;
        }
        bool IsInvitedForBattlegroundInstance(uint32 instanceId) const
        {
            for (uint8 i=0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
                if (m_bgBattlegroundQueueID[i].invitedToInstance == instanceId)
                    return true;
            return false;
        }

        WorldLocation const& GetBattlegroundEntryPoint() const { return m_bgData.joinPos; }
        void SetBattlegroundEntryPoint();

        void SetBGTeam(uint32 team) { m_bgData.bgTeam = team; }
        uint32 GetBGTeam() const { return m_bgData.bgTeam ? m_bgData.bgTeam : GetTeam(); }
        uint8 GetBGLastActiveSpec() const { return m_bgData.m_LastActiveSpec; }
        void SaveBGLastSpecialization() { m_bgData.m_LastActiveSpec = GetActiveSpec(); }

        void LeaveBattleground(bool teleportToEntryPoint = true);
        bool CanJoinToBattleground() const;
        bool CanReportAfkDueToLimit();
        void ReportedAfkBy(Player* reporter);
        void ClearAfkReports() { m_bgData.bgAfkReporter.clear(); }

        bool GetBGAccessByLevel(BattlegroundTypeId bgTypeId) const;
        bool isTotalImmunity();
        bool CanUseBattlegroundObject();
        bool isTotalImmune();
        bool CanCaptureTowerPoint();

        bool GetRandomWinner() { return m_IsBGRandomWinner; }
        void SetRandomWinner(bool isWinner);

        /*********************************************************/
        /***               OUTDOOR PVP SYSTEM                  ***/
        /*********************************************************/

        OutdoorPvP* GetOutdoorPvP() const;
        // returns true if the player is in active state for outdoor pvp objective capturing, false otherwise
        bool IsOutdoorPvPActive();

        /*********************************************************/
        /***                    REST SYSTEM                    ***/
        /*********************************************************/

        bool isRested() const { return GetRestTime() >= 10*IN_MILLISECONDS; }
        uint32 GetXPRestBonus(uint32 xp);
        uint32 GetRestTime() const { return m_restTime;}
        void SetRestTime(uint32 v) { m_restTime = v;}

        /*********************************************************/
        /***              ENVIROMENTAL SYSTEM                  ***/
        /*********************************************************/

        bool IsImmuneToEnvironmentalDamage();
        uint32 EnvironmentalDamage(EnviromentalDamage type, uint32 damage);

        /*********************************************************/
        /***               FLOOD FILTER SYSTEM                 ***/
        /*********************************************************/

        void UpdateSpeakTime();
        bool UpdatePmChatTime();
        bool CanSpeak() const;
        void ChangeSpeakTime(int utime);

        /*********************************************************/
        /***                 VARIOUS SYSTEMS                   ***/
        /*********************************************************/
        void UpdateFallInformationIfNeed(MovementInfo const& minfo, uint16 opcode);
        Unit* m_mover;
        WorldObject* m_seer;
        void SetFallInformation(uint32 time, float z)
        {
            m_lastFallTime = time;
            m_lastFallZ = z;
        }
        void HandleFall(MovementInfo const& movementInfo);

        bool IsKnowHowFlyIn(uint32 mapid, uint32 zone, uint32 spellId = 0) const;

        void SetClientControl(Unit* target, uint8 allowMove);

        void SetMover(Unit* target);

        bool SetHover(bool enable);

        void SendApplyMovementForce(uint64 p_Source, bool p_Apply, Position p_Direction, float p_Magnitude = 0.0f, uint8 p_Type = 0);
        void RemoveAllMovementForces();
        bool HasMovementForce(uint64 p_Source);

        void SetSeer(WorldObject* target) { m_seer = target; }
        void SetViewpoint(WorldObject* target, bool apply);
        WorldObject* GetViewpoint() const;
        void StopCastingCharm();
        void StopCastingBindSight();

        void SendPetTameResult(PetTameResult result);

        uint32 GetSaveTimer() const { return m_nextSave; }
        void   SetSaveTimer(uint32 timer) { m_nextSave = timer; }

        // Recall position
        uint32 m_recallMap;
        float  m_recallX;
        float  m_recallY;
        float  m_recallZ;
        float  m_recallO;
        void   SaveRecallPosition();

        void SetHomebind(WorldLocation const& loc, uint32 area_id);

        // Homebind coordinates
        uint32 m_homebindMapId;
        uint16 m_homebindAreaId;
        float m_homebindX;
        float m_homebindY;
        float m_homebindZ;

        WorldLocation GetStartPosition() const;

        // current pet slot
        PetSlot m_currentPetSlot;
        uint32 m_petSlotUsed;

        void setPetSlotUsed(PetSlot slot, bool used)
        {
            if (used)
                m_petSlotUsed |= (1 << int32(slot));
            else
                m_petSlotUsed &= ~(1 << int32(slot));
        }

        PetSlot getSlotForNewPet()
        {
            uint32 l_LastKnow = 0;
            // Call Pet Spells
            // 883 83242 83243 83244 83245
            //  1    2     3     4     5
            if (HasSpell(83245))
                l_LastKnow = 5;
            else if (HasSpell(83244))
                l_LastKnow = 4;
            else if (HasSpell(83243))
                l_LastKnow = 3;
            else if (HasSpell(83242))
                l_LastKnow = 2;
            else if (HasSpell(883))
                l_LastKnow = 1;

            for (uint32 i = uint32(PET_SLOT_HUNTER_FIRST); i < l_LastKnow; ++i)
                if ((m_petSlotUsed & (1 << i)) == 0)
                    return PetSlot(i);

            return PET_SLOT_FULL_LIST;
        }

        // currently visible objects at player client
        typedef std::set<uint64> ClientGUIDs;
        ClientGUIDs m_clientGUIDs;

        bool HaveAtClient(WorldObject const* u) const { return u == this || m_clientGUIDs.find(u->GetGUID()) != m_clientGUIDs.end(); }

        bool IsNeverVisible() const;

        bool IsVisibleGloballyFor(Player* player) const;

        void SendInitialVisiblePackets(Unit* target);
        void UpdateVisibilityForPlayer();
        void UpdateVisibilityOf(WorldObject* target);
        void UpdateTriggerVisibility();

        template<class T>
        void UpdateVisibilityOf(T* target, UpdateData& data, std::set<Unit*>& visibleNow);

        uint8 m_forced_speed_changes[MAX_MOVE_TYPE];

        bool HasAtLoginFlag(AtLoginFlags f) const { return m_atLoginFlags & f; }
        void SetAtLoginFlag(AtLoginFlags f) { m_atLoginFlags |= f; }
        void RemoveAtLoginFlag(AtLoginFlags flags, bool persist = false);

        bool isUsingLfg();

        typedef std::set<uint32> DFQuestsDoneList;
        DFQuestsDoneList m_DFQuests;

        // Temporarily removed pet cache
        uint32 GetTemporaryUnsummonedPetNumber() const { return m_temporaryUnsummonedPetNumber; }
        void SetTemporaryUnsummonedPetNumber(uint32 petnumber) { m_temporaryUnsummonedPetNumber = petnumber; }
        void UnsummonPetTemporaryIfAny();
        void ResummonPetTemporaryUnSummonedIfAny();
        bool IsPetNeedBeTemporaryUnsummoned() const { return !IsInWorld() || !isAlive(); }

        //////////////////////////////////////////////////////////////////////////
        /// Cinematic
        //////////////////////////////////////////////////////////////////////////
        void StopCinematic();
        void SendCinematicStart(uint32 CinematicSequenceId);
        bool IsInCinematic() { return m_InCinematic && m_CinematicSequence; }

        void SendMovieStart(uint32 MovieId);

        /*********************************************************/
        /***                 INSTANCE SYSTEM                   ***/
        /*********************************************************/

        typedef UNORDERED_MAP< uint32 /*mapId*/, InstancePlayerBind > BoundInstancesMap;

        void UpdateHomebindTime(uint32 time);

        uint32 m_HomebindTimer;
        bool m_InstanceValid;
        // permanent binds and solo binds by difficulty
        BoundInstancesMap m_boundInstances[MAX_DIFFICULTY];
        InstancePlayerBind* GetBoundInstance(uint32 mapid, Difficulty difficulty);
        BoundInstancesMap& GetBoundInstances(Difficulty difficulty) { return m_boundInstances[difficulty]; }
        InstanceSave* GetInstanceSave(uint32 mapid, bool raid);
        void UnbindInstance(uint32 mapid, Difficulty difficulty, bool unload = false);
        void UnbindInstance(BoundInstancesMap::iterator &itr, Difficulty difficulty, bool unload = false);
        InstancePlayerBind* BindToInstance(InstanceSave* save, bool permanent, bool load = false);
        void BindToInstance();
        void SetPendingBind(uint32 instanceId, uint32 bindTimer) { _pendingBindId = instanceId; _pendingBindTimer = bindTimer; }
        bool HasPendingBind() const { return _pendingBindId > 0; }
        void SendRaidInfo();
        void SendSavedInstances();
        static void ConvertInstancesToGroup(Player* player, Group* group, bool switchLeader);
        bool Satisfy(AccessRequirement const* ar, uint32 target_map, bool report = false);
        bool CheckInstanceLoginValid();
        bool CheckInstanceCount(uint32 instanceId) const
        {
            if (_instanceResetTimes.size() < sWorld->getIntConfig(CONFIG_MAX_INSTANCES_PER_HOUR))
                return true;
            return _instanceResetTimes.find(instanceId) != _instanceResetTimes.end();
        }

        void AddInstanceEnterTime(uint32 instanceId, time_t enterTime)
        {
            if (_instanceResetTimes.find(instanceId) == _instanceResetTimes.end())
                _instanceResetTimes.insert(InstanceTimeMap::value_type(instanceId, enterTime + HOUR));
        }

        // last used pet number (for BG's)
        uint32 GetLastPetNumber() const { return m_lastpetnumber; }
        void SetLastPetNumber(uint32 petnumber) { m_lastpetnumber = petnumber; }

        /*********************************************************/
        /***                   GROUP SYSTEM                    ***/
        /*********************************************************/

        uint32 GetGroupInvite() { return m_groupInviteGUID; }
        void SetGroupInvite(uint32 groupGUID) { m_groupInviteGUID = groupGUID; }
        Group* GetGroup() { return m_group.getTarget(); }
        const Group* GetGroup() const { return (const Group*)m_group.getTarget(); }
        GroupReference& GetGroupRef() { return m_group; }
        void SetGroup(Group* group, int8 subgroup = -1);
        uint8 GetSubGroup() const { return m_group.getSubGroup(); }
        uint32 GetGroupUpdateFlag() const { return m_groupUpdateMask; }
        void SetGroupUpdateFlag(uint32 flag) { m_groupUpdateMask |= flag; }
        uint64 GetAuraUpdateMaskForRaid() const { return m_auraRaidUpdateMask; }
        void SetAuraUpdateMaskForRaid(uint8 slot) { m_auraRaidUpdateMask |= (uint64(1) << slot); }
        Player* GetNextRandomRaidMember(float radius);
        PartyResult CanUninviteFromGroup() const;

        // Battleground / Battlefield Group System
        void SetBattlegroundOrBattlefieldRaid(Group* group, int8 subgroup = -1);
        void RemoveFromBattlegroundOrBattlefieldRaid();
        Group* GetOriginalGroup() { return m_originalGroup.getTarget(); }
        GroupReference& GetOriginalGroupRef() { return m_originalGroup; }
        uint8 GetOriginalSubGroup() const { return m_originalGroup.getSubGroup(); }
        void SetOriginalGroup(Group* group, int8 subgroup = -1);

        void SetPassOnGroupLoot(bool bPassOnGroupLoot) { m_bPassOnGroupLoot = bPassOnGroupLoot; }
        bool GetPassOnGroupLoot() const { return m_bPassOnGroupLoot; }

        MapReference &GetMapRef() { return m_mapRef; }

        // Set map to player and add reference
        void SetMap(Map* map);
        void ResetMap();

        bool isAllowedToLoot(const Creature* creature);

        DeclinedName const* GetDeclinedNames() const { return m_declinedname; }
        uint8 GetRunesState() const { return m_runes.runeState; }
        RuneType GetBaseRune(uint8 index) const { return RuneType(m_runes.runes[index].BaseRune); }
        RuneType GetCurrentRune(uint8 index) const { return RuneType(m_runes.runes[index].CurrentRune); }
        uint32 GetRuneCooldown(uint8 index) const { return m_runes.runes[index].Cooldown; }
        uint32 GetRuneBaseCooldown(uint8 index) const { return GetRuneTypeBaseCooldown(GetBaseRune(index)); }
        uint32 GetRuneConvertSpell(uint8 index) const { return m_runes.runes[index].spell_id; }
        uint32 GetRuneTypeBaseCooldown(RuneType runeType) const;
        bool IsBaseRuneSlotsOnCooldown(RuneType runeType) const;
        void SetDeathRuneUsed(uint8 index, bool apply) { m_runes.runes[index].DeathUsed = apply; }
        bool IsDeathRuneUsed(uint8 index) { return m_runes.runes[index].DeathUsed; }
        bool IsRunePermanentlyConverted(uint8 index) { return m_runes.runes[index].Permanently; }
        void SetBaseRune(uint8 index, RuneType baseRune) { m_runes.runes[index].BaseRune = baseRune; }
        void SetCurrentRune(uint8 index, RuneType currentRune) { m_runes.runes[index].CurrentRune = currentRune; }
        void SetRuneCooldown(uint8 index, uint32 cooldown) { m_runes.runes[index].Cooldown = cooldown; m_runes.SetRuneState(index, cooldown == 0); }
        void SetRuneConvertSpell(uint8 index, uint32 spell_id) { m_runes.runes[index].spell_id = spell_id; }
        void SetRuneConvertType(uint8 index, bool permanently) { m_runes.runes[index].Permanently = permanently; }
        void AddRuneBySpell(uint8 index, RuneType newType, uint32 spell_id) { SetRuneConvertSpell(index, spell_id); ConvertRune(index, newType); }
        void RemoveRunesBySpell(uint32 spell_id);
        void RestoreBaseRune(uint8 index);
        void ConvertRune(uint8 index, RuneType newType);
        void ResyncRunes(uint8 count);
        void SendDeathRuneUpdate();
        void AddRunePower(uint8 index);
        void InitRunes();

        AchievementMgr<Player>& GetAchievementMgr() { return m_achievementMgr; }
        AchievementMgr<Player> const& GetAchievementMgr() const { return m_achievementMgr; }
        void UpdateAchievementCriteria(AchievementCriteriaTypes type, uint64 miscValue1 = 0, uint64 miscValue2 = 0, uint64 miscValue3 = 0, Unit* unit = NULL);
        void CompletedAchievement(AchievementEntry const* entry);

        bool HasTitle(uint32 bitIndex);
        bool HasTitle(CharTitlesEntry const* title) { return HasTitle(title->MaskID); }
        void SetTitle(CharTitlesEntry const* title, bool lost = false);

        //bool isActiveObject() const { return true; }
        bool canSeeSpellClickOn(Creature const* creature) const;

        uint32 GetChampioningFaction() const { return m_ChampioningFaction; }
        void SetChampioningFaction(uint32 faction) { m_ChampioningFaction = faction; }
        Spell* m_spellModTakingSpell;

        uint32 GetAverageItemLevelEquipped();
        uint32 GetAverageItemLevelTotal();
        bool isDebugAreaTriggers;

        void ClearWhisperWhiteList() { WhisperList.clear(); }
        void AddWhisperWhiteList(uint64 guid) { WhisperList.push_back(guid); }
        bool IsInWhisperWhiteList(uint64 guid);

        /*! These methods send different packets to the client in apply and unapply case.
            These methods are only sent to the current unit.
        */
        void SendMovementSetCanFly(bool apply);
        void SendMovementSetCanTransitionBetweenSwimAndFly(bool apply);
        void SendMovementSetHover(bool apply);
        void SendMovementSetWaterWalking(bool apply);
        void SendMovementSetFeatherFall(bool apply);
        void SendMovementSetCollisionHeight(float height);

        bool CanFly() const { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_CAN_FLY); }

        //! Return collision height sent to client
        float GetCollisionHeight(bool mounted)
        {
            if (mounted)
            {
                CreatureDisplayInfoEntry const* mountDisplayInfo = sCreatureDisplayInfoStore.LookupEntry(GetUInt32Value(UNIT_FIELD_MOUNT_DISPLAY_ID));
                if (!mountDisplayInfo)
                    return GetCollisionHeight(false);

                CreatureModelDataEntry const* mountModelData = sCreatureModelDataStore.LookupEntry(mountDisplayInfo->ModelId);
                if (!mountModelData)
                    return GetCollisionHeight(false);

                CreatureDisplayInfoEntry const* displayInfo = sCreatureDisplayInfoStore.LookupEntry(GetNativeDisplayId());
                ASSERT(displayInfo);
                CreatureModelDataEntry const* modelData = sCreatureModelDataStore.LookupEntry(displayInfo->ModelId);
                ASSERT(modelData);

                float scaleMod = GetFloatValue(OBJECT_FIELD_SCALE); // 99% sure about this

                return scaleMod * mountModelData->MountHeight + modelData->CollisionHeight * 0.5f;
            }
            else
            {
                //! Dismounting case - use basic default model data
                CreatureDisplayInfoEntry const* displayInfo = sCreatureDisplayInfoStore.LookupEntry(GetNativeDisplayId());
                ASSERT(displayInfo);
                CreatureModelDataEntry const* modelData = sCreatureModelDataStore.LookupEntry(displayInfo->ModelId);
                ASSERT(modelData);

                return modelData->CollisionHeight;
            }
        }

        uint16 GetPrimaryProfession(uint8 index) const
        {
            //ASSERT(index < DEFAULT_MAX_PRIMARY_TRADE_SKILL);
            return uint16(GetUInt32Value(PLAYER_FIELD_PROFESSION_SKILL_LINE + index));
        }

        void SetPrimaryProfession(uint8 index, uint16 skillId)
        {
            //ASSERT(index < DEFAULT_MAX_PRIMARY_TRADE_SKILL);
            SetUInt32Value(PLAYER_FIELD_PROFESSION_SKILL_LINE + index, skillId);
        }

        // Void Storage
        bool IsVoidStorageUnlocked() const { return HasFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_VOID_UNLOCKED); }
        void UnlockVoidStorage() { SetFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_VOID_UNLOCKED); }
        void LockVoidStorage() { RemoveFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_VOID_UNLOCKED); }
        uint8 GetNextVoidStorageFreeSlot() const;
        uint8 GetNumOfVoidStorageFreeSlots() const;
        uint8 AddVoidStorageItem(const VoidStorageItem& item);
        void AddVoidStorageItemAtSlot(uint8 slot, const VoidStorageItem& item);
        void DeleteVoidStorageItem(uint8 slot);
        bool SwapVoidStorageItem(uint8 oldSlot, uint8 newSlot);
        VoidStorageItem* GetVoidStorageItem(uint8 slot) const;
        VoidStorageItem* GetVoidStorageItem(uint64 id, uint8& slot) const;

        uint32 GetLastTargetedGO() { return _lastTargetedGO; }
        void SetLastTargetedGO(uint32 lastTargetedGO) { _lastTargetedGO = lastTargetedGO; }
        void ShowNeutralPlayerFactionSelectUI();

        float GetPersonnalXpRate() { return m_PersonnalXpRate; }
        void SetPersonnalXpRate(float PersonnalXpRate);

        void SetKnockBackTime(uint32 timer) { m_knockBackTimer = timer; }
        uint32 GetKnockBackTime() const { return m_knockBackTimer; }

        void SetLastPlayedEmote(uint32 id) { m_lastPlayedEmote = id; }
        uint32 GetLastPlayedEmote() { return m_lastPlayedEmote; }

        void SetIgnoreMovementCount(uint8 count) { m_ignoreMovementCount = count; }
        uint8 GetIgnoreMovementCount() const { return m_ignoreMovementCount; }

        void HandleStoreItemCallback(PreparedQueryResult result);
        void HandleStoreLevelCallback(PreparedQueryResult result);
        void HandleStoreGoldCallback(PreparedQueryResult result);
        void HandleStoreTitleCallback(PreparedQueryResult result);

        void CheckSpellAreaOnQuestStatusChange(uint32 quest_id);

        void SendCUFProfiles();

        void SendResumeToken(uint32 token);
        void SendRefreshSpellMods();

        uint8 GetBattleGroundRoles() const { return m_bgRoles; }
        void SetBattleGroundRoles(uint8 roles) { m_bgRoles = roles; }

        /*********************************************************/
        /***                  SCENES SYSTEM                    ***/
        /*********************************************************/
        void PlayScene(uint32 sceneId, WorldObject* spectator);

        /// Compute the unlocked pet battle slot
        uint32 GetUnlockedPetBattleSlot();
        /// Summon current pet if any active
        void UnsummonCurrentBattlePetIfAny(bool p_Unvolontary);
        /// Summon new pet
        void SummonBattlePet(uint64 p_JournalID);
        /// Get current summoned battle pet
        Creature * GetSummonedBattlePet();
        /// Summon last summoned battle pet
        void SummonLastSummonedBattlePet();

        /// Get pet battles
        std::vector<std::shared_ptr<BattlePet>> GetBattlePets();
        /// Get pet battles
        std::shared_ptr<BattlePet> GetBattlePet(uint64 p_JournalID);
        /// Get pet battle combat team
        std::shared_ptr<BattlePet> * GetBattlePetCombatTeam();
        /// Get pet battle combat team size
        uint32 GetBattlePetCombatSize();

        /// Reload pet battles
        void ReloadPetBattles();
        /// PetBattleCountBattleSpeciesCallback
        void PetBattleCountBattleSpecies();
        /// Update battle pet combat team
        void UpdateBattlePetCombatTeam();

        //////////////////////////////////////////////////////////////////////////
        /// ToyBox
        void _LoadToyBox(PreparedQueryResult p_Result);
        void SendToyBox();
        void AddNewToyToBox(uint32 p_ItemID);
        void SetFavoriteToy(bool p_Apply, uint32 p_ItemID);

        PlayerToy* GetToy(uint32 p_ItemID)
        {
            if (m_PlayerToys.find(p_ItemID) != m_PlayerToys.end())
                return &m_PlayerToys[p_ItemID];

            return nullptr;
        }

        bool HasToy(uint32 p_ItemID) const
        {
            if (m_PlayerToys.find(p_ItemID) != m_PlayerToys.end())
                return true;

            return false;
        }
        //////////////////////////////////////////////////////////////////////////

        uint32 GetEquipItemLevelFor(ItemTemplate const* itemProto, Item const* item = nullptr) const;
        void RescaleItemTo(uint8 slot, uint32 ilvl);
        void RescaleAllItemsIfNeeded(bool p_KeepHPPct = false);
        bool UpdateItemLevelCutOff(uint32 p_StartsWith, uint32 p_MinLevel, uint32 p_MaxLevel, bool p_RescaleItems = true);
        void CutOffItemLevel(bool p_RescaleItems = true);

        void SetInPvPCombat(bool set);
        bool IsInPvPCombat() const { return m_pvpCombat; }
        void UpdatePvP(uint32 diff);
        void SetPvPTimer(uint32 duration) { m_PvPCombatTimer = duration; }

        uint32 GetQuestObjectiveCounter(uint32 objectiveId) const;

        //////////////////////////////////////////////////////////////////////////
        /// SpellCharges
        SpellChargesMap m_SpellChargesMap;

        void SendSpellCharges();
        void SendClearAllSpellCharges();
        void SendSetSpellCharges(uint32 p_SpellID);
        void SendClearSpellCharges(uint32 p_SpellID);

        bool CanUseCharge(uint32 p_SpellID) const;
        void UpdateCharges(uint32 const p_Time);
        void ConsumeCharge(uint32 p_SpellID, SpellCategoryEntry const* p_Category, bool p_SendPacket = false);
        ChargesData* GetChargesData(uint32 p_SpellID);
        //////////////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////////////
        /// ChallengesMode

        void _LoadCompletedChallenges(PreparedQueryResult&& p_Result);

        bool HasChallengeCompleted(uint32 p_MapID) const;
        CompletedChallenge* GetCompletedChallenge(uint32 p_MapID);

        CompletedChallengesMap m_CompletedChallenges;
        //////////////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////////////
        /// Vignette
        //////////////////////////////////////////////////////////////////////////

        Vignette::Manager const& GetVignetteMgr() { return m_VignetteMgr; }

        /////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////////////////

        struct MovieDelayedTeleport
        {
            uint32 MovieID;
            uint32 MapID;
            float X, Y, Z, O;
        };

        uint32 CurrentPlayedMovie;
        std::vector<MovieDelayedTeleport> MovieDelayedTeleports;
        std::mutex MovieDelayedTeleportMutex;

        void AddMovieDelayedTeleport(uint32 p_MovieID, uint32 p_MapID, float p_X, float p_Y, float p_Z, float p_O)
        {
            MovieDelayedTeleport l_Data;
            l_Data.MovieID  = p_MovieID;
            l_Data.MapID    = p_MapID;
            l_Data.X        = p_X;
            l_Data.Y        = p_Y;
            l_Data.Z        = p_Z;
            l_Data.O        = p_O;

            MovieDelayedTeleportMutex.lock();
            MovieDelayedTeleports.push_back(l_Data);
            MovieDelayedTeleportMutex.unlock();
        }

        DailyLootsCooldowns m_DailyLootsCooldowns;
        void _LoadDailyLootsCooldowns(PreparedQueryResult&& p_Result);
        void ResetDailyLoots();
        bool CanHaveDailyLootForItem(uint32 p_Entry) const { return m_DailyLootsCooldowns.find(p_Entry) == m_DailyLootsCooldowns.end(); }
        void AddDailyLootCooldown(uint32 p_Entry);

        void _GarrisonSetIn();
        void _GarrisonSetOut();

        void AddCriticalOperation(std::function<void()> const&& p_Function)
        {
            m_CriticalOperationLock.acquire();
            m_CriticalOperation.push(std::function<void()>(p_Function));
            m_CriticalOperationLock.release();
        }

    protected:
        void OnEnterPvPCombat();
        void OnLeavePvPCombat();

        /// Load pet battle async callback
        bool _LoadPetBattles(PreparedQueryResult&& p_Result);

        uint64 m_BattlePetSummon;
        uint32 m_LastSummonedBattlePet;

        std::vector<std::shared_ptr<BattlePet>> m_BattlePets;
        std::shared_ptr<BattlePet> m_BattlePetCombatTeam[3];
        std::vector<std::pair<uint32, uint32>> m_OldPetBattleSpellToMerge;

        PreparedQueryResultFuture _petBattleJournalCallback;

        PlayerToys m_PlayerToys;

    private:
        // Gamemaster whisper whitelist
        WhisperListContainer WhisperList;
        uint32 m_regenTimerCount;
        uint32 m_holyPowerRegenTimerCount;
        uint32 m_chiPowerRegenTimerCount;
        uint32 m_soulShardsRegenTimerCount;
        uint32 m_focusRegenTimerCount;
        uint32 m_demonicFuryPowerRegenTimerCount;
        float m_powerFraction[MAX_POWERS_PER_CLASS];
        uint32 m_contestedPvPTimer;

        std::set<uint64> m_ActiveMovementForces;

        /*********************************************************/
        /***               BATTLEGROUND SYSTEM                 ***/
        /*********************************************************/

        /*
        this is an array of BG queues (BgTypeIDs) in which is player
        */
        struct BgBattlegroundQueueID_Rec
        {
            MS::Battlegrounds::BattlegroundType::Type BgType;
            uint32 invitedToInstance;
        };

        BgBattlegroundQueueID_Rec m_bgBattlegroundQueueID[PLAYER_MAX_BATTLEGROUND_QUEUES];
        BGData                    m_bgData;

        bool m_IsBGRandomWinner;

        /*********************************************************/
        /***                    QUEST SYSTEM                   ***/
        /*********************************************************/

        //We allow only one timed quest active at the same time. Below can then be simple value instead of set.
        typedef std::set<uint32> QuestSet;
        typedef std::set<uint32> SeasonalQuestSet;
        typedef UNORDERED_MAP<uint32,SeasonalQuestSet> SeasonalEventQuestMap;
        QuestSet m_timedquests;
        QuestSet m_weeklyquests;
        QuestSet m_monthlyquests;
        SeasonalEventQuestMap m_seasonalquests;

        uint64 m_divider;
        uint32 m_ingametime;

        /*********************************************************/
        /***                   LOAD SYSTEM                     ***/
        /*********************************************************/

        void _LoadActions(PreparedQueryResult result);
        void _LoadAuras(PreparedQueryResult result, PreparedQueryResult resultEffect, uint32 timediff);
        void _LoadGlyphAuras();
        void _LoadBoundInstances(PreparedQueryResult result);
        void _LoadInventory(PreparedQueryResult result, uint32 timeDiff);
        void _LoadVoidStorage(PreparedQueryResult result);
        void _LoadMailInit(PreparedQueryResult resultUnread, PreparedQueryResult resultDelivery);
        void _LoadMail();
        void _LoadMailedItems(Mail* mail);
        void _LoadQuestStatus(PreparedQueryResult result);
        void _LoadQuestObjectiveStatus(PreparedQueryResult result);
        void _LoadQuestStatusRewarded(PreparedQueryResult result);
        void _LoadDailyQuestStatus(PreparedQueryResult result);
        void _LoadWeeklyQuestStatus(PreparedQueryResult result);
        void _LoadMonthlyQuestStatus(PreparedQueryResult result);
        void _LoadSeasonalQuestStatus(PreparedQueryResult result);
        void _LoadRandomBGStatus(PreparedQueryResult result);
        void _LoadGroup(PreparedQueryResult result);
        void _LoadSkills(PreparedQueryResult result);
        void _LoadSpells(PreparedQueryResult result);
        void _LoadFriendList(PreparedQueryResult result);
        bool _LoadHomeBind(PreparedQueryResult result);
        void _LoadDeclinedNames(PreparedQueryResult result);
        void _LoadEquipmentSets(PreparedQueryResult result);
        void _LoadArenaData(PreparedQueryResult result);
        void _LoadBGData(PreparedQueryResult result);
        void _LoadGlyphs(PreparedQueryResult result);
        void _LoadTalents(PreparedQueryResult result);
        void _LoadInstanceTimeRestrictions(PreparedQueryResult result);
        void _LoadCUFProfiles(PreparedQueryResult result);
        void _LoadCurrency(PreparedQueryResult result);

        /*********************************************************/
        /***                   SAVE SYSTEM                     ***/
        /*********************************************************/

        void _SaveActions(SQLTransaction& trans);
        void _SaveAuras(SQLTransaction& trans);
        void _SaveInventory(SQLTransaction& trans);
        void _SaveVoidStorage(SQLTransaction& trans);
        void _SaveMail(SQLTransaction& trans);
        void _SaveQuestStatus(SQLTransaction& trans);
        void _SaveQuestObjectiveStatus(SQLTransaction& trans);
        void _SaveDailyQuestStatus(SQLTransaction& trans);
        void _SaveWeeklyQuestStatus(SQLTransaction& trans);
        void _SaveMonthlyQuestStatus(SQLTransaction& trans);
        void _SaveSeasonalQuestStatus(SQLTransaction& trans);
        void _SaveSkills(SQLTransaction& trans);
        void _SaveSpells(SQLTransaction& charTrans, SQLTransaction& accountTrans);
        void _SaveEquipmentSets(SQLTransaction& trans);
        void _SaveArenaData(SQLTransaction& trans);
        void _SaveBGData(SQLTransaction& trans);
        void _SaveGlyphs(SQLTransaction& trans);
        void _SaveTalents(SQLTransaction& trans);
        void _SaveStats(SQLTransaction& trans);
        void _SaveInstanceTimeRestrictions(SQLTransaction& trans);
        void _SaveCurrency(SQLTransaction& trans);

        /*********************************************************/
        /***              ENVIRONMENTAL SYSTEM                 ***/
        /*********************************************************/
        void HandleSobering();
        void SendMirrorTimer(MirrorTimerType Type, uint32 MaxValue, uint32 CurrentValue, int32 Regen);
        void StopMirrorTimer(MirrorTimerType Type);
        void HandleDrowning(uint32 time_diff);
        int32 getMaxTimer(MirrorTimerType timer);

        /*********************************************************/
        /***                  HONOR SYSTEM                     ***/
        /*********************************************************/
        time_t m_lastHonorUpdateTime;

        void outDebugValues() const;
        uint64 m_lootGuid;

        uint32 m_lootSpecId;
        uint32 m_BonusRollFails;

        uint32 m_team;
        uint32 m_nextSave;
        time_t m_speakTime;
        uint32 m_speakCount;
        time_t m_pmChatTime;
        uint32 m_pmChatCount;
        Difficulty m_dungeonDifficulty;
        Difficulty m_raidDifficulty;
        Difficulty m_LegacyRaidDifficulty;
        Difficulty m_raidMapDifficulty;

        uint32 m_atLoginFlags;

        Item* m_items[PLAYER_SLOTS_COUNT];
        uint32 m_itemScale[INVENTORY_SLOT_BAG_END];

        uint32 m_currentBuybackSlot;

        PlayerCurrenciesMap _currencyStorage;

        VoidStorageItem* _voidStorageItems[VOID_STORAGE_MAX_SLOT];

        std::vector<Item*> m_itemUpdateQueue;
        bool m_itemUpdateQueueBlocked;

        uint32 m_ExtraFlags;
        uint64 m_curSelection;

        QuestStatusMap m_QuestStatus;
        QuestObjectiveStatusMap m_questObjectiveStatus;
        QuestStatusSaveMap m_QuestStatusSave;

        RewardedQuestSet m_RewardedQuests;
        QuestStatusSaveMap m_RewardedQuestsSave;

        SkillStatusMap mSkillStatus;

        uint32 m_GuildIdInvited;

        PlayerMails m_mail;
        PlayerSpellMap m_spells;
        uint32 m_lastPotionId;                              // last used health/mana potion in combat, that block next potion use

        GlobalCooldownMgr m_GlobalCooldownMgr;
        struct prohibited_struct
        {
            prohibited_struct(uint32 _time): m_time_prohibited_until(getMSTime() + _time) { }
            prohibited_struct(): m_time_prohibited_until(0) { }
            uint32 m_time_prohibited_until;
        };

        prohibited_struct prohibited[MAX_SPELL_SCHOOL];

        PlayerTalentInfo* _talentMgr;

        ActionButtonList m_actionButtons;

        float m_auraBaseMod[BASEMOD_END][MOD_END];
        int16 m_baseRatingValue[MAX_COMBAT_RATING];
        uint32 m_baseSpellPower;
        uint32 m_baseManaRegen;
        uint32 m_baseHealthRegen;
        int32 m_spellPenetrationItemMod;

        SpellModList m_spellMods[MAX_SPELLMOD];
        //uint32 m_pad;
//        Spell* m_spellModTakingSpell;  // Spell for which charges are dropped in spell::finish

        EnchantDurationList m_enchantDuration;
        ItemDurationList m_itemDuration;
        ItemDurationList m_itemSoulboundTradeable;

        void ResetTimeSync();
        void SendTimeSync();

        ResurrectionData* _resurrectionData;

        WorldSession* m_session;

        typedef std::list<Channel*> JoinedChannelsList;
        JoinedChannelsList m_channels;

        uint8 m_cinematic;

        TradeData* m_trade;
        uint32 m_ClientStateIndex;

        bool   m_DailyQuestChanged;
        bool   m_WeeklyQuestChanged;
        bool   m_MonthlyQuestChanged;
        bool   m_SeasonalQuestChanged;
        time_t m_lastDailyQuestTime;

        uint32 m_drunkTimer;
        uint32 m_weaponChangeTimer;

        uint32 m_zoneUpdateId;
        uint32 m_zoneUpdateTimer;
        uint32 m_areaUpdateId;

        uint32 m_deathTimer;
        time_t m_deathExpireTime;

        uint32 m_restTime;

        uint32 m_WeaponProficiency;
        uint32 m_ArmorProficiency;
        bool m_canParry;
        bool m_canBlock;
        bool m_canTitanGrip;
        uint8 m_swingErrorMsg;

        bool m_needSummonPetAfterStopFlying;

        ////////////////////Rest System/////////////////////
        time_t time_inn_enter;
        uint32 inn_pos_mapid;
        float  inn_pos_x;
        float  inn_pos_y;
        float  inn_pos_z;
        float m_rest_bonus;
        RestType rest_type;
        ////////////////////Rest System/////////////////////

        // Social
        PlayerSocial *m_social;

        // Groups
        GroupReference m_group;
        GroupReference m_originalGroup;
        uint32 m_groupInviteGUID;
        uint32 m_groupUpdateMask;
        uint64 m_auraRaidUpdateMask;
        bool m_bPassOnGroupLoot;

        // last used pet number (for BG's)
        uint32 m_lastpetnumber;

        // Player summoning
        time_t m_summon_expire;
        uint32 m_summon_mapid;
        float  m_summon_x;
        float  m_summon_y;
        float  m_summon_z;

        DeclinedName *m_declinedname;
        Runes m_runes;
        EquipmentSets m_EquipmentSets;

        bool CanAlwaysSee(WorldObject const* obj) const;

        bool IsAlwaysDetectableFor(WorldObject const* seer) const;

        uint8 m_grantableLevels;

        uint32 m_tokenCounter;

        typedef std::set<uint32> DailyQuestList;
        DailyQuestList m_dailyQuestStorage;

        MS::Utilities::BitSet m_CompletedQuestBits;

        std::queue<std::function<void()>> m_CriticalOperation;
        ACE_Thread_Mutex m_CriticalOperationLock;

    private:
        // internal common parts for CanStore/StoreItem functions
        InventoryResult CanStoreItem_InSpecificSlot(uint8 bag, uint8 slot, ItemPosCountVec& dest, ItemTemplate const* pProto, uint32& count, bool swap, Item* pSrcItem) const;
        InventoryResult CanStoreItem_InBag(uint8 bag, ItemPosCountVec& dest, ItemTemplate const* pProto, uint32& count, bool merge, bool non_specialized, Item* pSrcItem, uint8 skip_bag, uint8 skip_slot) const;
        InventoryResult CanStoreItem_InInventorySlots(uint8 slot_begin, uint8 slot_end, ItemPosCountVec& dest, ItemTemplate const* pProto, uint32& count, bool merge, Item* pSrcItem, uint8 skip_bag, uint8 skip_slot) const;
        Item* _StoreItem(uint16 pos, Item* pItem, uint32 count, bool clone, bool update);
        Item* _LoadItem(SQLTransaction& trans, uint32 zoneId, uint32 timeDiff, Field* fields);

        std::set<uint32> m_refundableItems;
        void SendRefundInfo(Item* item);
        void RefundItem(Item* item);
        void SendItemRefundResult(Item* item, ItemExtendedCostEntry const* iece, uint8 error);

        // know currencies are not removed at any point (0 displayed)
        void AddKnownCurrency(uint32 itemId);

        int32 CalculateReputationGain(uint32 creatureOrQuestLevel, int32 rep, int32 faction, bool for_quest, bool noQuestBonus = false);
        void AdjustQuestReqItemCount(Quest const* quest);

        bool IsCanDelayTeleport() const { return m_bCanDelayTeleport; }
        void SetCanDelayTeleport(bool setting) { m_bCanDelayTeleport = setting; }
        bool IsHasDelayedTeleport() const { return m_bHasDelayedTeleport; }
        void SetDelayedTeleportFlag(bool setting) { m_bHasDelayedTeleport = setting; }

        void ScheduleDelayedOperation(uint32 operation)
        {
            if (operation < DELAYED_END)
                m_DelayedOperations |= operation;
        }

        MapReference m_mapRef;

        void UpdateCharmedAI();

        uint32 m_lastFallTime;
        float  m_lastFallZ;

        int32 m_MirrorTimer[MAX_TIMERS];
        uint8 m_MirrorTimerFlags;
        uint8 m_MirrorTimerFlagsLast;
        bool m_isInWater;

        // Current teleport data
        WorldLocation m_teleport_dest;
        uint32 m_teleport_options;
        bool mSemaphoreTeleport_Near;
        bool mSemaphoreTeleport_Far;

        uint32 m_DelayedOperations;
        bool m_bCanDelayTeleport;
        bool m_bHasDelayedTeleport;
        bool m_isMoltenCored;

        // Temporary removed pet cache
        uint32 m_temporaryUnsummonedPetNumber;
        uint32 m_oldpetspell;

        AchievementMgr<Player> m_achievementMgr;
        ReputationMgr  m_reputationMgr;
        BattlePetMgr   m_battlePetMgr;

        SpellCooldowns m_spellCooldowns;

        uint32 m_ChampioningFaction;

        uint32 m_timeSyncCounter;
        uint32 m_timeSyncTimer;
        uint32 m_timeSyncClient;
        uint32 m_timeSyncServer;

        InstanceTimeMap _instanceResetTimes;
        uint32 _pendingBindId;
        uint32 _pendingBindTimer;

        uint32 _activeCheats;
        uint32 _maxPersonalArenaRate;

        PhaseMgr phaseMgr;

        uint32 _lastTargetedGO;
        float m_PersonnalXpRate;

        //////////////////////////////////////////////////////////////////////////
        /// Garrison
        //////////////////////////////////////////////////////////////////////////
        MS::Garrison::Manager * m_Garrison;
        IntervalTimer m_GarrisonUpdateTimer;

        //////////////////////////////////////////////////////////////////////////
        /// Cinematic
        //////////////////////////////////////////////////////////////////////////
        CinematicSequence * m_CinematicSequence;
        bool                m_InCinematic;
        uint32              m_CinematicClientStartTime;
        float               m_CinematicStartX, m_CinematicStartY, m_CinematicStartZ, m_CinematicStartO;
        bool                m_FlyingBeforeCinematic;

        uint32 m_knockBackTimer;
        uint8  m_ignoreMovementCount;

        uint32 m_groupUpdateDelay;

        bool m_initializeCallback;
        uint8 m_storeCallbackCounter;

        uint32 m_emote;

        uint32 m_lastPlayedEmote;

        ArchaeologyMgr m_archaeologyMgr;

        // Store callback
        PreparedQueryResultFuture _storeGoldCallback;
        PreparedQueryResultFuture _storeTitleCallback;
        PreparedQueryResultFuture _storeItemCallback;
        PreparedQueryResultFuture _storeLevelCallback;
        PreparedQueryResultFuture _petPreloadCallback;
        QueryResultHolderFuture _petLoginCallback;

        uint8 m_bgRoles;

        // Arena
        uint32 m_ArenaPersonalRating[MAX_PVP_SLOT];
        uint32 m_BestRatingOfWeek[MAX_PVP_SLOT];
        uint32 m_BestRatingOfSeason[MAX_PVP_SLOT];
        uint32 m_ArenaMatchMakerRating[MAX_PVP_SLOT];
        uint32 m_WeekWins[MAX_PVP_SLOT];
        uint32 m_PrevWeekWins[MAX_PVP_SLOT];
        uint32 m_SeasonWins[MAX_PVP_SLOT];
        uint32 m_WeekGames[MAX_PVP_SLOT];
        uint32 m_SeasonGames[MAX_PVP_SLOT];

        CUFProfiles m_cufProfiles;

        /*********************************************************/
        /***                  SCENES SYSTEM                    ***/
        /*********************************************************/
        SceneObject* m_LastPlayedScene;

        uint32 m_PvPCombatTimer;
        bool m_pvpCombat;

        //////////////////////////////////////////////////////////////////////////
        /// Vignette
        //////////////////////////////////////////////////////////////////////////
        Vignette::Manager m_VignetteMgr;
};

void AddItemsSetItem(Player*player, Item* item);
void RemoveItemsSetItem(Player*player, ItemTemplate const* proto);

// "the bodies of template functions must be made available in a header file"
template <class T> T Player::ApplySpellMod(uint32 spellId, SpellModOp op, T &basevalue, Spell* spell, bool removestacks)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return 0;

    float totalmul = 1.0f;
    int32 totalflat = 0;
    bool chaosBolt = false;
    bool soulFire = false;
    bool pyroblast = false;
    int32 value = 0;

    // Drop charges for triggering spells instead of triggered ones
    if (m_spellModTakingSpell)
        spell = m_spellModTakingSpell;

    int32 playerWeaponMask = 0;

    for (uint8 i = 0; i < WeaponAttackType::MaxAttack; ++i)
    {
        Item* tmpitem = GetWeaponForAttack(WeaponAttackType(i), true);
        if (!tmpitem || tmpitem->IsBroken() || !tmpitem->GetTemplate())
            continue;

        playerWeaponMask |= 1 << tmpitem->GetTemplate()->SubClass;
    }

    for (SpellModList::iterator itr = m_spellMods[op].begin(); itr != m_spellMods[op].end(); ++itr)
    {
        SpellModifier* mod = *itr;

        // Charges can be set only for mods with auras
        if (!mod->ownerAura)
            ASSERT(mod->charges == 0);

        if (!IsAffectedBySpellmod(spellInfo, mod, spell))
            continue;

        int32 spellWeaponMask = 0;
        if (mod->ownerAura)
        {
            const auto temp = mod->ownerAura->GetSpellInfo();
            if (temp)
                spellWeaponMask = temp->EquippedItemSubClassMask;
        }

        if (spellWeaponMask > 0 && playerWeaponMask > 0)
            if (!(playerWeaponMask & spellWeaponMask))
                continue;

        if (mod->type == SPELLMOD_FLAT)
            totalflat += mod->value;
        else if (mod->type == SPELLMOD_PCT)
        {
            // skip percent mods for null basevalue (most important for spell mods with charges)
            if (basevalue == T(0))
                continue;

            // special case (skip > 10sec spell casts for instant cast setting)
            if (mod->op == SPELLMOD_CASTING_TIME && basevalue >= T(10000) && mod->value <= -100)
                continue;

            value = mod->value;

            // Fix don't apply Backdraft twice for Chaos Bolt
            if (mod->spellId == 117828 && mod->op == SPELLMOD_CASTING_TIME && spellInfo->Id == 116858)
            {
                if (chaosBolt)
                    continue;
                else
                    chaosBolt = true;
            }
            // Fix don't apply Molten Core multiple times for Soul Fire
            else if (mod->spellId == 122355 && (spellInfo->Id == 6353 || spellInfo->Id == 104027))
            {
                if (soulFire)
                    continue;
                else
                    soulFire = true;

                if (m_isMoltenCored)
                    m_isMoltenCored = false;
                else if (mod->op == SPELLMOD_CASTING_TIME || mod->op == SPELLMOD_COST)
                    m_isMoltenCored = true;

                value = mod->value / mod->charges;
            }
            // Fix don't apply Pyroblast! and Presence of Mind at the same time for Pyroblast
            else if ((mod->spellId == 48108 || mod->spellId == 12043) && mod->op == SPELLMOD_CASTING_TIME && spellInfo->Id == 11366)
            {
                if (pyroblast)
                    continue;
                else
                    pyroblast = true;
            }

            if (value > 0)
                totalmul = CalculatePct(totalmul, 100 + value);
            else
                totalmul = CalculatePct(totalmul, 100 + value);
        }

        if (removestacks && !m_isMoltenCored && spell)
            DropModCharge(mod, spell);
    }

    float diff = (float)basevalue * (totalmul - 1.0f) + (float)totalflat;
    basevalue = T((float)basevalue + diff);
    return T(diff);
}
#endif
