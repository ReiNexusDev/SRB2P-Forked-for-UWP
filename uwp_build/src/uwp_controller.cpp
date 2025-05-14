#include <windows.h>
#include <winrt/Windows.Gaming.Input.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <vector>
#include <algorithm>
#include <mutex>
#include <cmath>

using namespace winrt;
using namespace Windows::Gaming::Input;
using namespace Windows::Foundation::Collections;

// Forward declare SRB2 console output function
extern "C" {
    void CONS_Printf(const char* fmt, ...);
}

// Structure to track controller state for SRB2
struct ControllerState {
    int playerIndex;
    bool connected;
    float leftStickX;
    float leftStickY;
    float rightStickX;
    float rightStickY;
    float leftTrigger;
    float rightTrigger;
    bool buttonA;
    bool buttonB;
    bool buttonX;
    bool buttonY;
    bool buttonStart;
    bool buttonBack;
    bool buttonLeftShoulder;
    bool buttonRightShoulder;
    bool buttonLeftThumb;
    bool buttonRightThumb;
    bool dpadUp;
    bool dpadDown;
    bool dpadLeft;
    bool dpadRight;
};

// Global state tracking
static std::vector<Gamepad> g_gamepads;
static std::vector<ControllerState> g_controllerStates;
static std::mutex g_gamepadMutex;
static const float DEAD_ZONE = 0.1f;

// Helper function to apply deadzone to analog sticks
float ApplyDeadZone(float value) {
    if (std::abs(value) < DEAD_ZONE) return 0.0f;
    return (value - DEAD_ZONE * (value > 0 ? 1 : -1)) / (1.0f - DEAD_ZONE);
}

// Initialize UWP controller support
void UWP_InitController() {
    // Register for gamepad add/remove events
    Gamepad::GamepadAdded([](auto&& sender, auto&& gamepad) {
        std::lock_guard<std::mutex> lock(g_gamepadMutex);
        
        // Check if this gamepad is already in our list
        auto it = std::find(g_gamepads.begin(), g_gamepads.end(), gamepad);
        if (it == g_gamepads.end()) {
            g_gamepads.push_back(gamepad);
            
            // Add a state entry for this controller
            int playerIndex = g_controllerStates.size();
            g_controllerStates.push_back({playerIndex, true, 0});
            
            CONS_Printf("Controller %d connected\n", playerIndex);
        }
    });

    Gamepad::GamepadRemoved([](auto&& sender, auto&& gamepad) {
        std::lock_guard<std::mutex> lock(g_gamepadMutex);
        
        // Find this gamepad in our list
        auto it = std::find(g_gamepads.begin(), g_gamepads.end(), gamepad);
        if (it != g_gamepads.end()) {
            int index = static_cast<int>(std::distance(g_gamepads.begin(), it));
            
            // Mark the controller as disconnected
            if (index < g_controllerStates.size()) {
                g_controllerStates[index].connected = false;
            }
            
            g_gamepads.erase(it);
            CONS_Printf("Controller %d disconnected\n", index);
        }
    });
    
    // Get any gamepads already connected
    std::lock_guard<std::mutex> lock(g_gamepadMutex);
    auto gamepads = Gamepad::Gamepads();
    for (const auto& gamepad : gamepads) {
        g_gamepads.push_back(gamepad);
        
        int playerIndex = g_controllerStates.size();
        g_controllerStates.push_back({playerIndex, true, 0});
        
        CONS_Printf("Controller %d connected\n", playerIndex);
    }
}

// Update controller states
void UWP_UpdateControllers() {
    std::lock_guard<std::mutex> lock(g_gamepadMutex);
    
    // Update all connected controllers
    for (size_t i = 0; i < g_gamepads.size(); i++) {
        if (i >= g_controllerStates.size()) continue;
        if (!g_controllerStates[i].connected) continue;
        
        try {
            const auto reading = g_gamepads[i].GetCurrentReading();
            
            // Update stick values with deadzone
            g_controllerStates[i].leftStickX = ApplyDeadZone(reading.LeftThumbstickX);
            g_controllerStates[i].leftStickY = ApplyDeadZone(reading.LeftThumbstickY);
            g_controllerStates[i].rightStickX = ApplyDeadZone(reading.RightThumbstickX);
            g_controllerStates[i].rightStickY = ApplyDeadZone(reading.RightThumbstickY);
            
            // Update trigger values
            g_controllerStates[i].leftTrigger = reading.LeftTrigger;
            g_controllerStates[i].rightTrigger = reading.RightTrigger;
            
            // Update button states
            g_controllerStates[i].buttonA = (reading.Buttons & GamepadButtons::A) == GamepadButtons::A;
            g_controllerStates[i].buttonB = (reading.Buttons & GamepadButtons::B) == GamepadButtons::B;
            g_controllerStates[i].buttonX = (reading.Buttons & GamepadButtons::X) == GamepadButtons::X;
            g_controllerStates[i].buttonY = (reading.Buttons & GamepadButtons::Y) == GamepadButtons::Y;
            g_controllerStates[i].buttonStart = (reading.Buttons & GamepadButtons::Menu) == GamepadButtons::Menu;
            g_controllerStates[i].buttonBack = (reading.Buttons & GamepadButtons::View) == GamepadButtons::View;
            g_controllerStates[i].buttonLeftShoulder = (reading.Buttons & GamepadButtons::LeftShoulder) == GamepadButtons::LeftShoulder;
            g_controllerStates[i].buttonRightShoulder = (reading.Buttons & GamepadButtons::RightShoulder) == GamepadButtons::RightShoulder;
            g_controllerStates[i].buttonLeftThumb = (reading.Buttons & GamepadButtons::LeftThumbstick) == GamepadButtons::LeftThumbstick;
            g_controllerStates[i].buttonRightThumb = (reading.Buttons & GamepadButtons::RightThumbstick) == GamepadButtons::RightThumbstick;
            g_controllerStates[i].dpadUp = (reading.Buttons & GamepadButtons::DPadUp) == GamepadButtons::DPadUp;
            g_controllerStates[i].dpadDown = (reading.Buttons & GamepadButtons::DPadDown) == GamepadButtons::DPadDown;
            g_controllerStates[i].dpadLeft = (reading.Buttons & GamepadButtons::DPadLeft) == GamepadButtons::DPadLeft;
            g_controllerStates[i].dpadRight = (reading.Buttons & GamepadButtons::DPadRight) == GamepadButtons::DPadRight;
        }
        catch (...) {
            // Controller might have been disconnected
            g_controllerStates[i].connected = false;
        }
    }
}

// Export C functions for SRB2 to call
extern "C" {
    // Initialize controller support
    void UWP_Controller_Init() {
        UWP_InitController();
    }
    
    // Update controller state
    void UWP_Controller_Update() {
        UWP_UpdateControllers();
    }
    
    // Get number of connected controllers
    int UWP_Controller_GetCount() {
        std::lock_guard<std::mutex> lock(g_gamepadMutex);
        return g_controllerStates.size();
    }
    
    // Check if a controller is connected
    bool UWP_Controller_IsConnected(int playerIndex) {
        std::lock_guard<std::mutex> lock(g_gamepadMutex);
        if (playerIndex < 0 || playerIndex >= g_controllerStates.size())
            return false;
        return g_controllerStates[playerIndex].connected;
    }
    
    // Get analog stick values
    float UWP_Controller_GetLeftStickX(int playerIndex) {
        std::lock_guard<std::mutex> lock(g_gamepadMutex);
        if (playerIndex < 0 || playerIndex >= g_controllerStates.size() || !g_controllerStates[playerIndex].connected)
            return 0.0f;
        return g_controllerStates[playerIndex].leftStickX;
    }
    
    float UWP_Controller_GetLeftStickY(int playerIndex) {
        std::lock_guard<std::mutex> lock(g_gamepadMutex);
        if (playerIndex < 0 || playerIndex >= g_controllerStates.size() || !g_controllerStates[playerIndex].connected)
            return 0.0f;
        return g_controllerStates[playerIndex].leftStickY;
    }
    
    float UWP_Controller_GetRightStickX(int playerIndex) {
        std::lock_guard<std::mutex> lock(g_gamepadMutex);
        if (playerIndex < 0 || playerIndex >= g_controllerStates.size() || !g_controllerStates[playerIndex].connected)
            return 0.0f;
        return g_controllerStates[playerIndex].rightStickX;
    }
    
    float UWP_Controller_GetRightStickY(int playerIndex) {
        std::lock_guard<std::mutex> lock(g_gamepadMutex);
        if (playerIndex < 0 || playerIndex >= g_controllerStates.size() || !g_controllerStates[playerIndex].connected)
            return 0.0f;
        return g_controllerStates[playerIndex].rightStickY;
    }
    
    // Get trigger values
    float UWP_Controller_GetLeftTrigger(int playerIndex) {
        std::lock_guard<std::mutex> lock(g_gamepadMutex);
        if (playerIndex < 0 || playerIndex >= g_controllerStates.size() || !g_controllerStates[playerIndex].connected)
            return 0.0f;
        return g_controllerStates[playerIndex].leftTrigger;
    }
    
    float UWP_Controller_GetRightTrigger(int playerIndex) {
        std::lock_guard<std::mutex> lock(g_gamepadMutex);
        if (playerIndex < 0 || playerIndex >= g_controllerStates.size() || !g_controllerStates[playerIndex].connected)
            return 0.0f;
        return g_controllerStates[playerIndex].rightTrigger;
    }
    
    // Get button states
    bool UWP_Controller_GetButtonA(int playerIndex) {
        std::lock_guard<std::mutex> lock(g_gamepadMutex);
        if (playerIndex < 0 || playerIndex >= g_controllerStates.size() || !g_controllerStates[playerIndex].connected)
            return false;
        return g_controllerStates[playerIndex].buttonA;
    }
    
    bool UWP_Controller_GetButtonB(int playerIndex) {
        std::lock_guard<std::mutex> lock(g_gamepadMutex);
        if (playerIndex < 0 || playerIndex >= g_controllerStates.size() || !g_controllerStates[playerIndex].connected)
            return false;
        return g_controllerStates[playerIndex].buttonB;
    }
    
    bool UWP_Controller_GetButtonX(int playerIndex) {
        std::lock_guard<std::mutex> lock(g_gamepadMutex);
        if (playerIndex < 0 || playerIndex >= g_controllerStates.size() || !g_controllerStates[playerIndex].connected)
            return false;
        return g_controllerStates[playerIndex].buttonX;
    }
    
    bool UWP_Controller_GetButtonY(int playerIndex) {
        std::lock_guard<std::mutex> lock(g_gamepadMutex);
        if (playerIndex < 0 || playerIndex >= g_controllerStates.size() || !g_controllerStates[playerIndex].connected)
            return false;
        return g_controllerStates[playerIndex].buttonY;
    }
    
    // Check for the most common Xbox buttons
    bool UWP_Controller_GetButtonStart(int playerIndex) {
        std::lock_guard<std::mutex> lock(g_gamepadMutex);
        if (playerIndex < 0 || playerIndex >= g_controllerStates.size() || !g_controllerStates[playerIndex].connected)
            return false;
        return g_controllerStates[playerIndex].buttonStart;
    }
    
    bool UWP_Controller_GetButtonDPadUp(int playerIndex) {
        std::lock_guard<std::mutex> lock(g_gamepadMutex);
        if (playerIndex < 0 || playerIndex >= g_controllerStates.size() || !g_controllerStates[playerIndex].connected)
            return false;
        return g_controllerStates[playerIndex].dpadUp;
    }
    
    bool UWP_Controller_GetButtonDPadDown(int playerIndex) {
        std::lock_guard<std::mutex> lock(g_gamepadMutex);
        if (playerIndex < 0 || playerIndex >= g_controllerStates.size() || !g_controllerStates[playerIndex].connected)
            return false;
        return g_controllerStates[playerIndex].dpadDown;
    }
    
    bool UWP_Controller_GetButtonDPadLeft(int playerIndex) {
        std::lock_guard<std::mutex> lock(g_gamepadMutex);
        if (playerIndex < 0 || playerIndex >= g_controllerStates.size() || !g_controllerStates[playerIndex].connected)
            return false;
        return g_controllerStates[playerIndex].dpadLeft;
    }
    
    bool UWP_Controller_GetButtonDPadRight(int playerIndex) {
        std::lock_guard<std::mutex> lock(g_gamepadMutex);
        if (playerIndex < 0 || playerIndex >= g_controllerStates.size() || !g_controllerStates[playerIndex].connected)
            return false;
        return g_controllerStates[playerIndex].dpadRight;
    }
}