#include <windows.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Windows.Storage.AccessCache.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <string>
#include <mutex>
#include <sstream>
#include <vector>
#include <future>
#include <ppltasks.h>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;
using namespace Windows::Storage::AccessCache;

// Forward declaration of SRB2 functions
extern "C" {
    void I_Error(const char* error, ...);
    void CONS_Printf(const char* fmt, ...);
}

// Global variable to store selected folder token
static std::string g_folderToken;
static StorageFolder g_selectedFolder = nullptr;
static std::mutex g_folderMutex;

// Function to check if we have access to a previously selected folder
bool UWP_HasStorageAccess()
{
    std::lock_guard<std::mutex> lock(g_folderMutex);
    if (g_folderToken.empty())
        return false;
    
    try {
        auto folder = StorageApplicationPermissions::FutureAccessList().GetFolderAsync(to_hstring(g_folderToken)).get();
        return (folder != nullptr);
    }
    catch (...) {
        return false;
    }
}

// Asynchronously picks a folder and returns its path
concurrency::task<std::string> UWP_PickFolderAsync()
{
    return concurrency::create_task([]() -> std::string {
        try {
            FolderPicker picker;
            picker.SuggestedStartLocation(PickerLocationId::DocumentsLibrary);
            picker.FileTypeFilter().Append(L"*");
            
            auto folder = picker.PickSingleFolderAsync().get();
            if (folder) {
                // Store folder for future access
                std::lock_guard<std::mutex> lock(g_folderMutex);
                g_selectedFolder = folder;
                g_folderToken = to_string(StorageApplicationPermissions::FutureAccessList().Add(folder));
                return to_string(folder.Path());
            }
        }
        catch (const hresult_error& ex) {
            std::stringstream ss;
            ss << "Error picking folder: " << to_string(ex.message()).c_str();
            I_Error(ss.str().c_str());
        }
        return "";
    });
}

// Function to get a path to a file in the selected folder
std::string UWP_GetPathInFolder(const char* relativePath)
{
    std::lock_guard<std::mutex> lock(g_folderMutex);
    if (g_selectedFolder == nullptr || g_folderToken.empty()) {
        I_Error("No folder selected for storage access");
        return "";
    }
    
    try {
        auto file = g_selectedFolder.GetFileAsync(to_hstring(relativePath)).get();
        if (file) {
            return to_string(file.Path());
        }
    }
    catch (...) {
        // File doesn't exist, return the path anyway
        return to_string(g_selectedFolder.Path()) + "\\" + relativePath;
    }
    
    return "";
}

// Function to save a file to the selected folder
bool UWP_SaveFileToFolder(const char* relativePath, const void* data, size_t dataSize)
{
    std::lock_guard<std::mutex> lock(g_folderMutex);
    if (g_selectedFolder == nullptr || g_folderToken.empty()) {
        I_Error("No folder selected for storage access");
        return false;
    }
    
    try {
        auto file = g_selectedFolder.CreateFileAsync(to_hstring(relativePath), CreationCollisionOption::ReplaceExisting).get();
        if (file) {
            auto stream = file.OpenAsync(FileAccessMode::ReadWrite).get();
            auto outputStream = stream.GetOutputStreamAt(0);
            auto dataWriter = Windows::Storage::Streams::DataWriter(outputStream);
            
            // Write the data
            dataWriter.WriteBytes(array_view<const uint8_t>(static_cast<const uint8_t*>(data), dataSize));
            dataWriter.StoreAsync().get();
            dataWriter.FlushAsync().get();
            dataWriter.Close();
            
            return true;
        }
    }
    catch (const hresult_error& ex) {
        std::stringstream ss;
        ss << "Error saving file: " << to_string(ex.message()).c_str();
        I_Error(ss.str().c_str());
    }
    
    return false;
}

// Function to load a file from the selected folder
std::vector<uint8_t> UWP_LoadFileFromFolder(const char* relativePath)
{
    std::lock_guard<std::mutex> lock(g_folderMutex);
    std::vector<uint8_t> fileData;
    
    if (g_selectedFolder == nullptr || g_folderToken.empty()) {
        I_Error("No folder selected for storage access");
        return fileData;
    }
    
    try {
        auto file = g_selectedFolder.GetFileAsync(to_hstring(relativePath)).get();
        if (file) {
            auto buffer = FileIO::ReadBufferAsync(file).get();
            auto dataReader = Windows::Storage::Streams::DataReader::FromBuffer(buffer);
            
            fileData.resize(buffer.Length());
            dataReader.ReadBytes(array_view<uint8_t>(fileData.data(), fileData.size()));
            dataReader.Close();
        }
    }
    catch (const hresult_error& ex) {
        std::stringstream ss;
        ss << "Error loading file: " << to_string(ex.message()).c_str();
        CONS_Printf(ss.str().c_str());
    }
    
    return fileData;
}

// Expose these functions to SRB2
extern "C" {
    // Pick a folder for storage (shows the Xbox file picker UI)
    void UWP_PickFolder()
    {
        auto task = UWP_PickFolderAsync();
        task.then([](std::string path) {
            if (!path.empty()) {
                CONS_Printf("Selected folder: %s\n", path.c_str());
            } else {
                CONS_Printf("No folder selected\n");
            }
        });
    }
    
    // Check if we have a storage folder selected
    bool UWP_HasFolder()
    {
        return UWP_HasStorageAccess();
    }
    
    // Get the path to the selected folder
    const char* UWP_GetFolderPath()
    {
        static std::string path;
        std::lock_guard<std::mutex> lock(g_folderMutex);
        
        if (g_selectedFolder == nullptr)
            return "";
            
        path = to_string(g_selectedFolder.Path());
        return path.c_str();
    }
    
    // Save data to a file in the selected folder
    bool UWP_SaveToExternalStorage(const char* filename, const void* data, size_t size)
    {
        return UWP_SaveFileToFolder(filename, data, size);
    }
    
    // Load data from a file in the selected folder
    void* UWP_LoadFromExternalStorage(const char* filename, size_t* outSize)
    {
        auto data = UWP_LoadFileFromFolder(filename);
        if (data.empty()) {
            *outSize = 0;
            return nullptr;
        }
        
        *outSize = data.size();
        void* buffer = malloc(data.size());
        if (buffer) {
            memcpy(buffer, data.data(), data.size());
        }
        return buffer;
    }
}