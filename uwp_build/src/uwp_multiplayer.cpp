#include <windows.h>
#include <winrt/Windows.Networking.Sockets.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Networking.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <string>
#include <vector>
#include <mutex>
#include <map>
#include <functional>
#include <sstream>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Networking;
using namespace Windows::Networking::Sockets;
using namespace Windows::Storage::Streams;

// Forward declarations for SRB2 functions
extern "C" {
    void CONS_Printf(const char* fmt, ...);
    void I_Error(const char* error, ...);
}

// Define callback types for networking
typedef void (*PacketReceivedCallback)(int playerNum, const void* data, size_t size);
typedef void (*PlayerJoinedCallback)(int playerNum, const char* playerName);
typedef void (*PlayerLeftCallback)(int playerNum);

// Global networking state
static StreamSocketListener g_serverSocket = nullptr;
static std::map<int, StreamSocket> g_clientSockets;
static StreamSocket g_hostSocket = nullptr;
static int g_localPlayerNum = 0;
static bool g_isHost = false;
static std::mutex g_socketMutex;
static PacketReceivedCallback g_packetCallback = nullptr;
static PlayerJoinedCallback g_playerJoinedCallback = nullptr;
static PlayerLeftCallback g_playerLeftCallback = nullptr;
static std::map<int, std::vector<uint8_t>> g_receiveBuffers;
static const int MAX_PLAYERS = 16;

// Packet header structure
#pragma pack(push, 1)
struct PacketHeader {
    uint32_t size;       // Total size including header
    uint8_t playerNum;   // Source player number
    uint8_t packetType;  // Type of packet (game data, join, leave, etc.)
};
#pragma pack(pop)

enum PacketType {
    PACKET_GAMEDATA = 0,
    PACKET_JOIN = 1,
    PACKET_LEAVE = 2,
    PACKET_PING = 3
};

// Helper function to send data through a socket
IAsyncAction SendDataAsync(StreamSocket socket, const void* data, size_t size) {
    try {
        DataWriter writer(socket.OutputStream());
        writer.WriteBytes(array_view<const uint8_t>(static_cast<const uint8_t*>(data), size));
        co_await writer.StoreAsync();
        co_await writer.FlushAsync();
        writer.DetachStream();
    }
    catch (const hresult_error& ex) {
        std::stringstream ss;
        ss << "Send error: " << to_string(ex.message()).c_str();
        CONS_Printf(ss.str().c_str());
    }
}

// Process received data
void ProcessReceivedData(int playerNum, const uint8_t* data, size_t size) {
    if (size < sizeof(PacketHeader)) {
        return; // Invalid packet
    }

    const PacketHeader* header = reinterpret_cast<const PacketHeader*>(data);
    
    // Verify packet size
    if (header->size != size) {
        return; // Corrupted packet
    }
    
    // Process packet based on type
    switch (header->packetType) {
        case PACKET_GAMEDATA:
            if (g_packetCallback) {
                g_packetCallback(header->playerNum, data + sizeof(PacketHeader), size - sizeof(PacketHeader));
            }
            break;
            
        case PACKET_JOIN:
            if (g_playerJoinedCallback) {
                // Extract player name from packet
                std::string playerName(reinterpret_cast<const char*>(data + sizeof(PacketHeader)), 
                                       size - sizeof(PacketHeader));
                g_playerJoinedCallback(header->playerNum, playerName.c_str());
            }
            break;
            
        case PACKET_LEAVE:
            if (g_playerLeftCallback) {
                g_playerLeftCallback(header->playerNum);
            }
            break;
            
        case PACKET_PING:
            // Respond to pings if we're the host
            if (g_isHost) {
                // Send ping response back to the sender
                std::lock_guard<std::mutex> lock(g_socketMutex);
                
                if (g_clientSockets.find(header->playerNum) != g_clientSockets.end()) {
                    SendDataAsync(g_clientSockets[header->playerNum], data, size);
                }
            }
            break;
            
        default:
            break;
    }
}

// Socket connection event handler
void OnClientConnected(StreamSocketListener sender, StreamSocketListenerConnectionReceivedEventArgs args) {
    try {
        std::lock_guard<std::mutex> lock(g_socketMutex);
        
        // Find a free player slot
        int playerNum = 1; // Start at 1 since host is player 0
        while (g_clientSockets.find(playerNum) != g_clientSockets.end() && playerNum < MAX_PLAYERS) {
            playerNum++;
        }
        
        if (playerNum >= MAX_PLAYERS) {
            // Server full, reject connection
            args.Socket().Close();
            return;
        }
        
        // Store the client socket
        g_clientSockets[playerNum] = args.Socket();
        
        // Set up data reader for this client
        auto socket = args.Socket();
        auto dataReader = DataReader(socket.InputStream());
        dataReader.InputStreamOptions(InputStreamOptions::Partial);
        
        // Start reading from this client
        auto readTask = [dataReader, playerNum, socket]() -> IAsyncAction {
            try {
                std::vector<uint8_t>& buffer = g_receiveBuffers[playerNum];
                
                while (true) {
                    // Try to read the packet header first
                    uint32_t bytesLoaded = co_await dataReader.LoadAsync(sizeof(PacketHeader));
                    
                    if (bytesLoaded == 0) {
                        // Connection closed
                        break;
                    }
                    
                    if (bytesLoaded < sizeof(PacketHeader)) {
                        // Incomplete header, wait for more data
                        continue;
                    }
                    
                    // Read the packet header
                    PacketHeader header;
                    dataReader.ReadBytes(array_view<uint8_t>(reinterpret_cast<uint8_t*>(&header), sizeof(header)));
                    
                    // Calculate remaining bytes to read
                    uint32_t remainingBytes = header.size - sizeof(PacketHeader);
                    bytesLoaded = co_await dataReader.LoadAsync(remainingBytes);
                    
                    if (bytesLoaded < remainingBytes) {
                        // Incomplete packet, discard
                        continue;
                    }
                    
                    // Read packet body
                    buffer.resize(header.size);
                    memcpy(buffer.data(), &header, sizeof(header));
                    dataReader.ReadBytes(array_view<uint8_t>(buffer.data() + sizeof(header), remainingBytes));
                    
                    // Process the packet
                    ProcessReceivedData(playerNum, buffer.data(), buffer.size());
                }
                
                // Client disconnected
                {
                    std::lock_guard<std::mutex> lock(g_socketMutex);
                    g_clientSockets.erase(playerNum);
                    g_receiveBuffers.erase(playerNum);
                }
                
                // Notify game of player disconnect
                if (g_playerLeftCallback) {
                    g_playerLeftCallback(playerNum);
                }
                
                CONS_Printf("Player %d disconnected\n", playerNum);
            }
            catch (const hresult_error& ex) {
                std::stringstream ss;
                ss << "Read error from player " << playerNum << ": " << to_string(ex.message()).c_str();
                CONS_Printf(ss.str().c_str());
                
                // Clean up this client's resources
                std::lock_guard<std::mutex> lock(g_socketMutex);
                g_clientSockets.erase(playerNum);
                g_receiveBuffers.erase(playerNum);
                
                // Notify game of player disconnect
                if (g_playerLeftCallback) {
                    g_playerLeftCallback(playerNum);
                }
            }
        }();
        
        CONS_Printf("Player %d connected\n", playerNum);
    }
    catch (const hresult_error& ex) {
        std::stringstream ss;
        ss << "Error accepting client: " << to_string(ex.message()).c_str();
        CONS_Printf(ss.str().c_str());
    }
}

// Client socket read handler
IAsyncAction ClientReadDataAsync() {
    try {
        auto dataReader = DataReader(g_hostSocket.InputStream());
        dataReader.InputStreamOptions(InputStreamOptions::Partial);
        std::vector<uint8_t> buffer;
        
        while (true) {
            // Try to read the packet header first
            uint32_t bytesLoaded = co_await dataReader.LoadAsync(sizeof(PacketHeader));
            
            if (bytesLoaded == 0) {
                // Connection closed
                break;
            }
            
            if (bytesLoaded < sizeof(PacketHeader)) {
                // Incomplete header, wait for more data
                continue;
            }
            
            // Read the packet header
            PacketHeader header;
            dataReader.ReadBytes(array_view<uint8_t>(reinterpret_cast<uint8_t*>(&header), sizeof(header)));
            
            // Calculate remaining bytes to read
            uint32_t remainingBytes = header.size - sizeof(PacketHeader);
            bytesLoaded = co_await dataReader.LoadAsync(remainingBytes);
            
            if (bytesLoaded < remainingBytes) {
                // Incomplete packet, discard
                continue;
            }
            
            // Read packet body
            buffer.resize(header.size);
            memcpy(buffer.data(), &header, sizeof(header));
            dataReader.ReadBytes(array_view<uint8_t>(buffer.data() + sizeof(header), remainingBytes));
            
            // Process the packet
            ProcessReceivedData(header.playerNum, buffer.data(), buffer.size());
        }
        
        // Server disconnected
        CONS_Printf("Disconnected from server\n");
        
        // Clean up all players
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (i != g_localPlayerNum && g_playerLeftCallback) {
                g_playerLeftCallback(i);
            }
        }
    }
    catch (const hresult_error& ex) {
        std::stringstream ss;
        ss << "Read error from server: " << to_string(ex.message()).c_str();
        CONS_Printf(ss.str().c_str());
    }
}

// Exported C functions for SRB2
extern "C" {
    // Initialize networking callbacks
    void UWP_Net_SetCallbacks(PacketReceivedCallback packetCb, PlayerJoinedCallback joinCb, PlayerLeftCallback leftCb) {
        g_packetCallback = packetCb;
        g_playerJoinedCallback = joinCb;
        g_playerLeftCallback = leftCb;
    }
    
    // Start a multiplayer server
    bool UWP_Net_StartServer(uint16_t port) {
        try {
            std::lock_guard<std::mutex> lock(g_socketMutex);
            
            // Close any existing server
            if (g_serverSocket) {
                g_serverSocket.Close();
                g_serverSocket = nullptr;
            }
            
            // Clear client connections
            for (auto& client : g_clientSockets) {
                try { client.second.Close(); } catch (...) {}
            }
            g_clientSockets.clear();
            g_receiveBuffers.clear();
            
            // Create a new listener
            g_serverSocket = StreamSocketListener();
            g_serverSocket.ConnectionReceived({ UWP_Net_StartServer, &OnClientConnected });
            
            // Bind to the specified port
            g_serverSocket.BindServiceNameAsync(to_hstring(std::to_string(port))).get();
            
            g_isHost = true;
            g_localPlayerNum = 0; // Host is always player 0
            
            CONS_Printf("Server started on port %d\n", port);
            return true;
        }
        catch (const hresult_error& ex) {
            std::stringstream ss;
            ss << "Failed to start server: " << to_string(ex.message()).c_str();
            I_Error(ss.str().c_str());
            return false;
        }
    }
    
    // Connect to a multiplayer server
    bool UWP_Net_ConnectToServer(const char* address, uint16_t port, const char* playerName) {
        try {
            std::lock_guard<std::mutex> lock(g_socketMutex);
            
            // Close any existing connection
            if (g_hostSocket) {
                g_hostSocket.Close();
                g_hostSocket = nullptr;
            }
            
            // Create a new socket
            g_hostSocket = StreamSocket();
            
            // Connect to the server
            g_hostSocket.ConnectAsync(
                HostName(to_hstring(address)), 
                to_hstring(std::to_string(port))
            ).get();
            
            g_isHost = false;
            g_localPlayerNum = 0; // Will be assigned by server
            
            // Start reading from server
            ClientReadDataAsync();
            
            // Send join packet
            size_t nameLen = strlen(playerName);
            size_t packetSize = sizeof(PacketHeader) + nameLen;
            std::vector<uint8_t> packet(packetSize);
            
            PacketHeader* header = reinterpret_cast<PacketHeader*>(packet.data());
            header->size = static_cast<uint32_t>(packetSize);
            header->playerNum = static_cast<uint8_t>(g_localPlayerNum);
            header->packetType = PACKET_JOIN;
            
            // Copy player name into packet
            memcpy(packet.data() + sizeof(PacketHeader), playerName, nameLen);
            
            // Send the packet
            SendDataAsync(g_hostSocket, packet.data(), packet.size());
            
            CONS_Printf("Connected to server %s:%d\n", address, port);
            return true;
        }
        catch (const hresult_error& ex) {
            std::stringstream ss;
            ss << "Failed to connect to server: " << to_string(ex.message()).c_str();
            I_Error(ss.str().c_str());
            return false;
        }
    }
    
    // Disconnect from server or stop hosting
    void UWP_Net_Disconnect() {
        std::lock_guard<std::mutex> lock(g_socketMutex);
        
        if (g_isHost) {
            // We're the host, send leave packets to all clients
            PacketHeader header;
            header.size = sizeof(header);
            header.playerNum = 0; // Host
            header.packetType = PACKET_LEAVE;
            
            for (auto& client : g_clientSockets) {
                try {
                    SendDataAsync(client.second, &header, sizeof(header));
                    client.second.Close();
                }
                catch (...) {}
            }
            
            g_clientSockets.clear();
            g_receiveBuffers.clear();
            
            if (g_serverSocket) {
                g_serverSocket.Close();
                g_serverSocket = nullptr;
            }
        }
        else {
            // We're a client, send leave packet to server
            if (g_hostSocket) {
                try {
                    PacketHeader header;
                    header.size = sizeof(header);
                    header.playerNum = static_cast<uint8_t>(g_localPlayerNum);
                    header.packetType = PACKET_LEAVE;
                    
                    SendDataAsync(g_hostSocket, &header, sizeof(header));
                    g_hostSocket.Close();
                }
                catch (...) {}
                
                g_hostSocket = nullptr;
            }
        }
        
        g_isHost = false;
    }
    
    // Send data to all players
    void UWP_Net_SendToAll(const void* data, size_t size) {
        if (!data || size == 0)
            return;
            
        std::lock_guard<std::mutex> lock(g_socketMutex);
        
        // Prepare packet with header
        size_t packetSize = sizeof(PacketHeader) + size;
        std::vector<uint8_t> packet(packetSize);
        
        PacketHeader* header = reinterpret_cast<PacketHeader*>(packet.data());
        header->size = static_cast<uint32_t>(packetSize);
        header->playerNum = static_cast<uint8_t>(g_localPlayerNum);
        header->packetType = PACKET_GAMEDATA;
        
        // Copy data into packet
        memcpy(packet.data() + sizeof(PacketHeader), data, size);
        
        if (g_isHost) {
            // Host sends to all clients
            for (auto& client : g_clientSockets) {
                SendDataAsync(client.second, packet.data(), packet.size());
            }
        }
        else {
            // Client sends only to host
            if (g_hostSocket) {
                SendDataAsync(g_hostSocket, packet.data(), packet.size());
            }
        }
    }
    
    // Check if we're connected or hosting
    bool UWP_Net_IsConnected() {
        std::lock_guard<std::mutex> lock(g_socketMutex);
        return g_isHost || g_hostSocket != nullptr;
    }
    
    // Get the local player number
    int UWP_Net_GetLocalPlayerNum() {
        return g_localPlayerNum;
    }
    
    // Set the local player number (called when server assigns it)
    void UWP_Net_SetLocalPlayerNum(int playerNum) {
        g_localPlayerNum = playerNum;
    }
    
    // Check if we're the host
    bool UWP_Net_IsHost() {
        return g_isHost;
    }
}