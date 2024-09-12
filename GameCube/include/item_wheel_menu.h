#ifndef ITEM_WHEEL_MENU_H
#define ITEM_WHEEL_MENU_H

#include "tp/resource.h"
#include "data/stages.h"

#include <cstdint>
#include <cstring>

using namespace libtp::data::stage;

namespace mod::item_wheel_menu
{
    // Set up an enum for accessing specific areas
    enum TrackedAreas
    {
        FOREST_TEMPLE = 0,
        GORON_MINES,
        LAKEBED_TEMPLE,
        ARBITERS_GROUNDS,
        SNOWPEAK_RUINS,
        TEMPLE_OF_TIME,
        CITY_IN_THE_SKY,
        PALACE_OF_TWILIGHT,
        HYRULE_CASTLE,
        FARON_WOODS,
        BULBLIN_CAMP,
        TRACKED_AREAS_END
    };

    // Set up a struct for holding the strings
    struct ItemWheelMenuStrings
    {
        // Areas being tracked
        const char* areasBeingTracked[TRACKED_AREAS_END];

        // Seed:
        const char* seedIsLoaded;

        // Yes/No
        const char* yes;
        const char* no;

        // Fused Shadows\nMirror Shards
        const char* shadowsShards;

        const char* pumpkin;
        const char* cheese;
        const char* gateKeys;
        const char* areas;
        const char* smallKeys;
        const char* bigKeys;
        const char* maps;
        const char* compasses;

        // Press buttons to display/hide menu
        const char* helpText;
    };

    // Set up a struct for holding the offsets
    struct ItemWheelMenuOffsets
    {
        // Fused Shadows
        uint8_t shadowsShardsOffset; // Fused Shadows text to the start of the values

        // Pumpkin
        uint8_t pumpkinOffset;    // Fused Shadows values to the start of the text
        uint8_t pumpkinYesOffset; // Pumpkin text to the start of the value
        uint8_t pumpkinNoOffset;  // Pumpkin text to the start of the value

        // Cheese
        uint8_t cheeseOffset;    // Pumpkin text to the start of the text
        uint8_t cheeseYesOffset; // Cheese text to the start of the value
        uint8_t cheeseNoOffset;  // Cheese text to the start of the value

        // Gate Keys
        uint8_t gateKeysOffset;    // Cheese text to the start of the text
        uint8_t gateKeysYesOffset; // Gate Keys text to the start of the value
        uint8_t gateKeysNoOffset;  // Gate Keys text to the start of the value

        // Headers
        uint8_t headerSmallKeysOffset; // Areas text to the start of the text
        uint8_t headerBigKeysOffset;   // Small keys text to the start of the text
        uint8_t headerMapsOffset;      // Big keys text to the start of the text
        uint8_t headerCompassesOffset; // Maps text to the start of the text

        // Header values
        // Small Keys
        uint8_t valuesSmallKeysOffset; // Small keys text to the start of the values

        // Big Keys
        uint8_t valuesBigKeysYesOffset;   // Big keys text to the start of the values
        uint8_t valuesBigKeysNoOffset;    // Big keys text to the start of the values
        uint8_t valuesBigKeysMinesOffset; // Big keys text to the start of the values

        // Maps
        uint8_t valuesMapsYesOffset; // Maps text to the start of the values
        uint8_t valuesMapsNoOffset;  // Maps text to the start of the values

        // Compasses
        uint8_t valuesCompassesYesOffset; // Compasses text to the start of the values
        uint8_t valuesCompassesNoOffset;  // Compasses text to the start of the values
    };

    // Set up a class for holding the strings and offsets
    class ItemWheelMenuData
    {
       public:
        ItemWheelMenuData() {}
        ~ItemWheelMenuData() {}

        const uint8_t* getAreaColorIdsPtr() const { return &this->areaColorIds[0]; }
        const AreaNodesID* getSmallKeyAreaNodesPtr() const { return &this->smallKeyAreaNodes[0]; }

        const char* getTextDataPtr() const { return this->textData; }
        ItemWheelMenuStrings* getStringsPtr() { return &this->strings; }
        ItemWheelMenuOffsets* getOffsetsPtr() { return &this->offsets; }

        void setTextDataPtr(const char* data) { this->textData = data; }
        void createOffsets(const ItemWheelMenuOffsets* src) { memcpy(&this->offsets, src, sizeof(this->offsets)); }

       private:
        // Set up an array to hold each area's color id
        static constexpr const uint8_t areaColorIds[] = {MSG_COLOR_GREEN_HEX,
                                                         MSG_COLOR_RED_HEX,
                                                         CUSTOM_MSG_COLOR_BLUE_HEX,
                                                         MSG_COLOR_ORANGE_HEX,
                                                         MSG_COLOR_LIGHT_BLUE_HEX,
                                                         CUSTOM_MSG_COLOR_DARK_GREEN_HEX,
                                                         MSG_COLOR_YELLOW_HEX,
                                                         MSG_COLOR_PURPLE_HEX,
                                                         CUSTOM_MSG_COLOR_SILVER_HEX,
                                                         MSG_COLOR_GREEN_HEX,
                                                         MSG_COLOR_ORANGE_HEX};

        // Set up an array with all of the area node ids that small keys are tracked
        static constexpr const AreaNodesID smallKeyAreaNodes[] = {AreaNodesID::Forest_Temple,
                                                                  AreaNodesID::Goron_Mines,
                                                                  AreaNodesID::Lakebed_Temple,
                                                                  AreaNodesID::Arbiters_Grounds,
                                                                  AreaNodesID::Snowpeak_Ruins,
                                                                  AreaNodesID::Temple_of_Time,
                                                                  AreaNodesID::City_in_the_Sky,
                                                                  AreaNodesID::Palace_of_Twilight,
                                                                  AreaNodesID::Hyrule_Castle,
                                                                  AreaNodesID::Faron,
                                                                  AreaNodesID::Gerudo_Desert};
        // Pointer to all of the dynamic text
        // This is allocated memory
        const char* textData;

        // Strings used within the menu
        ItemWheelMenuStrings strings;

        // Offsets used within the menu
        ItemWheelMenuOffsets offsets;
    };

    // Set up a class for holding all of the item wheel menu stuff
    class ItemWheelMenu
    {
       public:
        ItemWheelMenu() {}
        ~ItemWheelMenu() {}

        ItemWheelMenuData* getDataPtr() { return &this->data; }
        bool shouldDrawRingThisFrame() const { return this->ringDrawnThisFrame; }
        bool shouldDisplayMenu() const { return this->displayMenu; }
        bool ringIsOpen() const { return this->ringOpen; }

        void drawRingThisFrame() { this->ringDrawnThisFrame = true; }
        void resetRingDrawnThisFrame() { this->ringDrawnThisFrame = false; }
        void setDisplayMenu(bool value) { this->displayMenu = value; }
        void dontDisplayMenu() { this->displayMenu = false; }
        void setRingOpen(bool state) { this->ringOpen = state; }

       private:
        ItemWheelMenuData data;

        // The ring is drawn more than once per frame, so this is set to true on the first draw
        bool ringDrawnThisFrame;

        // Handles whether the menu should be displayed or not
        bool displayMenu;

        // Used to determine whether or not the item wheel ring is open
        bool ringOpen;
    };
} // namespace mod::item_wheel_menu

#endif