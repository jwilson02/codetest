import React from 'react';
import { useOverlay } from '@contexts/OverlayContext';
import SearchPanel from '@components/SearchPanel/SearchPanel';
import InfoPanel from '@components/InfoPanel/InfoPanel';
import PriceDisplay from '@components/PriceDisplay/PriceDisplay';
import BuildGuideViewer from '@components/BuildGuideViewer/BuildGuideViewer';
import CurrencyTracker from '@components/CurrencyTracker/CurrencyTracker';
import MapModChecker from '@components/MapModChecker/MapModChecker';
import SettingsPanel from '@components/SettingsPanel/SettingsPanel';
import HotkeyManager from '@components/HotkeyManager/HotkeyManager';
import './OverlayContainer.scss';

const OverlayContainer: React.FC = () => {
  const { settings, togglePanel } = useOverlay();

  return (
    <div
      className={`overlay-container ${settings.compactMode ? 'compact' : ''}`}
      style={{ opacity: settings.opacity }}
    >
      {settings.panels.search.visible && <SearchPanel />}
      {settings.panels.info.visible && <InfoPanel />}
      {settings.panels.price.visible && <PriceDisplay />}
      {settings.panels.build.visible && <BuildGuideViewer />}
      {settings.panels.currency.visible && (
        <CurrencyTracker onClose={() => togglePanel('currency')} />
      )}
      {settings.panels.mapMod.visible && (
        <MapModChecker onClose={() => togglePanel('mapMod')} />
      )}
      {settings.panels.settings.visible && <SettingsPanel />}
      {settings.panels.hotkeys.visible && <HotkeyManager />}
    </div>
  );
};

export default OverlayContainer;
