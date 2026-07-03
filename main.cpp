#include <string>
#include <format>                           // For formatting the tooltip strings
#include <SDL3/SDL.h>                       // For Surface and Tray-related stuff
#include <SDL3_gfx/SDL3_gfxPrimitives.h>    // For drawing to the Surfaces
#include <nlohmann/json.hpp>                // For parsing the headsetcontrol output

using Json = nlohmann::json;

// Run the headsetcontrol command line utility and give its STDOUT
std::string runHeadsetControl()
{
    FILE* pipe = popen("headsetcontrol -b -o JSON", "r");
    if(!pipe)
        return "";

    std::string output;
    char buffer[256];
    while(fgets(buffer, sizeof(buffer), pipe))
        output += buffer;
    pclose(pipe);
    return output;
}

// Creates a standardized icon surface with specified color
SDL_Surface* makeIcon(Uint8 r, Uint8 g, Uint8 b)
{
    SDL_Surface* surf = SDL_CreateSurface(22, 22, SDL_PIXELFORMAT_RGBA8888);
    SDL_Renderer* renderer = SDL_CreateSoftwareRenderer(surf);
    filledCircleRGBA(renderer, 11, 11, 9, r, g, b, 255);
    SDL_DestroyRenderer(renderer);
    return surf;
}

// Handles the tray icon's Exit menu option - sets the exit condition for the main loop
void onExit(void* userdata, SDL_TrayEntry* entry)
{
    bool* p = static_cast<bool*>(userdata);
    *p = false;
}

// Handles the tray icon's Scan Now menu option - shortcuts the wait loop
void onScanNow(void* userdata, SDL_TrayEntry* entry)
{
    bool* p = static_cast<bool*>(userdata);
    *p = true;
}

// Program entry point - setup, main loop, cleanup
int main(int argc, char* argv[])
{
    // Initial conditions for loop control
    bool running = true, scanNow = true;
    // Initialize SDL and return an error code if it fails
    if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
        return 1;

    // Will be overwritten later with real data - initialized to error state
    std::string tooltip  = "No Headset Detected";

    // All of the icon colors - cleaned up after loop
    SDL_Surface* iconGray = makeIcon(128, 128, 128);
    SDL_Surface* iconRed = makeIcon(200, 0, 0);
    SDL_Surface* iconYellow = makeIcon(200, 200, 0);
    SDL_Surface* iconGreen = makeIcon(0, 200, 0);

    // The system tray icon - initially gray to reflect nothing detected yet
    SDL_Tray* tray = SDL_CreateTray(iconGray, tooltip.c_str());

    // Set up the tray menu items for Scan Now and Exit
    SDL_TrayMenu* menu = SDL_CreateTrayMenu(tray);
    SDL_TrayEntry* scanNowEntry = SDL_InsertTrayEntryAt(menu, -1, "Scan Now", SDL_TRAYENTRY_BUTTON);
    SDL_TrayEntry* exitEntry = SDL_InsertTrayEntryAt(menu, -1, "Exit", SDL_TRAYENTRY_BUTTON);
    SDL_SetTrayEntryCallback(scanNowEntry, onScanNow, &scanNow);
    SDL_SetTrayEntryCallback(exitEntry, onExit, &running);

    // Main loop
    while(running)
    {
        // Wait one minute between headset polls unless otherwise specified
        // The first run shortcuts this and the Scan Now menu item shortcuts this
        for(int i = 0; i < 600 && running && !scanNow; i++)
        {
            SDL_Event event;
            while(SDL_PollEvent(&event)) { }
            SDL_Delay(100);
        }

        // If the Exit menu item shortcutted the wait, just bail early
        if(!running)
            break;

        // If the Scan Now was cued, reset it for next loop and refresh the devices list
        if(scanNow)
            scanNow = false;

        // Set up trackers for the battery level and device name
        int batteryLevel = -1;
        std::string deviceName = "NO DEVICE";

        // Perform the console command to retrieve headset status
        std::string output = runHeadsetControl();

        // Load the headset status into a Json node, bailing to next loop on error
        Json n;
        try { n = Json::parse(output); }
        catch(...) { continue; }
        if(!n.contains("device_count"))
            continue;

        // Retrieve the headset status elements
        int deviceCount = n["device_count"];
        if(deviceCount > 0)
        {
            const Json& deviceNode = n["devices"][0];
            if(deviceNode.contains("device"))
                deviceName = deviceNode["device"];
            if(!deviceNode.contains("capabilities"))
                continue;
            bool hasBattery = false;
            for(const std::string& capStr : deviceNode["capabilities"])
            {
                if(capStr == "CAP_BATTERY_STATUS")
                {
                    hasBattery = true;
                    break;
                }
            }
            if(!hasBattery)
                continue;
            const Json& batteryNode = deviceNode["battery"];
            if(!batteryNode.contains("level"))
                continue;
            batteryLevel = batteryNode["level"];
        }

        // If we were able to retrieve the status elements, set the tooltip and icon appropriately
        if(batteryLevel != -1)
        {
            tooltip = std::format("{}: {}%", deviceName.c_str(), batteryLevel);
            SDL_SetTrayTooltip(tray, tooltip.c_str());

            if(batteryLevel >= 25)
                SDL_SetTrayIcon(tray, iconGreen);
            else if (batteryLevel >= 10)
                SDL_SetTrayIcon(tray, iconYellow);
            else
                SDL_SetTrayIcon(tray, iconRed);
        }
        // On any error retrieving the status elements, set the tooltip and icon to error states
        else
        {
            tooltip = "No headset found";
            SDL_SetTrayTooltip(tray, tooltip.c_str());
            SDL_SetTrayIcon(tray, iconGray);
        }
    }

    // Clean up the tray icon
    SDL_DestroyTray(tray);

    // Clean up the icon surfaces
    SDL_DestroySurface(iconGreen);
    SDL_DestroySurface(iconYellow);
    SDL_DestroySurface(iconRed);
    SDL_DestroySurface(iconGray);

    // Clean up SDL
    SDL_Quit();
    // Return success
    return 0;
}