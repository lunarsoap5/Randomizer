/** @file game_patch.h
 * @author AECX
 * @author Lunar Soap
 * @brief Game patches are always applied & mostly used to fix a bug/softlog.
 *
 * @bug No known bugs
 */
#ifndef RANDO_GAME_PATCH_H
#define RANDO_GAME_PATCH_H

#include <cstdint>

#include "rando/randomizer.h"
#include "tp/control.h"
#include "tp/d_msg_flow.h"
#include "data/stages.h"
#include "data/items.h"
#include "gc_wii/OSCache.h"

namespace mod::game_patch
{
    // Special message ids that use values that are normally never used
    enum SpecialMessageIds
    {
        FOR = 10000,
        THE,
        SMALL_KEY,
        DUNGEON_MAP,
        COMPASS,
        BIG_KEY,
        FOREST_TEMPLE,
        GORON_MINES,
        LAKEBED_TEMPLE,
        ARBITERS_GROUNDS,
        SNOWPEAK_RUINS,
        TEMPLE_OF_TIME,
        CITY_IN_THE_SKY,
        PALACE_OF_TWILIGHT,
        HYRULE_CASTLE,
        BULBLIN_CAMP,
    };

    // 00 - Function definitions used to patch how Poes operate
    /**
     * @brief Patches the unused itemFunc corresponding to the poe itemID
     */
    void _00_poe();
    void _00_handle_poeItem();

    // 01 - Function definitions used for setting up the custom state function(s)
    int32_t _01_getLayerNo(const char* stageName, int32_t roomId, int32_t layerOverride);

    // 02 - Function definitions for custom items and patched vanilla items
    void _02_modifyItemData();
    void _02_modifyFoolishFieldModel();
    void _02_modifyFoolishShopModel(uint8_t* foolishModelIndexes, uint32_t loopCurrentCount, uint32_t shopID);
    void _02_modifyShopModelScale(uint32_t shopID, uint32_t itemID);
    void giveNodeDungeonItems(const libtp::data::stage::AreaNodesID nodeId, const libtp::data::items::NodeDungeonItemType type);
    void handleTradeItemFunc();

    extern const char* _02_hiddenSkillArc;
    extern const char* _02_mirrorShardArc;
    extern const char* _02_firstShadowArc;
    extern const char* _02_secondShadowArc;
    extern const char* _02_thirdShadowArc;
    extern const char* _02_masterSwordArc;

    void _02_forestSmallKeyItemFunc();
    void _02_forestMapItemFunc();
    void _02_forestCompassItemFunc();
    void _02_forestBigKeyItemFunc();
    void _02_minesSmallKeyItemFunc();
    void _02_minesMapItemFunc();
    void _02_minesCompassItemFunc();
    void _02_minesBigKeyItemFunc();
    void _02_lakebedSmallKeyItemFunc();
    void _02_lakebedMapItemFunc();
    void _02_lakebedCompassItemFunc();
    void _02_lakebedBigKeyItemFunc();
    void _02_arbitersSmallKeyItemFunc();
    void _02_arbitersMapItemFunc();
    void _02_arbitersCompassItemFunc();
    void _02_arbitersBigKeyItemFunc();
    void _02_snowpeakSmallKeyItemFunc();
    void _02_snowpeakMapItemFunc();
    void _02_snowpeakCompassItemFunc();
    void _02_snowpeakBigKeyItemFunc();
    void _02_totSmallKeyItemFunc();
    void _02_totMapItemFunc();
    void _02_totCompassItemFunc();
    void _02_totBigKeyItemFunc();
    void _02_citySmallKeyItemFunc();
    void _02_cityMapItemFunc();
    void _02_cityCompassItemFunc();
    void _02_cityBigKeyItemFunc();
    void _02_palaceSmallKeyItemFunc();
    void _02_palaceMapItemFunc();
    void _02_palaceCompassItemFunc();
    void _02_palaceBigKeyItemFunc();
    void _02_hyruleSmallKeyItemFunc();
    void _02_hyruleMapItemFunc();
    void _02_hyruleCompassItemFunc();
    void _02_hyruleBigKeyItemFunc();
    void _02_campSmallKeyItemFunc();
    void _02_faronSmallKeyItemFunc();
    void _02_faronCoroKeyItemFunc();
    void _02_shadowCrystalItemFunc();
    void _02_poweredDominionRodItemFunc();
    void _02_auruMemoItemFunc();
    void _02_ordonPumpkinItemFunc();
    void _02_ordonGoatCheeseItemFunc();
    void _02_partlyFilledSkybookItemFunc();
    void _02_filledSkybookItemFunc();
    void _02_bigWalletItemFunc();
    void _02_giantWalletItemFunc();
    void _02_gateKeysItemFunc();
    void _02_firstFusedShadowItemFunc();
    void _02_secondFusedShadowItemFunc();
    void _02_thirdFusedShadowItemFunc();
    void _02_firstMirrorShardItemFunc();
    void _02_secondMirrorShardItemFunc();
    void _02_thirdMirrorShardItemFunc();
    void _02_fourthMirrorShardItemFunc();
    void _02_endingBlowItemFunc();
    void _02_shieldAttackItemFunc();
    void _02_backSliceItemFunc();
    void _02_helmSplitterItemFunc();
    void _02_mortalDrawItemFunc();
    void _02_jumpStrikeItemFunc();
    void _02_greatSpinItemFunc();
    void _02_lanayruVesselItemFunc();
    void _02_foolishItemFunc();
    void _02_SFaronPortalItemFunc();
    void _02_NFaronPortalItemFunc();
    void _02_GorgePortalItemFunc();
    void _02_KakVillagePortalItemFunc();
    void _02_DeathMountainPortalItemFunc();
    void _02_CastleTownPortalItemFunc();
    void _02_ZorasDomainPortalItemFunc();
    void _02_LakeHyliaPortalItemFunc();
    void _02_GerudoDesertPortalItemFunc();
    void _02_MirrorChamberPortalItemFunc();
    void _02_SnowpeakPortalItemFunc();
    void _02_SacredGrovePortalItemFunc();
    void _02_EldinBridgePortalItemFunc();
    void _02_UpperZoraRiverPortalItemFunc();
    void _02_renadosLetterItemFunc();
    void _02_invoiceItemFunc();
    void _02_woodenStatueItemFunc();
    void _02_iliasCharmItemFunc();
    void _02_horseCallItemFunc();
    int32_t _02_bigWalletItemGetCheck();
    int32_t _02_giantWalletItemGetCheck();
    int32_t _02_firstFusedShadowItemGetCheck();
    int32_t _02_secondFusedShadowItemGetCheck();
    int32_t _02_thirdFusedShadowItemGetCheck();
    int32_t _02_firstMirrorShardItemGetCheck();
    int32_t _02_secondMirrorShardItemGetCheck();
    int32_t _02_thirdMirrorShardItemGetCheck();
    int32_t _02_fourthMirrorShardItemGetCheck();
    int32_t _02_shadowCrystalItemGetCheck();
    int32_t _02_endingBlowItemGetCheck();
    int32_t _02_shieldAttackItemGetCheck();
    int32_t _02_backSliceItemGetCheck();
    int32_t _02_helmSplitterItemGetCheck();
    int32_t _02_mortalDrawItemGetCheck();
    int32_t _02_jumpStrikeItemGetCheck();
    int32_t _02_campSmallKeyItemGetCheck();
    int32_t _02_gateKeysItemGetCheck();
    int32_t _02_letterItemGetCheck();
    int32_t _02_invoiceItemGetCheck();
    int32_t _02_statueItemGetCheck();
    int32_t _02_charmItemGetCheck();
    int32_t _02_horseCallItemGetCheck();

    // 03 - Function definitions to modify different values for Link's actions
    void _03_increaseClimbSpeed();

    // 04 - Function definitions to patch out progressive items
    uint32_t _04_verifyProgressiveItem(rando::Randomizer* randomizer, uint32_t itemID);
    uint32_t _04_getProgressiveSword();
    uint32_t _04_getProgressiveBow();
    uint32_t _04_getProgressiveSkill();
    uint32_t _04_getProgressiveSkyBook();
    uint32_t _04_getProgressiveKeyShard();
    uint32_t _04_getProgressiveMirrorShard();
    uint32_t _04_getProgressiveFusedShadow();

    // 05 - Function definitions related to custom messages
    void _05_setCustomItemMessage(libtp::tp::control::TControl* control, const void* TProcessor, uint16_t unk3, uint16_t msgId);

    void* _05_getZel00BmgInf();
    uint32_t _05_getCustomMsgColor(uint8_t colorId);
    const char* _05_getMsgById(uint32_t msgId);
    const char* _05_getMsgById(uint32_t msgId, uint16_t* msgSizeOut);
    const char* _05_getSpecialMsgById(uint32_t msgId);
    const char** _05_replaceMessageString(const char** text);

    // int _05_customEvent067(libtp::tp::d_msg_flow::dMsgFlow* msgFlow,
    //                        void* flowNode,
    //                        libtp::tp::f_op_actor::fopAc_ac_c* actorPtr);
    int _05_customEvent067(libtp::tp::d_msg_flow::dMsgFlow* msgFlow,
                           void* flowNode,
                           libtp::tp::f_op_actor::fopAc_ac_c* actorPtr);
    int _05_customEvent068(libtp::tp::d_msg_flow::dMsgFlow* msgFlow,
                           void* flowNode,
                           libtp::tp::f_op_actor::fopAc_ac_c* actorPtr);

    // extern int (*customEventFunctions[])(libtp::tp::d_msg_flow::dMsgFlow* msgFlow,
    //                                      void* flowNode,
    //                                      libtp::tp::f_op_actor::fopAc_ac_c* actorPtr);

    extern uint32_t customQueryFunctions[2][3];
    extern uint32_t customEventFunctions[3][3];

    // 06 - Function definitions for assembly patches
    void _06_writeASMPatches();

    // 07 - Function definitions for modifying where the player spawns in specific circumstances.
    void _07_checkPlayerStageReturn();
} // namespace mod::game_patch
#endif