/**	@file randomizer.h
 *  @brief Randomizer main class
 *
 *	@author AECX
 *	@bug No known bugs.
 */
#ifndef RANDO_RANDOMIZER_H
#define RANDO_RANDOMIZER_H

// TODO: Fix problems requiring the Randomizer class and gRandomizer to be declared before handling any includes
namespace mod::rando
{
    class Randomizer;
    extern rando::Randomizer* gRandomizer;
} // namespace mod::rando

#include "rando/data.h"
#include "rando/seed.h"
#include "tools.h"
#include "tp/dynamic_link.h"
#include "tp/dzx.h"
#include "tp/d_resource.h"
#include "tp/m_Do_dvd_thread.h"
#include "tp/d_a_alink.h"
#include "rando/customItems.h"
#include "tp/J2DPicture.h"
#include "item_wheel_menu.h"
#include "main.h"

#ifdef TP_EU
#include "tp/d_s_logo.h"
#endif

#include <cstdint>

namespace mod::rando
{
    enum TimeChange
    {
        NO_CHANGE = 0,
        CHANGE_TO_NIGHT,
        CHANGE_TO_DAY,
    };

    enum EventItemStatus : uint8_t
    {
        QUEUE_EMPTY = 0,
        ITEM_IN_QUEUE = 1,
        CLEAR_QUEUE = 2,
    };

    class Randomizer
    {
       public:
        Randomizer();
        ~Randomizer();

        /**
         * @brief Essentially call dvd::DVDConvertPathToEntrynum in very few assembly instructions using results cached during
         * startup.
         *
         * @param dvdEntryNumId Enum of the dvd filepath for which you want the entryNum.
         *
         * @return int32_t dvdEntryNum Result from dvd::DVDConvertPathToEntrynum
         */
        int32_t getDvdEntryNum(DvdEntryNumId dvdEntryNumId)
        {
            if (dvdEntryNumId >= DvdEntryNumId::DvdEntryNumIdSize)
            {
                return -1;
            }
            return this->m_LookupTable[dvdEntryNumId];
        }

        void setDvdEntryNum(int32_t entryNum, DvdEntryNumId dvdEntryNumId)
        {
            if (dvdEntryNumId >= DvdEntryNumId::DvdEntryNumIdSize)
            {
                return;
            }
            this->m_LookupTable[dvdEntryNumId] = entryNum;
        }

        Seed* getSeedPtr() const { return this->m_Seed; }
        GoldenWolfItemReplacement* getGoldenWolfItemReplacementPtr() { return &this->m_GoldenWolfItemReplacement; }
        customItems::FoolishItems* getFoolishItemsPtr() { return &this->m_FoolishItems; }
        item_wheel_menu::ItemWheelMenu* getItemWheelMenuPtr() { return &this->m_ItemWheelMenu; }

        const uint8_t* getMsgTableInfoPtr() const { return this->m_MsgTableInfo; }
        const uint8_t* getHintMsgTableInfoPtr() const { return this->m_HintMsgTableInfo; }
        libtp::tp::J2DPicture::J2DPicture* getBgWindowPtr() const { return this->m_BgWindow; }
        void* getZ2ScenePtr() const { return this->m_Z2ScenePtr; }
        uint32_t* getRandStatePtr() { return &this->m_RandState; }

        // Analog L is currently not being used, so commented out
        // float getPrevFrameAnalogL() const { return this->m_PrevFrameAnalogL; }
        float getPrevFrameAnalogR() const { return this->m_PrevFrameAnalogR; }

        uint16_t getLastButtonInput() const { return this->m_LastButtonInput; }
        uint16_t getTotalMsgEntries() const { return this->m_TotalMsgEntries; }
        uint16_t getTotalHintMsgEntries() const { return this->m_TotalHintMsgEntries; }
        bool getRoomReloadingState() const { return this->m_RoomReloadingState; }
        uint8_t getGameState() const { return this->m_GameState; }
        EventItemStatus getGiveItemToPlayerStatus() const { return this->m_GiveItemToPlayerStatus; }
        uint8_t getDungeonItemAreaColorIndex() const { return this->m_DungeonItemAreaColorIndex; }
        TimeChange getTimeChange() const { return this->m_TimeChange; }
        bool checkMenuRingOpen() const { return this->m_MenuRingOpen; }

#ifdef TP_EU
        libtp::tp::d_s_logo::Languages getCurrentLanguage() const { return this->m_CurrentLanguage; }
#endif
        bool randomizerIsEnabled() const { return this->m_Enabled; }
        bool seedAppliedToFile() const { return this->m_SeedInit; }

        void setMsgTableInfoPtr(const uint8_t* ptr) { this->m_MsgTableInfo = ptr; }
        void setHintMsgTableInfoPtr(const uint8_t* ptr) { this->m_HintMsgTableInfo = ptr; }
        void setBgWindowPtr(libtp::tp::J2DPicture::J2DPicture* ptr) { this->m_BgWindow = ptr; }
        void setZ2ScenePtr(void* ptr) { this->m_Z2ScenePtr = ptr; }
        void setRandState(uint32_t state) { this->m_RandState = state; }

        // Analog L is currently not being used, so commented out
        // void setPrevFrameAnalogL(float value) { this->m_PrevFrameAnalogL = value; }
        void setPrevFrameAnalogR(float value) { this->m_PrevFrameAnalogR = value; }

        void setLastButtonInput(uint16_t input) { this->m_LastButtonInput = input; }
        void setTotalMsgEntries(uint16_t entries) { this->m_TotalMsgEntries = entries; }
        void setTotalHintMsgEntries(uint16_t entries) { this->m_TotalHintMsgEntries = entries; }
        void setRoomReloadingState(bool state) { this->m_RoomReloadingState = state; }
        void setGameState(uint8_t state) { this->m_GameState = state; }
        void setGiveItemToPlayerStatus(EventItemStatus status) { this->m_GiveItemToPlayerStatus = status; }
        void setDungeonItemAreaColorIndex(uint8_t index) { this->m_DungeonItemAreaColorIndex = index; }
        void setTimeChange(TimeChange time) { this->m_TimeChange = time; }
        void setMenuRingOpen(bool state) { this->m_MenuRingOpen = state; }

#ifdef TP_EU
        void setCurrentLanguage(libtp::tp::d_s_logo::Languages language) { this->m_CurrentLanguage = language; }
#endif
        void enableRandomizer() { this->m_Enabled = true; }

        // This also sets `m_SeedInit` to false
        void disableRandomizer()
        {
            this->m_Enabled = false;
            this->m_SeedInit = false;
        }

        void onStageLoad();
        void initSave();
        void initRandState();
        void overrideREL();
        void overrideDZX(libtp::tp::dzx::ChunkTypeInfo* chunkTypeInfo);
        int32_t getPoeItem(uint8_t flag);
        uint8_t getSkyCharacter();
        uint8_t getBossItem();
        uint8_t getEventItem(uint8_t flag);
        void overrideARC(uint32_t fileAddr, FileDirectory fileDirectory, int32_t roomNo);
        void overrideObjectARC(libtp::tp::d_resource::dRes_info_c* resourcePtr, const char* fileName);
        void overrideEventARC();
        uint8_t overrideBugReward(uint8_t bugID);
        void getHiddenSkillItem(void* daNpcGWolfPtr, int16_t flag, uint32_t markerFlag);
        void replaceWolfLockDomeColor(libtp::tp::d_a_alink::daAlink* linkActrPtr);
        void addItemToEventQueue(uint32_t itemToAdd);
        void recolorArchiveTextures(libtp::tp::m_Do_dvd_thread::mDoDvdThd_mountArchive_c* mountArchive);
        uint8_t getFoolishItemModelId(uint8_t originalItem);

        // NOTE: This function returns dynamic memory
        BMDEntry* generateBmdEntries(DvdEntryNumId arcIndex, uint32_t numEntries);

       private:
        Seed* m_Seed;
        int32_t m_LookupTable[DvdEntryNumId::DvdEntryNumIdSize];
        GoldenWolfItemReplacement m_GoldenWolfItemReplacement;
        customItems::FoolishItems m_FoolishItems;
        item_wheel_menu::ItemWheelMenu m_ItemWheelMenu;

        const uint8_t* m_MsgTableInfo;     // Custom message string data
        const uint8_t* m_HintMsgTableInfo; // Custom message string data
        libtp::tp::J2DPicture::J2DPicture* m_BgWindow;
        void* m_Z2ScenePtr;
        uint32_t m_RandState;

        // Analog L is currently not being used, so commented out
        // float m_PrevFrameAnalogL;
        float m_PrevFrameAnalogR;

        uint16_t m_TotalMsgEntries;
        uint16_t m_TotalHintMsgEntries;
        uint16_t m_LastButtonInput;
        bool m_RoomReloadingState;
        bool m_MenuRingOpen;
        uint8_t m_GameState;
        uint8_t m_DungeonItemAreaColorIndex;
        EventItemStatus m_GiveItemToPlayerStatus;
        TimeChange m_TimeChange;

#ifdef TP_EU
        libtp::tp::d_s_logo::Languages m_CurrentLanguage;
#endif
        bool m_Enabled;  // True if the randomizer is currently enabled
        bool m_SeedInit; // True if seed-specific patches, flags, etc. have been applied to the save-file
    };

} // namespace mod::rando
#endif