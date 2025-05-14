#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <mutex>
#include <lua.hpp>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Foundation.h>

using namespace winrt;
using namespace Windows::Storage;
using namespace Windows::Foundation;

// Forward declarations for SRB2 functions
extern "C" {
    void CONS_Printf(const char* fmt, ...);
    void I_Error(const char* error, ...);
}

// External declaration for storage functions
extern "C" {
    bool UWP_HasFolder();
    const char* UWP_GetFolderPath();
}

// Global Lua state
static lua_State* g_luaState = nullptr;
static std::mutex g_luaMutex;
static std::map<std::string, std::function<int(lua_State*)>> g_luaFunctions;

// Helper function to load a Lua file
static int LoadLuaFile(lua_State* L, const char* filename) {
    if (!UWP_HasFolder()) {
        lua_pushstring(L, "No folder selected for Lua scripts");
        return lua_error(L);
    }
    
    std::string fullPath = std::string(UWP_GetFolderPath()) + "\\" + filename;
    
    int result = luaL_loadfile(L, fullPath.c_str());
    if (result != LUA_OK) {
        CONS_Printf("Error loading Lua script %s: %s\n", filename, lua_tostring(L, -1));
        return result;
    }
    
    result = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (result != LUA_OK) {
        CONS_Printf("Error executing Lua script %s: %s\n", filename, lua_tostring(L, -1));
        return result;
    }
    
    return LUA_OK;
}

// Custom Lua loader for UWP
static int UWP_LuaLoader(lua_State* L) {
    const char* name = lua_tostring(L, 1);
    if (!name) {
        lua_pushstring(L, "invalid module name");
        return 1;
    }
    
    // Try to load from external storage
    if (UWP_HasFolder()) {
        std::string modulePath = std::string(name);
        std::replace(modulePath.begin(), modulePath.end(), '.', '/');
        
        // Try with .lua extension
        std::string fullPath = std::string(UWP_GetFolderPath()) + "\\" + modulePath + ".lua";
        
        FILE* f = fopen(fullPath.c_str(), "rb");
        if (f) {
            fclose(f);
            if (LoadLuaFile(L, (modulePath + ".lua").c_str()) == LUA_OK) {
                // Return the loaded module
                if (lua_isnil(L, -1)) {
                    lua_pop(L, 1);
                    lua_pushboolean(L, 1);  // Return true instead of nil
                }
                return 1;
            }
        }
        
        // Try with /init.lua
        fullPath = std::string(UWP_GetFolderPath()) + "\\" + modulePath + "/init.lua";
        f = fopen(fullPath.c_str(), "rb");
        if (f) {
            fclose(f);
            if (LoadLuaFile(L, (modulePath + "/init.lua").c_str()) == LUA_OK) {
                // Return the loaded module
                if (lua_isnil(L, -1)) {
                    lua_pop(L, 1);
                    lua_pushboolean(L, 1);  // Return true instead of nil
                }
                return 1;
            }
        }
    }
    
    // Module not found, let Lua try the next loader
    lua_pushstring(L, "\n\tno file in UWP storage");
    return 1;
}

// Register a C function with Lua
bool UWP_Lua_RegisterFunction(const char* name, std::function<int(lua_State*)> func) {
    std::lock_guard<std::mutex> lock(g_luaMutex);
    
    if (!g_luaState) {
        return false;
    }
    
    g_luaFunctions[name] = func;
    
    lua_pushcfunction(g_luaState, [](lua_State* L) -> int {
        // Get function name from upvalue
        const char* funcName = lua_tostring(L, lua_upvalueindex(1));
        if (!funcName) {
            return luaL_error(L, "Invalid function name");
        }
        
        // Find the function in our map
        auto it = g_luaFunctions.find(funcName);
        if (it == g_luaFunctions.end()) {
            return luaL_error(L, "Function %s not registered", funcName);
        }
        
        // Call the actual function
        return it->second(L);
    });
    lua_setglobal(g_luaState, name);
    
    return true;
}

// Initialize Lua for UWP
extern "C" {
    bool UWP_Lua_Init() {
        std::lock_guard<std::mutex> lock(g_luaMutex);
        
        // Close any existing Lua state
        if (g_luaState) {
            lua_close(g_luaState);
            g_luaState = nullptr;
        }
        
        // Create a new Lua state
        g_luaState = luaL_newstate();
        if (!g_luaState) {
            I_Error("Failed to create Lua state");
            return false;
        }
        
        // Open standard libraries
        luaL_openlibs(g_luaState);
        
        // Add our custom module loader
        // Push the current package.loaders table
        lua_getglobal(g_luaState, "package");
        lua_getfield(g_luaState, -1, "searchers");  // In Lua 5.2+ it's 'searchers' instead of 'loaders'
        
        // Check if we got the searchers table
        if (!lua_istable(g_luaState, -1)) {
            // Try the older 'loaders' name for Lua 5.1
            lua_pop(g_luaState, 1);  // Pop the non-table value
            lua_getfield(g_luaState, -1, "loaders");
            
            if (!lua_istable(g_luaState, -1)) {
                lua_pop(g_luaState, 2);  // Pop both 'package' and the non-table value
                I_Error("Failed to find package.searchers or package.loaders");
                return false;
            }
        }
        
        // Get the table size
        size_t tableSize = lua_rawlen(g_luaState, -1);
        
        // Move the existing loaders down
        for (int i = tableSize; i > 1; i--) {
            lua_rawgeti(g_luaState, -1, i);
            lua_rawseti(g_luaState, -2, i + 1);
        }
        
        // Insert our loader at position 2 (after the default file loader)
        lua_pushcfunction(g_luaState, UWP_LuaLoader);
        lua_rawseti(g_luaState, -2, 2);
        
        // Pop the loaders table and package table
        lua_pop(g_luaState, 2);
        
        // Set some global helper values for UWP
        lua_pushboolean(g_luaState, 1);
        lua_setglobal(g_luaState, "UWP");
        
        lua_pushboolean(g_luaState, 1);
        lua_setglobal(g_luaState, "XBOX");
        
        CONS_Printf("Lua initialized for UWP/Xbox\n");
        return true;
    }
    
    // Shutdown Lua
    void UWP_Lua_Shutdown() {
        std::lock_guard<std::mutex> lock(g_luaMutex);
        
        if (g_luaState) {
            lua_close(g_luaState);
            g_luaState = nullptr;
        }
        
        g_luaFunctions.clear();
    }
    
    // Run a Lua script by filename
    bool UWP_Lua_RunFile(const char* filename) {
        std::lock_guard<std::mutex> lock(g_luaMutex);
        
        if (!g_luaState) {
            I_Error("Lua not initialized");
            return false;
        }
        
        if (LoadLuaFile(g_luaState, filename) != LUA_OK) {
            return false;
        }
        
        return true;
    }
    
    // Run Lua code from a string
    bool UWP_Lua_RunString(const char* code) {
        std::lock_guard<std::mutex> lock(g_luaMutex);
        
        if (!g_luaState) {
            I_Error("Lua not initialized");
            return false;
        }
        
        int result = luaL_loadstring(g_luaState, code);
        if (result != LUA_OK) {
            CONS_Printf("Error loading Lua code: %s\n", lua_tostring(g_luaState, -1));
            lua_pop(g_luaState, 1);
            return false;
        }
        
        result = lua_pcall(g_luaState, 0, LUA_MULTRET, 0);
        if (result != LUA_OK) {
            CONS_Printf("Error executing Lua code: %s\n", lua_tostring(g_luaState, -1));
            lua_pop(g_luaState, 1);
            return false;
        }
        
        return true;
    }
    
    // Get Lua state for direct access
    lua_State* UWP_Lua_GetState() {
        return g_luaState;
    }
    
    // Create a Lua table
    void UWP_Lua_CreateTable(const char* tableName) {
        std::lock_guard<std::mutex> lock(g_luaMutex);
        
        if (!g_luaState) {
            I_Error("Lua not initialized");
            return;
        }
        
        lua_newtable(g_luaState);
        lua_setglobal(g_luaState, tableName);
    }
    
    // Register BLUA hooks
    void UWP_Lua_RegisterHook(const char* hookName, void* function) {
        std::lock_guard<std::mutex> lock(g_luaMutex);
        
        if (!g_luaState) {
            I_Error("Lua not initialized");
            return;
        }
        
        // Get the hooks table
        lua_getglobal(g_luaState, "hooks");
        if (!lua_istable(g_luaState, -1)) {
            lua_pop(g_luaState, 1);
            lua_newtable(g_luaState);
            lua_pushvalue(g_luaState, -1);
            lua_setglobal(g_luaState, "hooks");
        }
        
        // Register the hook
        lua_pushstring(g_luaState, hookName);
        lua_pushlightuserdata(g_luaState, function);
        lua_settable(g_luaState, -3);
        
        lua_pop(g_luaState, 1);  // Pop the hooks table
    }
    
    // Call a Lua hook
    bool UWP_Lua_CallHook(const char* hookName, int numArgs) {
        std::lock_guard<std::mutex> lock(g_luaMutex);
        
        if (!g_luaState) {
            return false;
        }
        
        // Get the hook function from the global registry
        lua_getglobal(g_luaState, "hookfuncs");
        if (!lua_istable(g_luaState, -1)) {
            lua_pop(g_luaState, 1);
            return false;
        }
        
        lua_getfield(g_luaState, -1, hookName);
        if (!lua_isfunction(g_luaState, -1)) {
            lua_pop(g_luaState, 2);  // Pop the table and non-function
            return false;
        }
        
        // Move the function before the arguments
        lua_insert(g_luaState, -(numArgs + 1));
        // Remove the hook table
        lua_remove(g_luaState, -(numArgs + 2));
        
        // Call the function
        int result = lua_pcall(g_luaState, numArgs, 1, 0);
        if (result != LUA_OK) {
            CONS_Printf("Error calling Lua hook %s: %s\n", hookName, lua_tostring(g_luaState, -1));
            lua_pop(g_luaState, 1);  // Pop the error message
            return false;
        }
        
        // Check the return value
        bool ret = lua_toboolean(g_luaState, -1);
        lua_pop(g_luaState, 1);  // Pop the return value
        
        return ret;
    }
}