#include "main.h"
#include "patch.h"
#include "rando/seed.h"
#include "Z2AudioLib/Z2SceneMgr.h"
#include "Z2AudioLib/Z2SoundMgr.h"
#include "functionHooks.h"

namespace mod
{
    KEEP_FUNC void handle_sceneChange(libtp::z2audiolib::z2scenemgr::Z2SceneMgr* sceneMgr,
                                      libtp::z2audiolib::z2scenemgr::JAISoundID BGMid,
                                      uint8_t SeWave1,
                                      uint8_t SeWave2,
                                      uint8_t BgmWave1,
                                      uint8_t BgmWave2,
                                      uint8_t DemoWave,
                                      bool param_7)
    {
        // Make sure id is a bgm
        const uint32_t bgmId = BGMid.id;
        if ((bgmId < 0x1000000) || (bgmId >= 0x2000000))
        {
            // Call the original function
            return gReturn_sceneChange(sceneMgr, BGMid, SeWave1, SeWave2, BgmWave1, BgmWave2, DemoWave, param_7);
        }

        rando::Seed* seedPtr = rando::gRandomizer->getSeedPtr();
        const rando::BGMReplacement* bgmTablePtr = seedPtr->getBgmTablePtr();
        const uint32_t entries = seedPtr->getNumShuffledBgmTracks();
        const uint32_t bgmIdCheck = bgmId - 0x1000000;

        for (uint32_t i = 0; i < entries; i++)
        {
            const rando::BGMReplacement* currentBgmTablePtr = &bgmTablePtr[i];
            if (currentBgmTablePtr->getOriginalBgmTrack() == bgmIdCheck)
            {
                // Original ids are the same, return the function with replaced values
                libtp::z2audiolib::z2scenemgr::JAISoundID replacementId;
                replacementId.id = currentBgmTablePtr->getReplacementBgmTrack() + 0x1000000;

                return gReturn_sceneChange(sceneMgr,
                                           replacementId,
                                           SeWave1,
                                           SeWave2,
                                           currentBgmTablePtr->getReplacementBgmWave(),
                                           BgmWave2,
                                           DemoWave,
                                           param_7);
            }
        }

        // Call the original function
        gReturn_sceneChange(sceneMgr, BGMid, SeWave1, SeWave2, BgmWave1, BgmWave2, DemoWave, param_7);
    }

    KEEP_FUNC void handle_startSound(void* soundMgr,
                                     libtp::z2audiolib::z2scenemgr::JAISoundID soundId,
                                     void* soundHandle,
                                     void* pos)
    {
        const uint32_t id = soundId.id;
        if ((id < 0x1000000) || (id >= 0x2000000))
        {
            // Call the original function
            return gReturn_startSound(soundMgr, soundId, soundHandle, pos);
        }

        // Sound playing is part of the bgm table
        rando::Seed* seedPtr = rando::gRandomizer->getSeedPtr();
        const rando::BGMReplacement* fanfareTablePtr = seedPtr->getFanfareTablePtr();
        const uint32_t entries = seedPtr->getNumShuffledFanfares();
        const uint32_t idCheck = id - 0x1000000;

        for (uint32_t i = 0; i < entries; i++)
        {
            const rando::BGMReplacement* currentfanfareTable = &fanfareTablePtr[i];
            if (currentfanfareTable->getOriginalBgmTrack() == idCheck)
            {
                // Both have the same ID, so play the replacement
                libtp::z2audiolib::z2scenemgr::JAISoundID replacementId;
                replacementId.id = currentfanfareTable->getReplacementBgmTrack() + 0x1000000;

                return gReturn_startSound(soundMgr, replacementId, soundHandle, pos);
            }
        }

        // Call the original function
        gReturn_startSound(soundMgr, soundId, soundHandle, pos);
    }
} // namespace mod