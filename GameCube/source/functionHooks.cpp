#include "functionHooks.h"
#include "main.h"
#include "tp/dynamic_link.h"
#include "tp/dzx.h"
#include "tp/d_stage.h"
#include "tp/f_op_actor.h"
#include "tp/d_save.h"
#include "tp/control.h"
#include "tp/d_msg_flow.h"
#include "tp/d_a_alink.h"
#include "Z2AudioLib/Z2SceneMgr.h"
#include "Z2AudioLib/Z2SeqMgr.h"
#include "tp/d_resource.h"
#include "tp/m_Do_dvd_thread.h"
#include "tp/d_meter2_info.h"
#include "tp/d_gameover.h"
#include "tp/d_shop_system.h"
#include "tp/d_file_select.h"

namespace mod
{
    // Function hook return trampolines
    KEEP_VAR void (*gReturn_fapGm_Execute)() = nullptr;

    // DMC (REL) Hook
    KEEP_VAR bool (*gReturn_do_unlink)(libtp::tp::dynamic_link::DynamicModuleControl* dmc) = nullptr;

    // DZX trampolines
    KEEP_VAR bool (*gReturn_actorInit)(void* mStatus_roomControl,
                                       libtp::tp::dzx::ChunkTypeInfo* chunkTypeInfo,
                                       int32_t unk3,
                                       void* unk4) = nullptr;

    KEEP_VAR bool (*gReturn_actorInit_always)(void* mStatus_roomControl,
                                              libtp::tp::dzx::ChunkTypeInfo* chunkTypeInfo,
                                              int32_t unk3,
                                              void* unk4) = nullptr;

    KEEP_VAR bool (*gReturn_actorCommonLayerInit)(void* mStatus_roomControl,
                                                  libtp::tp::dzx::ChunkTypeInfo* chunkTypeInfo,
                                                  int32_t unk3,
                                                  void* unk4) = nullptr;

    KEEP_VAR int32_t (*gReturn_tgscInfoInit)(void* stageDt, void* i_data, int32_t entryNum, void* param_3) = nullptr;
    KEEP_VAR void (*gReturn_roomLoader)(void* data, void* stageDt, int32_t roomNo) = nullptr;
    // KEEP_VAR void ( *gReturn_stageLoader )( void* data, void* stageDt ) = nullptr;

    KEEP_VAR int32_t (*gReturn_dStage_playerInit)(void* stageDt,
                                                  libtp::tp::d_stage::stage_dzr_header_entry* i_data,
                                                  int32_t num,
                                                  void* raw_data) = nullptr;

    KEEP_VAR void (*gReturn_dComIfGp_setNextStage)(const char* stage,
                                                   int16_t point,
                                                   int8_t roomNo,
                                                   int8_t layer,
                                                   float lastSpeed,
                                                   uint32_t lastMode,
                                                   int32_t setPoint,
                                                   int8_t wipe,
                                                   int16_t lastAngle,
                                                   int32_t param_9,
                                                   int32_t wipSpeedT) = nullptr;

    // GetLayerNo trampoline
    KEEP_VAR int32_t (*gReturn_getLayerNo_common_common)(const char* stageName,
                                                         int32_t roomId,
                                                         int32_t layerOverride) = nullptr;

    // Item creation functions.
    KEEP_VAR int32_t (*gReturn_createItemForBoss)(const float pos[3],
                                                  int32_t item,
                                                  int32_t roomNo,
                                                  const int16_t rot[3],
                                                  const float scale[3],
                                                  float unk6,
                                                  float unk7,
                                                  int32_t parameters) = nullptr;

    KEEP_VAR int32_t (*gReturn_createItemForPresentDemo)(const float pos[3],
                                                         int32_t item,
                                                         uint8_t unk3,
                                                         int32_t unk4,
                                                         int32_t unk5,
                                                         const float unk6[3],
                                                         const float unk7[3]) = nullptr;

    KEEP_VAR int32_t (*gReturn_createItemForTrBoxDemo)(const float pos[3],
                                                       int32_t item,
                                                       int32_t itemPickupFlag,
                                                       int32_t roomNo,
                                                       const int16_t rot[3],
                                                       const float scale[3]) = nullptr;

    KEEP_VAR int32_t (*gReturn_createItemForMidBoss)(const float pos[3],
                                                     int32_t item,
                                                     int32_t roomNo,
                                                     const int16_t rot[3],
                                                     const float scale[3],
                                                     int32_t unk6,
                                                     int32_t itemPickupFlag) = nullptr;

    KEEP_VAR void (*gReturn_CheckFieldItemCreateHeap)(libtp::tp::f_op_actor::fopAc_ac_c* actor) = nullptr;

    // Item Wheel trampoline
    KEEP_VAR void (*gReturn_setLineUpItem)(libtp::tp::d_save::dSv_player_item_c*) = nullptr;

    // ItemGet functions.
    KEEP_VAR void (*gReturn_execItemGet)(uint8_t item) = nullptr;
    KEEP_VAR int32_t (*gReturn_checkItemGet)(uint8_t item, int32_t defaultValue) = nullptr;
    KEEP_VAR void (*gReturn_item_func_ASHS_SCRIBBLING)() = nullptr;

    // Message functions
    KEEP_VAR bool (*gReturn_setMessageCode_inSequence)(libtp::tp::control::TControl* control,
                                                       const void* TProcessor,
                                                       uint16_t unk3,
                                                       uint16_t msgId) = nullptr;

    KEEP_VAR uint32_t (*gReturn_getFontCCColorTable)(uint8_t colorId, uint8_t unk) = nullptr;
    KEEP_VAR uint32_t (*gReturn_getFontGCColorTable)(uint8_t colorId, uint8_t unk) = nullptr;

    KEEP_VAR void (*gReturn_jmessage_tSequenceProcessor__do_begin)(void* seqProcessor,
                                                                   const void* unk2,
                                                                   const char* text) = nullptr;

    KEEP_VAR bool (*gReturn_jmessage_tSequenceProcessor__do_tag)(void* seqProcessor,
                                                                 uint32_t unk2,
                                                                 const void* currentText,
                                                                 uint32_t unk4) = nullptr;
    // Query/Event functions.
    KEEP_VAR int32_t (*gReturn_query001)(void* unk1, void* unk2, int32_t unk3) = nullptr;
    KEEP_VAR int32_t (*gReturn_query022)(void* unk1, void* unk2, int32_t unk3) = nullptr;
    KEEP_VAR int32_t (*gReturn_query023)(void* unk1, void* unk2, int32_t unk3) = nullptr;
    KEEP_VAR int32_t (*gReturn_query025)(void* unk1, void* unk2, int32_t unk3) = nullptr;
    KEEP_VAR uint8_t (*gReturn_checkEmptyBottle)(libtp::tp::d_save::dSv_player_item_c* playerItem) = nullptr;
    KEEP_VAR int32_t (*gReturn_query042)(void* unk1, void* unk2, int32_t unk3) = nullptr;
    KEEP_VAR int32_t (*gReturn_query004)(void* unk1, void* unk2, int32_t unk3) = nullptr;
    KEEP_VAR int32_t (*gReturn_query037)(void* unk1, void* unk2, int32_t unk3) = nullptr;
    KEEP_VAR int32_t (*gReturn_query049)(void* unk1, void* unk2, int32_t unk3) = nullptr;
    // KEEP_VAR int32_t ( *gReturn_event000 )( void* messageFlow, void* nodeEvent, void* actrPtr ) = nullptr;
    KEEP_VAR int32_t (*gReturn_event017)(void* messageFlow, void* nodeEvent, void* actrPtr) = nullptr;
    KEEP_VAR int32_t (*gReturn_event003)(void* messageFlow, void* nodeEvent, void* actrPtr) = nullptr;

    KEEP_VAR int32_t (*gReturn_doFlow)(libtp::tp::d_msg_flow::dMsgFlow* msgFlow,
                                       libtp::tp::f_op_actor::fopAc_ac_c* actrPtr,
                                       libtp::tp::f_op_actor::fopAc_ac_c** actrValue,
                                       int32_t i_flow) = nullptr;

    KEEP_VAR int32_t (*gReturn_setNormalMsg)(libtp::tp::d_msg_flow::dMsgFlow* msgFlow,
                                             void* flowNode,
                                             libtp::tp::f_op_actor::fopAc_ac_c* actrPtr) = nullptr;

    // Save flag functions
    KEEP_VAR bool (*gReturn_isDungeonItem)(libtp::tp::d_save::dSv_memBit_c* memBitPtr, const int32_t memBit) = nullptr;
    KEEP_VAR void (*gReturn_onDungeonItem)(libtp::tp::d_save::dSv_memBit_c* memBitPtr, const int32_t memBit) = nullptr;
    KEEP_VAR bool (*gReturn_daNpcT_chkEvtBit)(int16_t flag) = nullptr;
    KEEP_VAR bool (*gReturn_isEventBit)(libtp::tp::d_save::dSv_event_c* eventPtr, uint16_t flag) = nullptr;
    KEEP_VAR void (*gReturn_onEventBit)(libtp::tp::d_save::dSv_event_c* eventPtr, uint16_t flag) = nullptr;
    KEEP_VAR bool (*gReturn_isSwitch_dSv_memBit)(libtp::tp::d_save::dSv_memBit_c* memoryBit, int32_t flag) = nullptr;
    KEEP_VAR void (*gReturn_onSwitch_dSv_memBit)(libtp::tp::d_save::dSv_memBit_c* memoryBit, int32_t flag) = nullptr;
    KEEP_VAR void (*gReturn_onSwitch_dSv_info)(libtp::tp::d_save::dSv_info_c* memoryBit, int32_t flag, int32_t room) = nullptr;
    KEEP_VAR bool (*gReturn_isDarkClearLV)(void* playerStatusPtr, int32_t twilightNode) = nullptr;

    // Pause menu functions
    KEEP_VAR void (*gReturn_collect_save_open_init)(uint8_t param_1) = nullptr;

    // Link functions
    KEEP_VAR bool (*gReturn_checkBootsMoveAnime)(libtp::tp::d_a_alink::daAlink* d_a_alink, int32_t param_1) = nullptr;
    KEEP_VAR bool (*gReturn_checkDamageAction)(libtp::tp::d_a_alink::daAlink* linkMapPtr) = nullptr;
    KEEP_VAR void (*gReturn_setGetItemFace)(libtp::tp::d_a_alink::daAlink* daALink, uint16_t itemID) = nullptr;
    KEEP_VAR void (*gReturn_setWolfLockDomeModel)(libtp::tp::d_a_alink::daAlink* daALink) = nullptr;
    KEEP_VAR bool (*gReturn_procFrontRollCrashInit)(libtp::tp::d_a_alink::daAlink* daALink) = nullptr;
    KEEP_VAR bool (*gReturn_procWolfAttackReverseInit)(libtp::tp::d_a_alink::daAlink* daALink) = nullptr;
    KEEP_VAR bool (*gReturn_procWolfDashReverseInit)(libtp::tp::d_a_alink::daAlink* daALink, bool param_1) = nullptr;
    KEEP_VAR libtp::tp::f_op_actor::fopAc_ac_c* (*gReturn_searchBouDoor)(libtp::tp::f_op_actor::fopAc_ac_c* actrPtr) = nullptr;

    KEEP_VAR float (*gReturn_damageMagnification)(libtp::tp::d_a_alink::daAlink* daALink,
                                                  int32_t param_1,
                                                  int32_t param_2) = nullptr;

    KEEP_VAR bool (*gReturn_checkCastleTownUseItem)(uint16_t item_id) = nullptr;
    KEEP_VAR int32_t (*gReturn_procCoGetItemInit)(libtp::tp::d_a_alink::daAlink* linkActrPtr) = nullptr;

    // Audio functions
    KEEP_VAR void (*gReturn_loadSeWave)(void* Z2SceneMgr, uint32_t waveID) = nullptr;

    KEEP_VAR void (*gReturn_sceneChange)(libtp::z2audiolib::z2scenemgr::Z2SceneMgr* sceneMgr,
                                         libtp::z2audiolib::z2scenemgr::JAISoundID BGMid,
                                         uint8_t SeWave1,
                                         uint8_t SeWave2,
                                         uint8_t BgmWave1,
                                         uint8_t BgmWave2,
                                         uint8_t DemoWave,
                                         bool param_7) = nullptr;

    KEEP_VAR void (*gReturn_startSound)(void* soungMgr,
                                        libtp::z2audiolib::z2scenemgr::JAISoundID soundId,
                                        void* soundHandle,
                                        void* pos) = nullptr;

    KEEP_VAR bool (*gReturn_checkBgmIDPlaying)(libtp::z2audiolib::z2seqmgr::Z2SeqMgr* seqMgr, uint32_t sfx_id) = nullptr;

    // Title Screen functions
    KEEP_VAR void* (*gReturn_dScnLogo_c_dt)(void* dScnLogo_c, int16_t bFreeThis) = nullptr;

    // Archive/resource functions
    KEEP_VAR libtp::tp::d_resource::dRes_info_c* (*gReturn_getResInfo)(const char* arcName,
                                                                       libtp::tp::d_resource::dRes_info_c* objectInfo,
                                                                       int32_t size) = nullptr;

    KEEP_VAR bool (*gReturn_mountArchive__execute)(libtp::tp::m_Do_dvd_thread::mDoDvdThd_mountArchive_c* mountArchive) =
        nullptr;

    // d_meter functions
    KEEP_VAR void (*gReturn_resetMiniGameItem)(libtp::tp::d_meter2_info::G_Meter2_Info* gMeter2InfoPtr,
                                               bool minigameFlag) = nullptr;

    // Game Over functions
    KEEP_VAR void (*gReturn_dispWait_init)(libtp::tp::d_gameover::dGameOver* ptr) = nullptr;

    // Shop Functions
    KEEP_VAR int32_t (*gReturn_seq_decide_yes)(libtp::tp::d_shop_system::dShopSystem* shopPtr,
                                               libtp::tp::f_op_actor::fopAc_ac_c* actor,
                                               void* msgFlow) = nullptr;

    // Title Screen functions
    KEEP_VAR void (*gReturn_dFile_select_c___create)(libtp::tp::d_file_select::dFile_select_c* thisPtr) = nullptr;

    // Pause Menu functions
    KEEP_VAR void (*gReturn_dMenuOption__tv_open1_move)(void* thisPtr) = nullptr;

    // Item Wheel menu
    namespace item_wheel_menu
    {
        KEEP_VAR void (*gReturn_dMenuRing__create)(void* dMenuRing) = nullptr;
        KEEP_VAR void (*gReturn_dMenuRing__delete)(void* dMenuRing) = nullptr;
        KEEP_VAR void (*gReturn_dMenuRing__draw)(void* dMenuRing) = nullptr;
    } // namespace item_wheel_menu
} // namespace mod
