export interface Position {
  x: number;
  y: number;
}

export interface Size {
  width: number;
  height: number;
}

export interface PanelConfig {
  id: string;
  visible: boolean;
  position: Position;
  size: Size;
  minimized: boolean;
}

export interface OverlaySettings {
  theme: 'dark' | 'light';
  opacity: number;
  alwaysOnTop: boolean;
  clickThrough: boolean;
  compactMode: boolean;
  panels: {
    [key: string]: PanelConfig;
  };
}

export interface ItemData {
  name: string;
  type: string;
  rarity: 'normal' | 'magic' | 'rare' | 'unique';
  stats: string[];
  mods: string[];
  corrupted: boolean;
  quality?: number;
  level?: number;
}

export interface PriceData {
  item: string;
  currency: string;
  amount: number;
  confidence: number;
  listings: number;
  source: string;
}

export interface BuildGuide {
  id: string;
  name: string;
  class: string;
  ascendancy: string;
  skills: string[];
  items: string[];
  url: string;
}

export interface Hotkey {
  id: string;
  name: string;
  description: string;
  key: string;
  modifiers: string[];
  action: () => void;
}

export type PanelType =
  | 'search'
  | 'info'
  | 'price'
  | 'build'
  | 'settings'
  | 'hotkeys'
  | 'grafts'
  | 'bloodline';
