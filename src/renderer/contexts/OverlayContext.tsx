import React, { createContext, useContext, useState, useEffect, ReactNode } from 'react';
import { OverlaySettings, PanelConfig, Position, Size } from '@types/index';

interface OverlayContextType {
  settings: OverlaySettings;
  updateSettings: (settings: Partial<OverlaySettings>) => void;
  togglePanel: (panelId: string) => void;
  updatePanelPosition: (panelId: string, position: Position) => void;
  updatePanelSize: (panelId: string, size: Size) => void;
  togglePanelMinimize: (panelId: string) => void;
  resetLayout: () => void;
}

const defaultSettings: OverlaySettings = {
  theme: 'dark',
  opacity: 0.95,
  alwaysOnTop: true,
  clickThrough: false,
  compactMode: false,
  panels: {
    search: {
      id: 'search',
      visible: false,
      position: { x: 100, y: 100 },
      size: { width: 400, height: 500 },
      minimized: false,
    },
    info: {
      id: 'info',
      visible: false,
      position: { x: 520, y: 100 },
      size: { width: 350, height: 400 },
      minimized: false,
    },
    price: {
      id: 'price',
      visible: false,
      position: { x: 890, y: 100 },
      size: { width: 300, height: 350 },
      minimized: false,
    },
    build: {
      id: 'build',
      visible: false,
      position: { x: 100, y: 620 },
      size: { width: 500, height: 600 },
      minimized: false,
    },
    currency: {
      id: 'currency',
      visible: false,
      position: { x: 150, y: 150 },
      size: { width: 450, height: 650 },
      minimized: false,
    },
    mapMod: {
      id: 'mapMod',
      visible: false,
      position: { x: 200, y: 200 },
      size: { width: 550, height: 700 },
      minimized: false,
    },
    settings: {
      id: 'settings',
      visible: false,
      position: { x: 620, y: 520 },
      size: { width: 450, height: 550 },
      minimized: false,
    },
    hotkeys: {
      id: 'hotkeys',
      visible: false,
      position: { x: 300, y: 300 },
      size: { width: 400, height: 450 },
      minimized: false,
    },
  },
};

const OverlayContext = createContext<OverlayContextType | undefined>(undefined);

export const useOverlay = () => {
  const context = useContext(OverlayContext);
  if (!context) {
    throw new Error('useOverlay must be used within OverlayProvider');
  }
  return context;
};

interface OverlayProviderProps {
  children: ReactNode;
}

export const OverlayProvider: React.FC<OverlayProviderProps> = ({ children }) => {
  const [settings, setSettings] = useState<OverlaySettings>(() => {
    const saved = localStorage.getItem('poe-overlay-settings');
    return saved ? JSON.parse(saved) : defaultSettings;
  });

  useEffect(() => {
    localStorage.setItem('poe-overlay-settings', JSON.stringify(settings));
  }, [settings]);

  const updateSettings = (newSettings: Partial<OverlaySettings>) => {
    setSettings((prev) => ({ ...prev, ...newSettings }));
  };

  const togglePanel = (panelId: string) => {
    setSettings((prev) => ({
      ...prev,
      panels: {
        ...prev.panels,
        [panelId]: {
          ...prev.panels[panelId],
          visible: !prev.panels[panelId].visible,
        },
      },
    }));
  };

  const updatePanelPosition = (panelId: string, position: Position) => {
    setSettings((prev) => ({
      ...prev,
      panels: {
        ...prev.panels,
        [panelId]: {
          ...prev.panels[panelId],
          position,
        },
      },
    }));
  };

  const updatePanelSize = (panelId: string, size: Size) => {
    setSettings((prev) => ({
      ...prev,
      panels: {
        ...prev.panels,
        [panelId]: {
          ...prev.panels[panelId],
          size,
        },
      },
    }));
  };

  const togglePanelMinimize = (panelId: string) => {
    setSettings((prev) => ({
      ...prev,
      panels: {
        ...prev.panels,
        [panelId]: {
          ...prev.panels[panelId],
          minimized: !prev.panels[panelId].minimized,
        },
      },
    }));
  };

  const resetLayout = () => {
    setSettings(defaultSettings);
  };

  return (
    <OverlayContext.Provider
      value={{
        settings,
        updateSettings,
        togglePanel,
        updatePanelPosition,
        updatePanelSize,
        togglePanelMinimize,
        resetLayout,
      }}
    >
      {children}
    </OverlayContext.Provider>
  );
};
