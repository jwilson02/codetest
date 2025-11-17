# POE Trade Overlay - Architecture Summary

## Overview
A desktop overlay application for Path of Exile that provides real-time price checking, trade search, and market analysis by monitoring clipboard activity and integrating with the official POE Trade API.

## Project Structure

```
/home/user/codetest/
├── src/
│   ├── index.js                    # Main Electron entry point
│   ├── App.jsx                     # Main React application
│   ├── services/
│   │   ├── clipboardMonitor.js     # Clipboard monitoring service
│   │   ├── itemParser.js           # POE item parser
│   │   ├── tradeService.js         # POE Trade API integration
│   │   └── priceChecker.js         # Price checking functionality
│   └── components/
│       ├── PriceChecker.jsx        # Price display component
│       ├── TradeSearch.jsx         # Trade search component
│       ├── CurrencyExchange.jsx    # Currency exchange component
│       └── WhisperGenerator.jsx    # Trade whisper generator
├── public/
│   └── index.html                  # Electron window template
└── package.json                    # Project dependencies
```

## Core Components

### 1. Clipboard Monitor (`clipboardMonitor.js`)
**Purpose**: Monitors system clipboard for POE item data

**Key Features**:
- Polls clipboard every 500ms for changes
- Detects POE items by format markers (Rarity:, Item Class:, etc.)
- Triggers callbacks when POE item is detected
- Can be started/stopped on demand

**Usage**:
```javascript
const monitor = new ClipboardMonitor();
monitor.start();
monitor.onItemDetected((itemText) => {
  // Process POE item
});
```

### 2. Item Parser (`itemParser.js`)
**Purpose**: Parses POE item clipboard format into structured data

**Parsing Capabilities**:
- ✓ Rarity (Normal, Magic, Rare, Unique, Currency, Gem, etc.)
- ✓ Item name and base type
- ✓ Item class/category
- ✓ Item level, quality
- ✓ Sockets and links (R-G-B patterns)
- ✓ Requirements (level, stats)
- ✓ Mods/Affixes (implicit, explicit, crafted, fractured, enchant)
- ✓ Numeric value extraction from mods
- ✓ Influences (Shaper, Elder, Crusader, etc.)
- ✓ Corrupted/Mirrored status
- ✓ Stat aggregation (life, ES, resistances)

**Output Format**:
```javascript
{
  rarity: 'Rare',
  name: 'Gale Knuckle',
  baseName: 'Titanium Spirit Shield',
  itemLevel: 85,
  corrupted: false,
  sockets: { R: 2, G: 1, B: 0, W: 0, total: 3 },
  links: 3,
  mods: {
    implicit: [...],
    explicit: [...],
    crafted: [...]
  },
  stats: {
    life: 75,
    energyShield: 120,
    fireRes: 45
  }
}
```

### 3. Trade Service (`tradeService.js`)
**Purpose**: Integrates with official POE Trade API

**API Integration**:
- Base URL: `https://www.pathofexile.com/api/trade`
- Endpoints:
  - `/search/{league}` - Search for items
  - `/fetch/{ids}` - Fetch specific listings
  - `/exchange/{league}` - Currency exchange rates

**Features**:
- Rate limiting (1 second between requests)
- Response caching (5 minute TTL)
- Automatic query building from parsed items
- Handles item filters (links, ilvl, corrupted, influences)
- Price extraction and statistics

**Query Building**:
- Uniques: Match by name + base type
- Rares: Match by base type + key mods
- Sockets/Links: Filter by minimum values
- Influences: Apply influence filters
- Item level: Match exact or range

### 4. Price Checker (`priceChecker.js`)
**Purpose**: Combines parsing and API to provide price estimates

**Functionality**:
- Automatic item categorization (equipment, currency, gem, map, card)
- Price range calculation (min, max, avg, median)
- Confidence scoring based on market data
- Price recommendations (quick sell, fair, high)
- Trade whisper generation
- Price history tracking

**Price Recommendation Algorithm**:
```javascript
{
  quick_sell: min * 0.9,    // Sell fast
  fair_price: median,        // Market rate
  high_price: avg,           // Patience required
  confidence: 'high/medium/low',
  notes: [...]
}
```

## React Components

### PriceChecker.jsx
Displays comprehensive price information:
- Min/Max/Avg/Median prices
- Total listings count
- Price recommendations
- Similar listings
- Item details panel

### TradeSearch.jsx
Advanced item search interface:
- Text-based search
- Filters (price range, links, ilvl, corrupted)
- Online-only toggle
- Results display

### CurrencyExchange.jsx
Currency exchange rates:
- Select currency pairs
- Live exchange rates
- Stock information
- Quick reference rates

### WhisperGenerator.jsx
Trade whisper creation:
- Auto-generate from listings
- Template support
- One-click copy to clipboard
- Trading tips

## Electron Integration

### Main Process (`index.js`)
- Creates frameless, transparent overlay window
- Always-on-top functionality
- Initializes services
- Handles IPC communication

### IPC Handlers
- `set-league` - Change active league
- `manual-price-check` - Trigger price check
- `get-price-history` - Retrieve price history
- `search-items` - Search trade API
- `get-currency-exchange` - Get exchange rates

## Data Flow

1. **Clipboard Detection**:
   ```
   User copies item (Ctrl+C in POE)
   → ClipboardMonitor detects change
   → Validates POE item format
   → Triggers callback
   ```

2. **Price Check**:
   ```
   Item text received
   → ItemParser parses structure
   → PriceChecker categorizes item
   → TradeService searches API
   → Results aggregated & displayed
   ```

3. **UI Update**:
   ```
   IPC message to renderer
   → App state updated
   → PriceChecker component renders
   → User sees price data
   ```

## Key Technical Decisions

### Why Electron?
- Native clipboard access
- Overlay capability (always-on-top, transparent)
- Cross-platform support
- React integration

### Why Client-Side Parsing?
- No server needed
- Instant feedback
- Privacy (no data sent externally)
- Works offline for parsing

### Rate Limiting Strategy
- 1 second between API requests
- 5 minute cache TTL
- Prevents API bans
- Improves responsiveness

### Caching Approach
- In-memory Map-based cache
- Per-item cache keys
- Time-based invalidation
- Reduces API load

## POE Item Format Notes

POE items use a specific clipboard format:
```
Rarity: Rare
Gale Knuckle
Titanium Spirit Shield
--------
Quality: +20% (augmented)
Chance to Block: 25%
Energy Shield: 120 (augmented)
--------
Requirements:
Level: 68
Int: 159
--------
Sockets: R-G-B
--------
Item Level: 85
--------
+75 to maximum Life (crafted)
+45% to Fire Resistance
+38% to Cold Resistance
--------
Corrupted
```

Key markers:
- `Rarity:` - First line, indicates tier
- `--------` - Section separators
- `(implicit)`, `(crafted)`, `(fractured)` - Mod types
- `Corrupted`, `Mirrored` - Special states
- `Sockets:` - Format like `R-G-B` or `R-R-R B-B-B`

## API Rate Limits

Official POE Trade API limits:
- ~1 request per second recommended
- Stricter limits for search vs fetch
- IP-based rate limiting
- User-Agent required

## Future Enhancements

Potential improvements:
- [ ] Stat weight calculation for rares
- [ ] Historical price graphs
- [ ] Bulk item price checking
- [ ] Custom price alerts
- [ ] Stash tab integration
- [ ] Trade offer tracking
- [ ] Market trend analysis
- [ ] Profile/build integration

## Testing the System

### Manual Test Flow:
1. Start overlay: `npm start`
2. Copy POE item from game
3. Observe automatic price check
4. Verify parsed data accuracy
5. Check API response caching
6. Test different item types

### Test Items:
- Unique items (well-known pricing)
- Rare items (complex mod parsing)
- Currency (bulk exchange)
- Gems (quality/level variants)
- Maps (tier/quality/corruption)

## Dependencies

Core libraries:
- `electron` - Desktop framework
- `react` - UI framework
- `axios` - HTTP client for API
- `electron-store` - Settings persistence

## Configuration

Settings to expose:
- Active league
- Price check hotkey
- Overlay position/size
- Cache duration
- API rate limits
- Price confidence thresholds

## Performance Considerations

- Clipboard polling: 500ms is balance of responsiveness vs CPU
- API caching: Reduces repeated searches significantly
- Lazy component rendering: Only active view rendered
- Debounced search: Prevent spam during typing

## Security & Privacy

- All processing client-side
- No item data stored externally
- API requests to official POE servers only
- No authentication required
- Local cache only

---

**Implementation Complete**: All core features for POE item parsing, clipboard monitoring, and trade API integration have been implemented following the requirements.
