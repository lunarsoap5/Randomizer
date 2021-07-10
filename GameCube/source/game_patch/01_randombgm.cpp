/**
 * @file 01_bgm.cpp
 * @brief Handles randomizing the game's BGM
 * 
 * @author jdflyer
 * @bug No known bugs
 */
#include "game_patch/01_randombgm.h"
#include "main.h"
#include "tp/Z2SceneMgr.h"
#include "patch.h"

namespace mod::game_patch::bgm::bgmrando
{
    /** 
     * This data structure stores a single bgm track
     * It doesn't store the full ID in order to save memory
     * However, this means that we can't play music streams
     * In addition, both BGMWaves may not be needed to play
     * some tracks. More research needs to be done.
     */
    struct BGM {
        uint8_t id = 0x0;
        uint8_t bgmWave1 = 0x0;
        uint8_t bgmWave2 = 0x0;
    };

    /**
     * This array stores the BGM id and both BGM Waves
     * in order to play a track. Once the music ids are
     * randomized, this can be used as a lookup table
     * for BGM Wave ids in order to load BGM waves.
     * @todo Fix broken music
     */
    static const BGM bgmSource[] = {
        {0x0,0x19},
        {0x1,0x19},
        {0x4,0xd},
        {0x5,0x3,0x4},
        {0x6,0x1,0x2},
        {0x9,0xa,0x40},
        {0xd,0xc},
        {0xe,0xc},
        {0xf},
        {0x10,0x1,0x2},
        {0x11,0x1,0x2},
        {0x16,0xe},
        {0x17,0x5},
        {0x18,0x13,0x3c},
        {0x1a,0x11},
        {0x1b,0xe},
        {0x1d,0x13,0x14},
        {0x1e,0x7},
        {0x1f,0x7},
        {0x20,0x15,0x40},
        {0x23,0xf},
        {0x24,0x8,0x9},
        {0x25,0x16},
        {0x26,0x10,0x18},
        {0x27,0x15,0x17},
        {0x29,0x10,0x18},
        {0x2c,0x8},
        {0x2d,0x1a,0x41},
        //{0x2e,0x40}, //Lost woods statue game (Research must be done to fix)
        {0x2f,0xe,0x1c},
        {0x30,0x1e},
        {0x31,0x1e},
        {0x34,0x1f},
        {0x35,0x20},
        {0x36,0xe,0x1c},
        {0x37,0x1d,0x40},
        {0x39,0x28,0x48},
        {0x3a,0x24},
        {0x3b,0x25},
        {0x3c,0x26,0x40},
        {0x3d,0x27},
        {0x3e,0x28},
        {0x3f,0x29},
        {0x40,0x11,0x12},
        {0x41,0x2a,0x1b},
        {0x42,0x2b},
        {0x47,0xe,0x2c},
        {0x48,0x2c},
        {0x49},
        {0x4a,0x2d,0x3a},
        {0x4b,0x2d,0x3a},
        {0x4c,0x2e},
        {0x4d,0x2e},
        {0x50,0x2f},
        {0x51,0x30},
        {0x57,0x31},
        {0x58,0x32},
        {0x59,0x33},
        {0x5a,0x34}, //Lake hylia, but plays plums song (Music state needs to be changed for this BGM)
        {0x5e,0x35},
        {0x5f,0x36},
        {0x60,0x37},
        {0x61,0x26,0x3f},
        {0x62,0x39},
        {0x6b,0x3e},
        {0x6c,0x26,0x3f},
        {0x6d,0x40},
        {0x6e,0x24,0x57},
        {0x6f,0x26,0x3f},
        {0x70,0x44},
        {0x77,0x45},
        {0x78,0x46},
        {0x7a,0x46},
        {0x85,0x28,0x48},
        {0x86,0x49},
        {0x87,0x4a},
        {0x88,0x4b},
        {0x8b,0x4c,0x1a},
        {0x8c,0x4c,0x1a},
        {0x8f,0x1e,0x27},
        {0x90,0x1e,0x27},
        {0x91,0x8,0x47},
        {0x94,0x4e,0x26},
        {0x95,0x4e,0x26},
        {0x96,0x1c,0xe}, //Waves may not be correct
        {0x97,0x3d,0x1c},
        {0x9a,0x8},
        {0x9b,0x56},
        {0x9e,0x19},
        {0x9f,0x55},
        {0xa5,0x57},
        {0xa8,0x58,0x29}, //Waves may not be correct
        {0xa9,0x59}
    };

    const uint8_t bgmSourceLength = sizeof(bgmSource)/sizeof(BGM);

    static uint8_t randomizedBgms[bgmSourceLength];

    void (*sceneChangeTrampoline)(libtp::tp::z2audiolib::z2scenemgr::Z2SceneMgr* sceneMgr, libtp::tp::z2audiolib::z2scenemgr::JAISoundID id,uint8_t SeWave1,uint8_t SeWave2,uint8_t BgmWave1,uint8_t BgmWave2,uint8_t DemoWave,bool param_7) = nullptr;

    void initRandomBgm( rando::Randomizer* randomizer, bool set) {
        mod::console << randomizer->m_SeedInfo->header.seed << "::RandomBgm [" << (set ? "x" : " ") << "]\n";

        if (!set){
            if (sceneChangeTrampoline != nullptr) {
                libtp::patch::unhookFunction(sceneChangeTrampoline);
            }
            return;
        }

        /**
         * @todo Find a better way to assign a random track to each source track
         * The current implementation of doing this is not optimized
         */
        for(uint8_t i = 0; i<bgmSourceLength; i++){ //Fills in the randomizedBGMs array with random ids
            bool gotUnique = false;
            while (!gotUnique) { //Depends on randomness to shuffle the original array; needs a better method
                uint8_t random = randomizer->getRandom(bgmSourceLength);
                uint8_t idGot = bgmSource[random].id;
                bool valueExists = false;
                for (uint8_t j = 0; j<i; j++) {
                    if (randomizedBgms[j] == idGot){
                        valueExists = true;
                        break;
                    }
                }
                if (valueExists == false){
                    randomizedBgms[i] = bgmSource[random].id;
                    gotUnique = true;
                    break;
                }
            }
        } 
        libtp::patch::hookFunction(libtp::tp::z2audiolib::z2scenemgr::sceneChange,[](libtp::tp::z2audiolib::z2scenemgr::Z2SceneMgr* sceneMgr, libtp::tp::z2audiolib::z2scenemgr::JAISoundID BGMid,uint8_t SeWave1,uint8_t SeWave2,uint8_t BgmWave1,uint8_t BgmWave2,uint8_t DemoWave,bool param_7){
            uint32_t id = BGMid.id;
            if (id>=0x1000000 && id<0x2000000)
            {
                 //Only Sequences are applied here
                id = id-0x1000000;
                uint8_t index_of_id = 0;
                bool found = false;
                for (uint8_t i = 0; i < bgmSourceLength; i++) {
                    if (randomizedBgms[i] == id) {
                        index_of_id = i;
                        found = true;
                        break;
                    }
                }
                if (found) {
                    libtp::tp::z2audiolib::z2scenemgr::JAISoundID new_id;
                    new_id.id = bgmSource[index_of_id].id + 0x1000000;
                    sceneChangeTrampoline(sceneMgr,new_id,SeWave1,SeWave2,bgmSource[index_of_id].bgmWave1,bgmSource[index_of_id].bgmWave2,DemoWave,param_7);
                }else{
                    sceneChangeTrampoline(sceneMgr,BGMid,SeWave1,SeWave2,BgmWave1,BgmWave2,DemoWave,param_7);
                }
            }else{
                sceneChangeTrampoline(sceneMgr,BGMid,SeWave1,SeWave2,BgmWave1,BgmWave2,DemoWave,param_7);
            }
                });
    }
}   //namespace mod::game_patch::bgm::bgmrando