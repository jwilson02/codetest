import React from 'react';
import { useOverlay } from '@contexts/OverlayContext';
import SearchPanel from '@components/SearchPanel/SearchPanel';
import InfoPanel from '@components/InfoPanel/InfoPanel';
import PriceDisplay from '@components/PriceDisplay/PriceDisplay';
import BuildGuideViewer from '@components/BuildGuideViewer/BuildGuideViewer';
import CurrencyTracker from '@components/CurrencyTracker/CurrencyTracker';
import MapModChecker from '@components/MapModChecker/MapModChecker';
import CraftingHelper from '@components/CraftingHelper/CraftingHelper';
import BetrayalBoard from '@components/BetrayalBoard/BetrayalBoard';
import SettingsPanel from '@components/SettingsPanel/SettingsPanel';
import HotkeyManager from '@components/HotkeyManager/HotkeyManager';
import TradeWhisperManager from '@components/TradeWhisperManager/TradeWhisperManager';
import VendorRecipes from '@components/VendorRecipes/VendorRecipes';
import DivinationCardTracker from '@components/DivinationCardTracker/DivinationCardTracker';
import AtlasTracker from '@components/AtlasTracker/AtlasTracker';
import GraftsPlanner from '@components/GraftsPlanner/GraftsPlanner';
import BloodlineAscendancy from '@components/BloodlineAscendancy/BloodlineAscendancy';
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
      {settings.panels.crafting.visible && (
        <CraftingHelper onClose={() => togglePanel('crafting')} />
      )}
      {settings.panels.betrayal.visible && (
        <BetrayalBoard onClose={() => togglePanel('betrayal')} />
      )}
      {settings.panels.settings.visible && <SettingsPanel />}
      {settings.panels.hotkeys.visible && <HotkeyManager />}
      {settings.panels.tradeWhisper.visible && (
        <TradeWhisperManager onClose={() => togglePanel('tradeWhisper')} />
      )}
      {settings.panels.vendorRecipe.visible && (
        <VendorRecipes onClose={() => togglePanel('vendorRecipe')} />
      )}
      {settings.panels.divinationCards.visible && (
        <DivinationCardTracker onClose={() => togglePanel('divinationCards')} />
      )}
      {settings.panels.atlasTracker.visible && (
        <AtlasTracker onClose={() => togglePanel('atlasTracker')} />
      )}
      {settings.panels.grafts.visible && (
        <GraftsPlanner onClose={() => togglePanel('grafts')} />
      )}
      {settings.panels.bloodline.visible && (
        <BloodlineAscendancy onClose={() => togglePanel('bloodline')} />
      )}
    </div>
  );
};

export default OverlayContainer;
