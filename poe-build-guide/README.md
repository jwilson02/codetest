# Path of Exile Build Guide - Maxroll.gg Integration

A comprehensive React application that integrates with Maxroll.gg to provide Path of Exile build guides, complete with skill gems, item recommendations, and passive skill tree information.

## Features

### Core Functionality
- **Build Browser**: Browse and filter Path of Exile builds from Maxroll.gg
- **Build Details**: View comprehensive build guides including:
  - Overview with pros/cons and stats
  - Gem link setups with support gems
  - Item recommendations with pricing
  - Passive skill tree information
- **Advanced Search**: Filter builds by class, ascendancy, difficulty, budget, and more
- **Data Caching**: Smart caching system for offline support and reduced API calls
- **Rate Limiting**: Respectful rate limiting to avoid overwhelming Maxroll.gg

### Architecture

```
poe-build-guide/
├── src/
│   ├── models/              # Data models
│   │   ├── Build.js         # Build data model
│   │   ├── Skill.js         # Skill gem data model
│   │   └── Item.js          # Item data model
│   ├── services/
│   │   ├── maxroll/         # Maxroll.gg integration services
│   │   │   ├── buildGuideService.js    # Fetch build guides
│   │   │   ├── skillDataService.js     # Fetch skill data
│   │   │   ├── itemDataService.js      # Fetch item data
│   │   │   └── searchService.js        # Search and filter
│   │   ├── cache/
│   │   │   └── cacheService.js         # Local caching system
│   │   └── utils/
│   │       └── rateLimiter.js          # Rate limiting
│   ├── components/
│   │   ├── BuildBrowser/    # Browse builds
│   │   ├── BuildDetail/     # Build detail view
│   │   ├── GemLinks/        # Gem setups display
│   │   ├── ItemRecommendations/  # Item display
│   │   └── SkillTree/       # Passive tree display
│   ├── App.jsx              # Main application
│   └── index.jsx            # Entry point
├── public/
└── package.json
```

## Installation

### Prerequisites
- Node.js 16+ and npm
- Modern web browser

### Setup

1. **Install dependencies**:
```bash
cd poe-build-guide
npm install
```

2. **Start the development server**:
```bash
npm start
```

The application will open at `http://localhost:3000`

3. **Build for production**:
```bash
npm run build
```

## Services Architecture

### 1. Build Guide Service (`buildGuideService.js`)

Fetches and parses build guides from Maxroll.gg.

**Key Methods**:
- `fetchAllBuilds()` - Get all available builds
- `fetchBuildGuide(buildId)` - Get detailed build information
- Automatic caching with TTL
- Rate limiting protection

**Usage**:
```javascript
import { buildGuideService } from './services/maxroll/buildGuideService';

// Fetch all builds
const builds = await buildGuideService.fetchAllBuilds();

// Fetch specific build
const build = await buildGuideService.fetchBuildGuide('poison-spark-pathfinder');
```

### 2. Skill Data Service (`skillDataService.js`)

Manages skill gem data and recommendations.

**Key Methods**:
- `fetchAllSkills()` - Get all skill gems
- `fetchSkill(skillName)` - Get specific skill data
- `searchSkills(criteria)` - Search skills by type, color, tags
- `getSkillRecommendations(mainSkill)` - Get support gem recommendations

**Usage**:
```javascript
import { skillDataService } from './services/maxroll/skillDataService';

// Fetch all skills
const skills = await skillDataService.fetchAllSkills();

// Search for support gems
const supports = await skillDataService.searchSkills({ type: 'Support' });
```

### 3. Item Data Service (`itemDataService.js`)

Fetches unique item data and provides recommendations.

**Key Methods**:
- `fetchAllItems()` - Get all unique items
- `fetchItem(itemName)` - Get specific item data
- `searchItems(criteria)` - Search items by type, slot, rarity
- `getItemRecommendations(buildClass, mainStat)` - Get item recommendations
- `getBudgetAlternatives(itemName)` - Find cheaper alternatives

**Usage**:
```javascript
import { itemDataService } from './services/maxroll/itemDataService';

// Fetch all items
const items = await itemDataService.fetchAllItems();

// Get budget alternatives
const alternatives = await itemDataService.getBudgetAlternatives('The Covenant');
```

### 4. Search Service (`searchService.js`)

Comprehensive search and filtering across all data types.

**Key Methods**:
- `searchBuilds(criteria)` - Advanced build search
- `getBuildsByClass(className)` - Filter by class
- `getBuildsBySkill(skillName)` - Filter by skill
- `getPopularBuilds(limit)` - Get top builds by popularity
- `getTopRatedBuilds(limit)` - Get highest rated builds
- `getLeagueStarterBuilds()` - Get league starter builds
- `searchAll(searchText)` - Search across builds, skills, and items

**Usage**:
```javascript
import { searchService } from './services/maxroll/searchService';

// Search builds with filters
const builds = await searchService.searchBuilds({
  className: 'Ranger',
  difficulty: 'Easy',
  budget: 'Low',
  sortBy: 'rating',
  sortOrder: 'desc'
});

// Get league starters
const leagueStarters = await searchService.getLeagueStarterBuilds();
```

## Data Caching System

### Cache Service (`cacheService.js`)

The caching system provides:
- **TTL Support**: Automatic expiration of cached data
- **Persistence**: Saves to localStorage for offline support
- **Stale Cache**: Returns expired data when fresh data unavailable
- **Statistics**: Monitor cache performance

**Key Methods**:
- `set(key, value, ttl)` - Cache data with TTL
- `get(key, allowStale)` - Retrieve cached data
- `has(key)` - Check if key exists and is valid
- `delete(key)` - Remove cached entry
- `clear()` - Clear all cache
- `clearExpired()` - Remove expired entries
- `getStats()` - Get cache statistics
- `export()` / `import(data)` - Backup/restore cache

**Usage**:
```javascript
import { cacheService } from './services/cache/cacheService';

// Cache data for 1 hour
cacheService.set('myKey', data, 3600);

// Retrieve cached data
const data = cacheService.get('myKey');

// Get cache statistics
const stats = cacheService.getStats();
console.log(`Cache entries: ${stats.totalEntries}`);
console.log(`Cache size: ${stats.totalSizeKB} KB`);
```

## Rate Limiting

### Rate Limiter (`rateLimiter.js`)

Prevents overwhelming Maxroll.gg with requests.

**Configuration**:
- **Max Requests**: 10 requests per 60 seconds
- **Min Delay**: 1 second between requests
- **Auto-queuing**: Requests wait if limit reached

**Usage**:
```javascript
import { rateLimiter } from './services/utils/rateLimiter';

// Consume rate limit
await rateLimiter.consume('maxroll', 1);

// Check status
const status = rateLimiter.getStatus('maxroll');
console.log(`Remaining: ${status.remaining}/${status.total}`);
```

## React Components

### BuildBrowser
Main browsing interface with filtering and search.

**Features**:
- Grid layout of build cards
- Advanced filtering (class, ascendancy, difficulty, budget)
- Text search
- Sorting options
- Real-time filtering

### BuildDetail
Comprehensive build guide view.

**Features**:
- Tabbed interface (Overview, Gems, Items, Tree)
- Build statistics (defense/offense)
- Pros/cons analysis
- Pantheon and bandit choices
- Links to Maxroll.gg

### GemLinks
Display gem setups for builds.

**Features**:
- Visual gem representation with colors
- Main skill and support gems
- Socket count indicators
- Alternative gem suggestions
- Gem quality and level information

### ItemRecommendations
Show recommended items and gear.

**Features**:
- Item cards with modifiers
- Rarity and importance badges
- Price estimates
- Budget alternatives
- Slot filtering

### SkillTree
Passive skill tree information.

**Features**:
- Tree statistics
- Keystone nodes
- Notable passive nodes
- Links to external tree planners

## Data Models

### Build Model
Comprehensive build data structure including:
- Basic info (title, class, ascendancy, skill)
- Metadata (difficulty, budget, rating, popularity)
- Build details (gems, items, passive tree)
- Stats (defense, offense)
- Leveling information

### Skill Model
Skill gem data including:
- Basic info (name, type, color)
- Stats (level, quality, mana cost)
- Tags and classifications
- Vaal and Awakened versions

### Item Model
Unique item data including:
- Basic info (name, type, rarity)
- Modifiers (implicit, explicit, crafted)
- Requirements and sockets
- Price estimates
- Alternative recommendations

## Important Considerations

### Terms of Service
- **Rate Limiting**: Application respects Maxroll.gg's servers with rate limiting
- **Attribution**: All build data properly attributed to Maxroll.gg
- **Caching**: Reduces server load through intelligent caching
- **No Commercial Use**: For educational purposes only

### Implementation Notes

**Current Implementation**:
- Uses mock data for demonstration
- Includes full service architecture for real scraping
- Proper error handling and offline support
- Extensible design for adding real scraping

**To Add Real Scraping**:
1. Implement HTML parsing in service methods using Cheerio
2. Add proxy support if needed
3. Implement CAPTCHA handling
4. Add more robust error recovery
5. Monitor for HTML structure changes

### Error Handling
- **Network Errors**: Falls back to cached data
- **Rate Limiting**: Automatic queuing and retry
- **Missing Data**: Graceful degradation with helpful messages
- **Offline Mode**: Full functionality with cached data

## Development

### Adding New Features

**New Build Filter**:
1. Add filter option in `BuildFilters.jsx`
2. Update search criteria in `searchService.js`
3. Add filter logic in `searchBuilds()` method

**New Data Type**:
1. Create model in `src/models/`
2. Create service in `src/services/maxroll/`
3. Add caching and rate limiting
4. Create React component for display

### Testing

```bash
# Run tests
npm test

# Build for production
npm run build

# Serve production build
npx serve -s build
```

## Future Enhancements

- [ ] Real-time price data from poe.ninja
- [ ] Build comparison tool
- [ ] User build favorites/bookmarks
- [ ] Export builds to Path of Building
- [ ] Interactive passive skill tree viewer
- [ ] Video guide integration
- [ ] Build notes and customization
- [ ] Community ratings and comments
- [ ] Mobile app version
- [ ] Build update notifications

## API Reference

### Service Methods

All services follow consistent patterns:

```javascript
// Fetch all
const items = await service.fetchAll(options);

// Fetch one
const item = await service.fetch(id);

// Search
const results = await service.search(criteria);

// Options support
options = {
  forceRefresh: false,  // Bypass cache
  timeout: 5000,        // Request timeout
  retries: 3           // Retry attempts
}
```

## Troubleshooting

### Common Issues

**Builds not loading**:
- Check browser console for errors
- Verify cache is not corrupted (clear localStorage)
- Check rate limiting status

**Slow performance**:
- Clear expired cache entries
- Reduce concurrent requests
- Check network connection

**Stale data**:
- Force refresh with `forceRefresh: true`
- Clear cache and reload
- Check cache TTL settings

## License

MIT License - See LICENSE file for details

## Credits

- **Data Source**: [Maxroll.gg](https://maxroll.gg/poe)
- **Path of Exile**: [Grinding Gear Games](https://www.pathofexile.com)

## Support

For issues or questions:
- Check the troubleshooting section
- Review service documentation
- Inspect browser console for errors

---

**Built with React and respect for Maxroll.gg's excellent Path of Exile content.**
