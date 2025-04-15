#ifndef FUNCTIONHOOKS_H
#define FUNCTIONHOOKS_H

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

#include <cstdint>

namespace mod
{
    // Function hook handlers & trampolines
    void handle_fapGm_Execute_FailLoadSeed();
    void handle_fapGm_Execute();
    extern void (*gReturn_fapGm_Execute)();

    // DMC (REL) Hook
    bool handle_do_unlink(libtp::tp::dynamic_link::DynamicModuleControl* dmc);
    extern bool (*gReturn_do_unlink)(libtp::tp::dynamic_link::DynamicModuleControl* dmc);

    // DZX Functions; Handler is lambda -> randomizer::onDZX();
    bool handle_actorInit(void* mStatus_roomControl, libtp::tp::dzx::ChunkTypeInfo* chunkTypeInfo, int32_t unk3, void* unk4);

    extern bool (*gReturn_actorInit)(void* mStatus_roomControl,
                                     libtp::tp::dzx::ChunkTypeInfo* chunkTypeInfo,
                                     int32_t unk3,
                                     void* unk4);

    bool handle_actorInit_always(void* mStatus_roomControl,
                                 libtp::tp::dzx::ChunkTypeInfo* chunkTypeInfo,
                                 int32_t unk3,
                                 void* unk4);

    extern bool (*gReturn_actorInit_always)(void* mStatus_roomControl,
                                            libtp::tp::dzx::ChunkTypeInfo* chunkTypeInfo,
                                            int32_t unk3,
                                            void* unk4);

    bool handle_actorCommonLayerInit(void* mStatus_roomControl,
                                     libtp::tp::dzx::ChunkTypeInfo* chunkTypeInfo,
                                     int32_t unk3,
                                     void* unk4);

    extern bool (*gReturn_actorCommonLayerInit)(void* mStatus_roomControl,
                                                libtp::tp::dzx::ChunkTypeInfo* chunkTypeInfo,
                                                int32_t unk3,
                                                void* unk4);

    void handle_dComIfGp_setNextStage(const char* stage,
                                      int16_t point,
                                      int8_t roomNo,
                                      int8_t layer,
                                      float lastSpeed,
                                      uint32_t lastMode,
                                      int32_t setPoint,
                                      int8_t wipe,
                                      int16_t lastAngle,
                                      int32_t param_9,
                                      int32_t wipSpeedT);

    extern void (*gReturn_dComIfGp_setNextStage)(const char* stage,
                                                 int16_t point,
                                                 int8_t roomNo,
                                                 int8_t layer,
                                                 float lastSpeed,
                                                 uint32_t lastMode,
                                                 int32_t setPoint,
                                                 int8_t wipe,
                                                 int16_t lastAngle,
                                                 int32_t param_9,
                                                 int32_t wipSpeedT);

    int32_t handle_tgscInfoInit(void* stageDt, void* i_data, int32_t entryNum, void* param_3);
    extern int32_t (*gReturn_tgscInfoInit)(void* stageDt, void* i_data, int32_t entryNum, void* param_3);

    void handle_roomLoader(void* data, void* stageDt, int32_t roomNo);
    extern void (*gReturn_roomLoader)(void* data, void* stageDt, int32_t roomNo);

    // void handle_stageLoader( void* data, void* stageDt );
    // extern void ( *gReturn_stageLoader )( void* data, void* stageDt );

    int32_t handle_dStage_playerInit(void* stageDt,
                                     libtp::tp::d_stage::stage_dzr_header_entry* i_data,
                                     int32_t num,
                                     void* raw_data);

    extern int32_t (*gReturn_dStage_playerInit)(void* stageDt,
                                                libtp::tp::d_stage::stage_dzr_header_entry* i_data,
                                                int32_t num,
                                                void* raw_data);

    // State functions
    extern int32_t (*gReturn_getLayerNo_common_common)(const char* stageName, int32_t roomId, int32_t layerOverride);

    int32_t procCoGetItemInitCreateItem(const float pos[3],
                                        int32_t item,
                                        uint8_t unk3,
                                        int32_t unk4,
                                        int32_t unk5,
                                        const float rot[3],
                                        const float scale[3]);

    // Item creation functions. These are ran when the game displays an item though various means.
    int32_t handle_createItemForBoss(const float pos[3],
                                     int32_t item,
                                     int32_t roomNo,
                                     const int16_t rot[3],
                                     const float scale[3],
                                     float unk6,
                                     float unk7,
                                     int32_t parameters);

    extern int32_t (*gReturn_createItemForBoss)(const float pos[3],
                                                int32_t item,
                                                int32_t roomNo,
                                                const int16_t rot[3],
                                                const float scale[3],
                                                float unk6,
                                                float unk7,
                                                int32_t parameters);

    int32_t handle_createItemForPresentDemo(const float pos[3],
                                            int32_t item,
                                            uint8_t unk3,
                                            int32_t unk4,
                                            int32_t unk5,
                                            const float unk6[3],
                                            const float unk7[3]);

    extern int32_t (*gReturn_createItemForPresentDemo)(const float pos[3],
                                                       int32_t item,
                                                       uint8_t unk3,
                                                       int32_t unk4,
                                                       int32_t unk5,
                                                       const float unk6[3],
                                                       const float unk7[3]);

    int32_t handle_createItemForTrBoxDemo(const float pos[3],
                                          int32_t item,
                                          int32_t itemPickupFlag,
                                          int32_t roomNo,
                                          const int16_t rot[3],
                                          const float scale[3]);

    extern int32_t (*gReturn_createItemForTrBoxDemo)(const float pos[3],
                                                     int32_t item,
                                                     int32_t itemPickupFlag,
                                                     int32_t roomNo,
                                                     const int16_t rot[3],
                                                     const float scale[3]);

    int32_t handle_createItemForMidBoss(const float pos[3],
                                        int32_t item,
                                        int32_t roomNo,
                                        const int16_t rot[3],
                                        const float scale[3],
                                        int32_t unk6,
                                        int32_t itemPickupFlag);

    extern int32_t (*gReturn_createItemForMidBoss)(const float pos[3],
                                                   int32_t item,
                                                   int32_t roomNo,
                                                   const int16_t rot[3],
                                                   const float scale[3],
                                                   int32_t unk6,
                                                   int32_t itemPickupFlag);

    void handle_CheckFieldItemCreateHeap(libtp::tp::f_op_actor::fopAc_ac_c* actor);
    extern void (*gReturn_CheckFieldItemCreateHeap)(libtp::tp::f_op_actor::fopAc_ac_c* actor);

    // Item Wheel functions
    void handle_setLineUpItem(libtp::tp::d_save::dSv_player_item_c*);
    extern void (*gReturn_setLineUpItem)(libtp::tp::d_save::dSv_player_item_c*);

    // ItemGet functions. These functions handle the player and the flags set for their inventory
    void handle_execItemGet(uint8_t item);
    extern void (*gReturn_execItemGet)(uint8_t item);

    int32_t handle_checkItemGet(uint8_t item, int32_t defaultValue);
    extern int32_t (*gReturn_checkItemGet)(uint8_t item, int32_t defaultValue);

    void handle_item_func_ASHS_SCRIBBLING();
    extern void (*gReturn_item_func_ASHS_SCRIBBLING)();

    // Message functions
    bool handle_setMessageCode_inSequence(libtp::tp::control::TControl* control,
                                          const void* TProcessor,
                                          uint16_t unk3,
                                          uint16_t msgId);

    extern bool (*gReturn_setMessageCode_inSequence)(libtp::tp::control::TControl* control,
                                                     const void* TProcessor,
                                                     uint16_t unk3,
                                                     uint16_t msgId);

    uint32_t handle_getFontCCColorTable(uint8_t colorId, uint8_t unk);
    extern uint32_t (*gReturn_getFontCCColorTable)(uint8_t colorId, uint8_t unk);

    uint32_t handle_getFontGCColorTable(uint8_t colorId, uint8_t unk);
    extern uint32_t (*gReturn_getFontGCColorTable)(uint8_t colorId, uint8_t unk);

    void handle_jmessage_tSequenceProcessor__do_begin(void* seqProcessor, const void* unk2, const char* text);
    extern void (*gReturn_jmessage_tSequenceProcessor__do_begin)(void* seqProcessor, const void* unk2, const char* text);

    bool handle_jmessage_tSequenceProcessor__do_tag(void* seqProcessor, uint32_t unk2, const void* currentText, uint32_t unk4);
    extern bool (*gReturn_jmessage_tSequenceProcessor__do_tag)(void* seqProcessor,
                                                               uint32_t unk2,
                                                               const void* currentText,
                                                               uint32_t unk4);

    // Query/Event functions. Various uses
    int32_t handle_query001(void* unk1, void* unk2, int32_t unk3);
    extern int32_t (*gReturn_query001)(void* unk1, void* unk2, int32_t unk3);

    int32_t handle_query022(void* unk1, void* unk2, int32_t unk3);
    extern int32_t (*gReturn_query022)(void* unk1, void* unk2, int32_t unk3);

    int32_t handle_query023(void* unk1, void* unk2, int32_t unk3);
    extern int32_t (*gReturn_query023)(void* unk1, void* unk2, int32_t unk3);

    int32_t handle_query025(void* unk1, void* unk2, int32_t unk3);
    extern int32_t (*gReturn_query025)(void* unk1, void* unk2, int32_t unk3);

    uint8_t handle_checkEmptyBottle(libtp::tp::d_save::dSv_player_item_c* playerItem);
    extern uint8_t (*gReturn_checkEmptyBottle)(libtp::tp::d_save::dSv_player_item_c* playerItem);

    int32_t handle_query042(void* unk1, void* unk2, int32_t unk3);
    extern int32_t (*gReturn_query042)(void* unk1, void* unk2, int32_t unk3);

    int32_t handle_query037(void* unk1, void* unk2, int32_t unk3);
    extern int32_t (*gReturn_query037)(void* unk1, void* unk2, int32_t unk3);

    int32_t handle_query049(void* unk1, void* unk2, int32_t unk3);
    extern int32_t (*gReturn_query049)(void* unk1, void* unk2, int32_t unk3);

    // int32_t handle_event000( void* messageFlow, void* nodeEvent, void* actrPtr );
    // extern int32_t ( *gReturn_event000 )( void* messageFlow, void* nodeEvent, void* actrPtr );

    int32_t handle_event017(void* messageFlow, void* nodeEvent, void* actrPtr);
    extern int32_t (*gReturn_event017)(void* messageFlow, void* nodeEvent, void* actrPtr);

    int32_t handle_doFlow(libtp::tp::d_msg_flow::dMsgFlow* msgFlow,
                          libtp::tp::f_op_actor::fopAc_ac_c* actrPtr,
                          libtp::tp::f_op_actor::fopAc_ac_c** actrValue,
                          int32_t i_flow);

    extern int32_t (*gReturn_doFlow)(libtp::tp::d_msg_flow::dMsgFlow* msgFlow,
                                     libtp::tp::f_op_actor::fopAc_ac_c* actrPtr,
                                     libtp::tp::f_op_actor::fopAc_ac_c** actrValue,
                                     int32_t i_flow);

    int32_t handle_setNormalMsg(libtp::tp::d_msg_flow::dMsgFlow* msgFlow,
                                void* flowNode,
                                libtp::tp::f_op_actor::fopAc_ac_c* actrPtr);

    extern int32_t (*gReturn_setNormalMsg)(libtp::tp::d_msg_flow::dMsgFlow* msgFlow,
                                           void* flowNode,
                                           libtp::tp::f_op_actor::fopAc_ac_c* actrPtr);

    // Save flag functions
    bool handle_isDungeonItem(libtp::tp::d_save::dSv_memBit_c* memBitPtr, const int32_t memBit);
    extern bool (*gReturn_isDungeonItem)(libtp::tp::d_save::dSv_memBit_c* memBitPtr, const int32_t memBit);

    void handle_onDungeonItem(libtp::tp::d_save::dSv_memBit_c* memBitPtr, const int32_t memBit);
    extern void (*gReturn_onDungeonItem)(libtp::tp::d_save::dSv_memBit_c* memBitPtr, const int32_t memBit);

    bool handle_daNpcT_chkEvtBit(int16_t flag);
    extern bool (*gReturn_daNpcT_chkEvtBit)(int16_t flag);

    bool handle_isEventBit(libtp::tp::d_save::dSv_event_c* eventPtr, uint16_t flag);
    extern bool (*gReturn_isEventBit)(libtp::tp::d_save::dSv_event_c* eventPtr, uint16_t flag);

    void handle_onEventBit(libtp::tp::d_save::dSv_event_c* eventPtr, uint16_t flag);
    extern void (*gReturn_onEventBit)(libtp::tp::d_save::dSv_event_c* eventPtr, uint16_t flag);

    bool handle_isSwitch_dSv_memBit(libtp::tp::d_save::dSv_memBit_c* memoryBit, int32_t flag);
    extern bool (*gReturn_isSwitch_dSv_memBit)(libtp::tp::d_save::dSv_memBit_c* memoryBit, int32_t flag);

    void handle_onSwitch_dSv_memBit(libtp::tp::d_save::dSv_memBit_c* memoryBit, int32_t flag);
    extern void (*gReturn_onSwitch_dSv_memBit)(libtp::tp::d_save::dSv_memBit_c* memoryBit, int32_t flag);

    void handle_onSwitch_dSv_info(libtp::tp::d_save::dSv_info_c* memoryBit, int32_t flag, int32_t room);
    extern void (*gReturn_onSwitch_dSv_info)(libtp::tp::d_save::dSv_info_c* memoryBit, int32_t flag, int32_t room);

    bool handle_isDarkClearLV(void* playerStatusPtr, int32_t twilightNode);
    extern bool (*gReturn_isDarkClearLV)(void* playerStatusPtr, int32_t twilightNode);

    // Pause menu functions
    void handle_collect_save_open_init(uint8_t param_1);
    extern void (*gReturn_collect_save_open_init)(uint8_t param_1);

    // Link functions
    bool handle_checkBootsMoveAnime(libtp::tp::d_a_alink::daAlink* d_a_alink, int32_t param_1);
    extern bool (*gReturn_checkBootsMoveAnime)(libtp::tp::d_a_alink::daAlink* d_a_alink, int32_t param_1);

    void handle_setGetItemFace(libtp::tp::d_a_alink::daAlink* daALink, uint16_t itemID);
    extern void (*gReturn_setGetItemFace)(libtp::tp::d_a_alink::daAlink* daALink, uint16_t itemID);

    void handle_setWolfLockDomeModel(libtp::tp::d_a_alink::daAlink* daALink);
    extern void (*gReturn_setWolfLockDomeModel)(libtp::tp::d_a_alink::daAlink* daALink);

    bool handle_procFrontRollCrashInit(libtp::tp::d_a_alink::daAlink* daALink);
    extern bool (*gReturn_procFrontRollCrashInit)(libtp::tp::d_a_alink::daAlink* daALink);

    bool handle_procWolfDashReverseInit(libtp::tp::d_a_alink::daAlink* daALink, bool param_1);
    extern bool (*gReturn_procWolfDashReverseInit)(libtp::tp::d_a_alink::daAlink* daALink, bool param_1);

    bool handle_procWolfAttackReverseInit(libtp::tp::d_a_alink::daAlink* daALink);
    extern bool (*gReturn_procWolfAttackReverseInit)(libtp::tp::d_a_alink::daAlink* daALink);

    libtp::tp::f_op_actor::fopAc_ac_c* handle_searchBouDoor(libtp::tp::f_op_actor::fopAc_ac_c* actrPtr);
    extern libtp::tp::f_op_actor::fopAc_ac_c* (*gReturn_searchBouDoor)(libtp::tp::f_op_actor::fopAc_ac_c* actrPtr);

    bool handle_checkCastleTownUseItem(uint16_t item_id);
    extern bool (*gReturn_checkCastleTownUseItem)(uint16_t item_id);

    float handle_damageMagnification(libtp::tp::d_a_alink::daAlink* daALink, int32_t param_1, int32_t param_2);
    extern float (*gReturn_damageMagnification)(libtp::tp::d_a_alink::daAlink* daALink, int32_t param_1, int32_t param_2);

    int32_t handle_procCoGetItemInit(libtp::tp::d_a_alink::daAlink* linkActrPtr);
    extern int32_t (*gReturn_procCoGetItemInit)(libtp::tp::d_a_alink::daAlink* linkActrPtr);

    // Audio functions
    void handle_loadSeWave(void* Z2SceneMgr, uint32_t waveID);
    extern void (*gReturn_loadSeWave)(void* Z2SceneMgr, uint32_t waveID);

    void handle_sceneChange(libtp::z2audiolib::z2scenemgr::Z2SceneMgr* sceneMgr,
                            libtp::z2audiolib::z2scenemgr::JAISoundID BGMid,
                            uint8_t SeWave1,
                            uint8_t SeWave2,
                            uint8_t BgmWave1,
                            uint8_t BgmWave2,
                            uint8_t DemoWave,
                            bool param_7);

    extern void (*gReturn_sceneChange)(libtp::z2audiolib::z2scenemgr::Z2SceneMgr* sceneMgr,
                                       libtp::z2audiolib::z2scenemgr::JAISoundID BGMid,
                                       uint8_t SeWave1,
                                       uint8_t SeWave2,
                                       uint8_t BgmWave1,
                                       uint8_t BgmWave2,
                                       uint8_t DemoWave,
                                       bool param_7);

    void handle_startSound(void* soungMgr, libtp::z2audiolib::z2scenemgr::JAISoundID soundId, void* soundHandle, void* pos);
    extern void (*gReturn_startSound)(void* soundMgr,
                                      libtp::z2audiolib::z2scenemgr::JAISoundID soundId,
                                      void* soundHandle,
                                      void* pos);

    bool handle_checkBgmIDPlaying(libtp::z2audiolib::z2seqmgr::Z2SeqMgr* seqMgr, uint32_t sfx_id);
    extern bool (*gReturn_checkBgmIDPlaying)(libtp::z2audiolib::z2seqmgr::Z2SeqMgr* seqMgr, uint32_t sfx_id);

    // Title Screen functions
    void* handle_dScnLogo_c_dt(void* dScnLogo_c, int16_t bFreeThis);
    extern void* (*gReturn_dScnLogo_c_dt)(void* dScnLogo_c, int16_t bFreeThis);

    // Archive/Resource functions
    libtp::tp::d_resource::dRes_info_c* handle_getResInfo(const char* arcName,
                                                          libtp::tp::d_resource::dRes_info_c* objectInfo,
                                                          int32_t size);
    extern libtp::tp::d_resource::dRes_info_c* (*gReturn_getResInfo)(const char* arcName,
                                                                     libtp::tp::d_resource::dRes_info_c* objectInfo,
                                                                     int32_t size);

    bool handle_mountArchive__execute(libtp::tp::m_Do_dvd_thread::mDoDvdThd_mountArchive_c* mountArchive);
    extern bool (*gReturn_mountArchive__execute)(libtp::tp::m_Do_dvd_thread::mDoDvdThd_mountArchive_c* mountArchive);

    // d_meter functions
    void handle_resetMiniGameItem(libtp::tp::d_meter2_info::G_Meter2_Info* gMeter2InfoPtr, bool minigameFlag);
    extern void (*gReturn_resetMiniGameItem)(libtp::tp::d_meter2_info::G_Meter2_Info* gMeter2InfoPtr, bool minigameFlag);

    // Game Over functions
    void handle_dispWait_init(libtp::tp::d_gameover::dGameOver* ptr);
    extern void (*gReturn_dispWait_init)(libtp::tp::d_gameover::dGameOver* ptr);

    // Shop Functions
    int32_t handle_seq_decide_yes(libtp::tp::d_shop_system::dShopSystem* shopPtr,
                                  libtp::tp::f_op_actor::fopAc_ac_c* actor,
                                  void* msgFlow);

    extern int32_t (*gReturn_seq_decide_yes)(libtp::tp::d_shop_system::dShopSystem* shopPtr,
                                             libtp::tp::f_op_actor::fopAc_ac_c* actor,
                                             void* msgFlow);

    // Title Screen functions
    void resetQueueOnFileSelectScreen(libtp::tp::d_file_select::dFile_select_c* thisPtr);
    extern void (*gReturn_dFile_select_c___create)(libtp::tp::d_file_select::dFile_select_c* thisPtr);

    // Pause Menu functions
    void handle_dMenuOption__tv_open1_move(void* thisPtr);
    extern void (*gReturn_dMenuOption__tv_open1_move)(void* thisPtr);

    // Item Wheel menu
    namespace item_wheel_menu
    {
        void handle_dMenuRing__create(void* dMenuRing);
        extern void (*gReturn_dMenuRing__create)(void* dMenuRing);

        // dMenuRing__delete is an empty function, so the original function does not need to be called, so a return function is
        // not needed
        void handle_dMenuRing__delete(void* dMenuRing);

        void handle_dMenuRing__draw(void* dMenuRing);
        extern void (*gReturn_dMenuRing__draw)(void* dMenuRing);
    } // namespace item_wheel_menu
} // namespace mod

#endif
