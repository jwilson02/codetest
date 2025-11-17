import React, { useEffect } from 'react';
import { OverlayProvider, useOverlay } from '@contexts/OverlayContext';
import OverlayContainer from '@components/OverlayContainer/OverlayContainer';

// Declare the Electron API from preload script
declare global {
  interface Window {
    electronAPI?: {
      getClickThroughState: () => Promise<boolean>;
      setClickThrough: (enabled: boolean) => Promise<boolean>;
      toggleClickThrough: () => Promise<boolean>;
      onClickThroughChanged: (callback: (isClickThrough: boolean) => void) => void;
      getDisplayInfo: () => Promise<any>;
      getVersion: () => Promise<any>;
      readClipboard: () => Promise<string>;
      writeClipboard: (text: string) => Promise<boolean>;
      togglePanel: (panelId: string) => Promise<boolean>;
      onHotkeyPressed: (callback: (hotkeyName: string) => void) => void;
      onTogglePanel: (callback: (panelId: string) => void) => void;
      platform: string;
    };
    logger?: {
      info: (...args: any[]) => void;
      warn: (...args: any[]) => void;
      error: (...args: any[]) => void;
      debug: (...args: any[]) => void;
    };
  }
}

// Inner component to use overlay context
const AppContent: React.FC = () => {
  const { togglePanel } = useOverlay();

  useEffect(() => {
    // Listen for hotkey events from main process
    if (window.electronAPI?.onHotkeyPressed) {
      window.electronAPI.onHotkeyPressed((hotkeyName) => {
        window.logger?.info('Hotkey pressed:', hotkeyName);

        // Handle different hotkeys
        switch (hotkeyName) {
          case 'price-check':
            togglePanel('price');
            // Also trigger clipboard read and price check
            window.electronAPI?.readClipboard().then((text) => {
              window.logger?.info('Clipboard content for price check:', text);
              // The PriceDisplay component should handle the actual price check
            });
            break;
          case 'build-guide':
            togglePanel('build');
            break;
          case 'map-info':
            togglePanel('mapMod');
            break;
          case 'currency':
            togglePanel('currency');
            break;
          case 'toggle-overlay':
            // Already handled by main process visibility toggle
            window.logger?.info('Overlay visibility toggled');
            break;
          default:
            window.logger?.warn('Unknown hotkey:', hotkeyName);
        }
      });
    }

    // Listen for panel toggle events
    if (window.electronAPI?.onTogglePanel) {
      window.electronAPI.onTogglePanel((panelId) => {
        window.logger?.info('Panel toggle requested:', panelId);
        togglePanel(panelId);
      });
    }

    // Listen for click-through state changes
    if (window.electronAPI?.onClickThroughChanged) {
      window.electronAPI.onClickThroughChanged((isClickThrough) => {
        window.logger?.info('Click-through state changed:', isClickThrough);
      });
    }

    // Log app initialization
    window.logger?.info('POE Overlay initialized');

    // Get version info on startup
    if (window.electronAPI?.getVersion) {
      window.electronAPI.getVersion().then((version) => {
        window.logger?.info('Version info:', version);
      });
    }
  }, [togglePanel]);

  return <OverlayContainer />;
};

const App: React.FC = () => {
  return (
    <OverlayProvider>
      <AppContent />
    </OverlayProvider>
  );
};

export default App;
