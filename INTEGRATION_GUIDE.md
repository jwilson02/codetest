# POE Trade Overlay - Integration Guide

## How Components Work Together

### 1. Application Startup

```javascript
// Main Process (index.js)
app.whenReady() → createWindow() → initializeServices()
                                   ↓
                    ClipboardMonitor.start()
                    PriceChecker initialized with current league
```

### 2. Item Detection Flow

```
User copies item in POE (Ctrl+C)
    ↓
ClipboardMonitor detects change (500ms poll)
    ↓
Validates format: isPOEItem()
    ↓
Triggers callback: onItemDetected(itemText)
    ↓
Main process receives item text
```

### 3. Price Check Flow

```
Item text received
    ↓
PriceChecker.checkPrice(itemText)
    ↓
ItemParser.parseItem(itemText) → Structured item data
    ↓
Determine category (equipment, currency, gem, etc.)
    ↓
TradeService.searchItems(parsedItem) → Build API query
    ↓
Send to POE Trade API
    ↓
Receive search results (item IDs)
    ↓
TradeService.fetchListings(ids) → Get detailed listings
    ↓
Extract prices from listings
    ↓
Calculate statistics (min, max, avg, median)
    ↓
Generate recommendations (quick sell, fair, high)
    ↓
Return complete price data
```

### 4. UI Update Flow

```
Price data ready
    ↓
Main process sends IPC message
    ↓
mainWindow.webContents.send('price-check-result', result)
    ↓
Renderer process receives event
    ↓
App.jsx updates state
    ↓
PriceChecker component renders with data
    ↓
User sees price information
```

## Code Examples

### Example 1: Using Clipboard Monitor

```javascript
const ClipboardMonitor = require('./services/clipboardMonitor');

const monitor = new ClipboardMonitor();

// Register callback
monitor.onItemDetected(async (itemText) => {
  console.log('POE item detected!');
  // Process item...
});

// Start monitoring
monitor.start();

// Later, stop monitoring
monitor.stop();
```

### Example 2: Parsing an Item

```javascript
const ItemParser = require('./services/itemParser');

const parser = new ItemParser();

const itemText = `Rarity: Unique
Tabula Rasa
Simple Robe
--------
Sockets: W-W-W-W-W-W
--------
Item Level: 1`;

const parsed = parser.parseItem(itemText);

console.log(parsed.name);        // "Tabula Rasa"
console.log(parsed.rarity);      // "Unique"
console.log(parsed.links);       // 6
console.log(parsed.sockets);     // { W: 6, R: 0, G: 0, B: 0, total: 6 }
```

### Example 3: Checking Price

```javascript
const PriceChecker = require('./services/priceChecker');

const checker = new PriceChecker('Standard');

const result = await checker.checkPrice(itemText);

if (result.success) {
  const { item, priceData } = result;
  
  console.log(`Item: ${item.name}`);
  console.log(`Min Price: ${priceData.prices.min} ${priceData.prices.currency}`);
  console.log(`Avg Price: ${priceData.prices.avg} ${priceData.prices.currency}`);
  console.log(`Total Listings: ${priceData.total}`);
  
  if (priceData.recommendation) {
    console.log(`Quick Sell: ${priceData.recommendation.quick_sell}`);
    console.log(`Fair Price: ${priceData.recommendation.fair_price}`);
  }
}
```

### Example 4: Manual Trade Search

```javascript
const TradeService = require('./services/tradeService');

const trade = new TradeService('Standard');

// Search for item
const searchResult = await trade.searchItems(parsedItem);

console.log(`Found ${searchResult.total} listings`);

// Fetch first 10
const listings = await trade.fetchListings(
  searchResult.id,
  searchResult.result.slice(0, 10)
);

// Display listings
listings.forEach(listing => {
  const { price, account } = listing.listing;
  console.log(`${price.amount} ${price.currency} - @${account.name}`);
});
```

### Example 5: Currency Exchange

```javascript
const TradeService = require('./services/tradeService');

const trade = new TradeService('Standard');

// Get exchange rate: Chaos → Exalted
const result = await trade.getCurrencyExchange('exalted', 'chaos');

console.log(`Found ${result.result.length} exchange offers`);
```

## React Component Integration

### Using PriceChecker Component

```jsx
import PriceChecker from './components/PriceChecker';

function App() {
  const [priceData, setPriceData] = useState(null);
  
  return (
    <div>
      {priceData && (
        <PriceChecker
          item={priceData.item}
          priceData={priceData.priceData}
          onClose={() => setPriceData(null)}
        />
      )}
    </div>
  );
}
```

### Using TradeSearch Component

```jsx
import TradeSearch from './components/TradeSearch';

function App() {
  const [searchResults, setSearchResults] = useState(null);
  
  const handleSearch = async (searchParams) => {
    const results = await tradeService.search(searchParams);
    setSearchResults(results);
  };
  
  return (
    <TradeSearch
      onSearch={handleSearch}
      searchResults={searchResults}
    />
  );
}
```

## IPC Communication

### Main Process → Renderer

```javascript
// Main process
mainWindow.webContents.send('price-check-result', {
  success: true,
  item: parsedItem,
  priceData: priceData
});
```

### Renderer → Main Process

```javascript
// Renderer process
const result = await ipcRenderer.invoke('manual-price-check', itemText);

if (result.success) {
  // Display price data
}
```

## Available IPC Handlers

### `set-league`
```javascript
await ipcRenderer.invoke('set-league', 'Standard');
```

### `manual-price-check`
```javascript
const result = await ipcRenderer.invoke('manual-price-check', itemText);
```

### `get-price-history`
```javascript
const history = await ipcRenderer.invoke('get-price-history', itemName, baseName);
```

### `search-items`
```javascript
const results = await ipcRenderer.invoke('search-items', {
  searchTerm: 'Tabula Rasa',
  filters: { online: true }
});
```

### `get-currency-exchange`
```javascript
const exchange = await ipcRenderer.invoke('get-currency-exchange', 'exalted', 'chaos');
```

## Configuration

### Setting League

```javascript
const Config = require('./utils/config');
const config = new Config();

config.setLeague('Standard');
// or
config.setLeague('Hardcore');
// or
config.setLeague('Challenge'); // Current league
```

### Overlay Settings

```javascript
config.updateOverlaySettings({
  alwaysOnTop: true,
  opacity: 0.95,
  width: 400,
  height: 600
});
```

## Testing

### Test Parser

```bash
node test-parser.js
```

This will parse all test items and show results.

### Test Individual Items

```javascript
const { getTestItem } = require('./src/utils/testData');
const ItemParser = require('./src/services/itemParser');

const parser = new ItemParser();
const itemText = getTestItem('uniqueWeapon');
const parsed = parser.parseItem(itemText);

console.log(parsed);
```

## Error Handling

### Clipboard Monitor

```javascript
monitor.onItemDetected(async (itemText) => {
  try {
    await processItem(itemText);
  } catch (error) {
    console.error('Error processing item:', error);
  }
});
```

### Price Checker

```javascript
const result = await checker.checkPrice(itemText);

if (!result.success) {
  console.error('Price check failed:', result.error);
  return;
}

// Process successful result
```

### Trade Service

```javascript
try {
  const searchResult = await trade.searchItems(parsedItem);
} catch (error) {
  if (error.response && error.response.status === 429) {
    console.error('Rate limited! Wait before retrying.');
  } else {
    console.error('Search failed:', error.message);
  }
}
```

## Performance Tips

1. **Use Caching**: TradeService automatically caches responses for 5 minutes
2. **Rate Limiting**: Built-in 1-second delay between API requests
3. **Lazy Rendering**: Only render active view in App.jsx
4. **Clipboard Polling**: 500ms is optimal balance

## Security Considerations

1. All data processed locally
2. No authentication required
3. Only connects to official POE API
4. No data sent to third parties
5. Local cache only

## Troubleshooting

### Clipboard Not Detecting Items
- Ensure monitor is started: `monitor.start()`
- Check item format has POE markers
- Verify clipboard access permissions

### API Errors
- Check internet connection
- Verify league name is correct
- Ensure not rate limited (wait 1 second between requests)
- Check POE API status

### Parsing Issues
- Validate item format
- Check for special characters
- Ensure complete item text
- Test with provided test items

## Best Practices

1. **Always handle errors** in async operations
2. **Use caching** to reduce API calls
3. **Respect rate limits** to avoid bans
4. **Validate input** before parsing
5. **Clean up resources** when done (stop monitor, clear cache)

---

This integration guide shows how all components work together to create a complete POE trading experience!
