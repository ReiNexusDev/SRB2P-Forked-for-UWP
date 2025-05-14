#include <windows.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Input.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <thread>

// Forward declarations of SRB2 functions that we need to call
extern "C" {
    int srb2_main(int argc, char *argv[]);
    void I_QuitGame(void);
}

// Global variables
bool g_windowClosed = false;
bool g_windowVisible = true;

using namespace winrt;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::UI::Core;
using namespace Windows::UI::Composition;
using namespace Windows::UI::Input;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;

// Xbox UWP Application class
class SRB2PApp : public implements<SRB2PApp, IFrameworkViewSource, IFrameworkView>
{
public:
    // IFrameworkViewSource methods
    IFrameworkView CreateView()
    {
        return *this;
    }

    // IFrameworkView methods
    void Initialize(CoreApplicationView const& applicationView)
    {
        // Register event handlers for app lifecycle
        applicationView.Activated({ this, &SRB2PApp::OnActivated });
        CoreApplication::Suspending({ this, &SRB2PApp::OnSuspending });
        CoreApplication::Resuming({ this, &SRB2PApp::OnResuming });
    }

    void SetWindow(CoreWindow const& window)
    {
        // Register window event handlers
        window.SizeChanged({ this, &SRB2PApp::OnWindowSizeChanged });
        window.VisibilityChanged({ this, &SRB2PApp::OnVisibilityChanged });
        window.Closed({ this, &SRB2PApp::OnWindowClosed });

        // Register pointer and keyboard event handlers
        auto dispatcher = CoreWindow::GetForCurrentThread().Dispatcher();
        dispatcher.AcceleratorKeyActivated({ this, &SRB2PApp::OnAcceleratorKeyActivated });

        auto currentDisplayInformation = Windows::Graphics::Display::DisplayInformation::GetForCurrentView();
    }

    void Load(winrt::hstring const& /* entryPoint */)
    {
        // Load resources or other initialization tasks
    }

    void Run()
    {
        // Start the SRB2 main function in a separate thread
        std::thread srb2Thread([this]() 
        {
            // Prepare command line args for SRB2
            char* argv[] = { const_cast<char*>("SRB2P.exe"), nullptr };
            srb2_main(1, argv);
        });

        // Main UWP message loop
        CoreWindow window = CoreWindow::GetForCurrentThread();
        CoreDispatcher dispatcher = window.Dispatcher();
        
        // Process events until our window closes
        while (!g_windowClosed)
        {
            dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
        }

        // Cleanup
        I_QuitGame();
        if (srb2Thread.joinable())
            srb2Thread.join();
    }

    void Uninitialize()
    {
        // Cleanup resources
    }

    // Event handlers
    void OnActivated(CoreApplicationView const& /* applicationView */, 
                    Windows::ApplicationModel::Activation::IActivatedEventArgs const& /* args */)
    {
        CoreWindow::GetForCurrentThread().Activate();
    }

    void OnSuspending(IInspectable const& /* sender */, 
                     Windows::ApplicationModel::SuspendingEventArgs const& args)
    {
        auto deferral = args.SuspendingOperation().GetDeferral();
        // Save game state and suspend game

        deferral.Complete();
    }

    void OnResuming(IInspectable const& /* sender */, IInspectable const& /* args */)
    {
        // Resume game from suspended state
    }

    void OnWindowSizeChanged(CoreWindow const& sender, WindowSizeChangedEventArgs const& /* args */)
    {
        // Update game to reflect new window size
    }

    void OnVisibilityChanged(CoreWindow const& /* sender */, VisibilityChangedEventArgs const& args)
    {
        g_windowVisible = args.Visible();
    }

    void OnWindowClosed(CoreWindow const& /* sender */, CoreWindowEventArgs const& /* args */)
    {
        g_windowClosed = true;
    }

    void OnAcceleratorKeyActivated(CoreDispatcher const& /* sender */, 
                                  AcceleratorKeyEventArgs const& args)
    {
        // Handle special key combinations (e.g., Alt+Enter for fullscreen toggle)
        if (args.EventType() == CoreAcceleratorKeyEventType::SystemKeyDown)
        {
            // Handle system keys
        }
    }
};

// Main entry point for UWP apps
int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    // Initialize COM
    winrt::init_apartment();
    
    // Create and run the app
    CoreApplication::Run(winrt::make<SRB2PApp>());
    
    return 0;
}