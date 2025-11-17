# Maxroll.gg Integration Architecture

## Overview

This document describes the architecture of the Path of Exile build guide application with Maxroll.gg integration.

## System Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                          React Frontend                          │
│                                                                   │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐          │
│  │ BuildBrowser │  │ BuildDetail  │  │  GemLinks    │          │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘          │
│         │                  │                  │                   │
│         └──────────────────┴──────────────────┘                  │
│                            │                                      │
└────────────────────────────┼──────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│                      Service Layer                               │
│                                                                   │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │              Maxroll Services                            │   │
│  │  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  │   │
│  │  │    Build     │  │    Skill     │  │    Item      │  │   │
│  │  │   Service    │  │   Service    │  │   Service    │  │   │
│  │  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘  │   │
│  │         │                  │                  │           │   │
│  │         └──────────────────┴──────────────────┘           │   │
│  │                            │                               │   │
│  │                    ┌───────┴────────┐                     │   │
│  │                    │ Search Service │                     │   │
│  │                    └───────┬────────┘                     │   │
│  └────────────────────────────┼──────────────────────────────┘   │
│                               │                                   │
│  ┌────────────────────────────┼──────────────────────────────┐   │
│  │         Support Services   │                              │   │
│  │  ┌──────────────┐  ┌───────┴───────┐  ┌──────────────┐  │   │
│  │  │    Cache     │◄─┤ Rate Limiter  │  │   Logger     │  │   │
│  │  │   Service    │  └───────────────┘  └──────────────┘  │   │
│  │  └──────────────┘                                        │   │
│  └──────────────────────────────────────────────────────────┘   │
└────────────────────────────────┬─────────────────────────────────┘
                                 │
                                 ▼
┌─────────────────────────────────────────────────────────────────┐
│                    Data Storage Layer                            │
│                                                                   │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐          │
│  │  localStorage│  │   IndexedDB  │  │  Memory Cache│          │
│  │   (Primary)  │  │   (Future)   │  │   (Runtime)  │          │
│  └──────────────┘  └──────────────┘  └──────────────┘          │
└─────────────────────────────────────────────────────────────────┘
```

## Component Layer

### 1. BuildBrowser Component
**Responsibility**: Display and filter build list

**State Management**:
- Builds array
- Filter criteria
- Loading state
- Error state

**Data Flow**:
```
User Input → BuildFilters → searchService.searchBuilds()
                                     ↓
                              buildGuideService.fetchAllBuilds()
                                     ↓
                              cacheService.get/set
                                     ↓
                              Build Models → BuildCard components
```

### 2. BuildDetail Component
**Responsibility**: Display comprehensive build information

**State Management**:
- Build object
- Active tab
- Loading state

**Data Flow**:
```
buildId (URL param) → buildGuideService.fetchBuildGuide()
                                     ↓
                              cacheService.get/set
                                     ↓
                              Build Model → Tab Components
                                     ↓
                    GemLinks | ItemRecommendations | SkillTree
```

### 3. Supporting Components

**GemLinks**: Display gem setups
**ItemRecommendations**: Show items with filtering
**SkillTree**: Display passive tree info
**BuildFilters**: Advanced filtering UI

## Service Layer

### Service Hierarchy

```
BaseService (Abstract)
    ├── buildGuideService
    ├── skillDataService
    ├── itemDataService
    └── searchService
```

### 1. Build Guide Service

**Methods**:
```javascript
fetchAllBuilds(options)
  → rateLimiter.consume()
  → HTTP Request / Parse
  → Build Models
  → cacheService.set()
  → return Build[]

fetchBuildGuide(buildId)
  → Check cache
  → rateLimiter.consume()
  → HTTP Request / Parse
  → Build Model
  → cacheService.set()
  → return Build
```

**Caching Strategy**:
- TTL: 3600 seconds (1 hour)
- Stale-while-revalidate pattern
- Offline fallback

### 2. Skill Data Service

**Methods**:
```javascript
fetchAllSkills()
searchSkills(criteria)
getSkillRecommendations(mainSkill)
```

**Caching Strategy**:
- TTL: 7200 seconds (2 hours)
- Static data, longer cache

### 3. Item Data Service

**Methods**:
```javascript
fetchAllItems()
fetchItem(itemName)
searchItems(criteria)
getItemRecommendations(buildClass)
getBudgetAlternatives(itemName)
```

**Caching Strategy**:
- TTL: 7200 seconds
- Price data may need shorter TTL in production

### 4. Search Service

**Methods**:
```javascript
searchBuilds(criteria)
  → Load all builds
  → Apply filters
  → Sort results
  → Paginate
  → return filtered Build[]

getFilterOptions()
  → Analyze all builds
  → Extract unique values
  → return options object
```

**Optimization**:
- Cache search results
- TTL: 1800 seconds (30 min)
- Invalidate on build data update

## Support Services

### 1. Cache Service

**Architecture**:
```
CacheService
  ├── Memory Cache (Map)
  ├── Metadata Store (TTL, hits)
  └── Persistence (localStorage)

Methods:
  - set(key, value, ttl)
  - get(key, allowStale)
  - has(key)
  - delete(key)
  - clear()
  - clearExpired()
  - getStats()
```

**Cache Strategies**:

1. **Cache-First**: Check cache before network
2. **Network-First**: Try network, fall back to cache
3. **Stale-While-Revalidate**: Return stale, update in background

**Implementation**:
```javascript
// Cache-First (default)
get(key) {
  if (cache.has(key) && !expired) return cache.get(key);
  return null;
}

// Stale-While-Revalidate
get(key, allowStale = true) {
  if (cache.has(key) && !expired) return cache.get(key);
  if (allowStale && cache.has(key)) return cache.get(key);
  return null;
}
```

### 2. Rate Limiter

**Architecture**:
```
RateLimiter
  ├── Queue per key (Map)
  ├── Configuration (points, duration, minDelay)
  └── Request tracking (timestamps)

Algorithm: Token Bucket with minimum delay
```

**Implementation**:
```javascript
consume(key, points) {
  1. Remove old requests (outside window)
  2. Check if at limit
     → If yes: Calculate wait time and delay
     → If no: Continue
  3. Enforce minimum delay
  4. Add request timestamp
  5. Return
}
```

**Configuration**:
- Max: 10 requests per 60 seconds
- Min Delay: 1000ms between requests
- Auto-retry on limit

## Data Models

### Build Model

```javascript
Build {
  // Metadata
  id, title, className, ascendancy
  mainSkill, author, league, patch
  url, description

  // Classification
  difficulty, budget, playstyle
  rating, popularity

  // Build Details
  passiveTree: PassiveTree
  gemLinks: GemLink[]
  items: Item[]

  // Game Choices
  pantheon: { major, minor }
  bandit: string

  // Progression
  levelingGuide: LevelingStep[]
  gearProgression: GearTier[]

  // Stats
  defenseStats: {}
  offenseStats: {}

  // Analysis
  pros: string[]
  cons: string[]

  // Methods
  validate()
  toJSON()
}
```

### Skill Model

```javascript
Skill {
  id, name, type, color
  level, quality
  description, tags
  manaCost, cooldown
  vaalVersion, awakened
  alternatives

  toJSON()
}
```

### Item Model

```javascript
Item {
  id, name, type, rarity
  basetype, level, itemLevel

  // Modifiers
  explicits, implicits, crafted
  corrupted

  // Requirements
  requirements: { level, str, dex, int }

  // Properties
  sockets, links, quality

  // Economy
  price: { chaos, divine }
  alternatives

  // Build Integration
  importance, slot

  toJSON()
}
```

## Data Flow Patterns

### 1. Initial Load

```
App Mount
    ↓
BuildBrowser Mount
    ↓
fetchAllBuilds()
    ↓
Check Cache → Hit? Return cached
    ↓ (Miss)
Rate Limit Check
    ↓
HTTP Request (Future: scrape HTML)
    ↓
Parse Response
    ↓
Create Build Models
    ↓
Cache Results
    ↓
Update State
    ↓
Render BuildCards
```

### 2. Build Detail View

```
Route Change (/build/:id)
    ↓
BuildDetail Mount
    ↓
fetchBuildGuide(id)
    ↓
Check Cache → Hit? Return cached
    ↓ (Miss)
Rate Limit Check
    ↓
HTTP Request
    ↓
Parse Detailed Data
    ↓
Create Build Model
    ↓
Cache Result
    ↓
Update State
    ↓
Render Tabs (Overview, Gems, Items, Tree)
```

### 3. Search and Filter

```
User Updates Filter
    ↓
Update Filter State
    ↓
searchService.searchBuilds(criteria)
    ↓
Check Search Cache
    ↓ (Miss)
Load All Builds (from cache)
    ↓
Apply Filters
  - Class filter
  - Ascendancy filter
  - Skill filter
  - Difficulty filter
  - Budget filter
  - Text search
    ↓
Sort Results
    ↓
Paginate
    ↓
Cache Search Results
    ↓
Return Filtered Builds
    ↓
Update UI
```

## Error Handling

### Error Hierarchy

```
Error
  ├── NetworkError
  │   ├── TimeoutError
  │   └── ConnectionError
  ├── RateLimitError
  ├── ParseError
  └── CacheError
```

### Error Recovery Strategies

1. **Network Errors**: Fall back to cached data
2. **Rate Limit**: Auto-queue and retry
3. **Parse Errors**: Log and use defaults
4. **Cache Errors**: Continue without cache

### Implementation

```javascript
try {
  const data = await service.fetch();
  return data;
} catch (error) {
  if (error instanceof NetworkError) {
    // Try cache
    const cached = cache.get(key, true); // Allow stale
    if (cached) return cached;
  }

  if (error instanceof RateLimitError) {
    // Auto-handled by rate limiter
    throw error;
  }

  // Log and rethrow
  logger.error(error);
  throw error;
}
```

## Performance Optimization

### 1. Caching Strategy
- **Build List**: Cache for 1 hour
- **Build Details**: Cache for 1 hour
- **Skill Data**: Cache for 2 hours (static)
- **Item Data**: Cache for 2 hours
- **Search Results**: Cache for 30 minutes

### 2. Lazy Loading
- Build images lazy loaded
- Build details loaded on demand
- Pagination for large lists

### 3. Code Splitting
```javascript
const BuildDetail = lazy(() => import('./components/BuildDetail'));
const BuildBrowser = lazy(() => import('./components/BuildBrowser'));
```

### 4. Memoization
- Filter options memoized
- Search results memoized
- Component re-renders minimized

## Security Considerations

### 1. Rate Limiting
- Prevents abuse of Maxroll.gg
- Configurable limits
- Per-domain tracking

### 2. Data Validation
- Validate all external data
- Sanitize HTML content
- Type checking with models

### 3. CORS Handling
- Proxy server for production
- CORS headers configuration
- Error handling for blocked requests

### 4. Privacy
- No user data collection
- localStorage only for cache
- No external analytics (configurable)

## Scalability

### Current Capacity
- 100+ builds
- 200+ skills
- 500+ items
- Instant search/filter

### Future Scaling
1. **IndexedDB**: For larger datasets
2. **Worker Threads**: Background data processing
3. **Virtual Scrolling**: For very large lists
4. **CDN Caching**: For static assets
5. **Backend API**: For production deployment

## Testing Strategy

### Unit Tests
- Service methods
- Data models
- Cache functionality
- Rate limiter

### Integration Tests
- Service + Cache
- Component + Service
- End-to-end flows

### Performance Tests
- Load time benchmarks
- Cache hit rates
- Memory usage

## Deployment

### Development
```bash
npm start  # localhost:3000
```

### Production
```bash
npm run build
serve -s build
```

### Environment Variables
```env
REACT_APP_API_URL=https://maxroll.gg
REACT_APP_CACHE_TTL=3600
REACT_APP_RATE_LIMIT_MAX=10
REACT_APP_RATE_LIMIT_WINDOW=60
```

## Future Enhancements

### Phase 1: Real Scraping
- Implement Cheerio parsing
- Add proxy support
- Handle dynamic content (Puppeteer)

### Phase 2: Advanced Features
- User accounts
- Build favorites
- Custom builds
- Build comparison

### Phase 3: Integration
- PoE Ninja price data
- Path of Building export
- Trade API integration

### Phase 4: Community
- User ratings
- Comments
- Build guides submission
- Build statistics

## Conclusion

This architecture provides:
- **Scalability**: Modular services, caching
- **Performance**: Smart caching, lazy loading
- **Reliability**: Error handling, offline support
- **Maintainability**: Clear separation of concerns
- **Extensibility**: Easy to add new features

The system respects Maxroll.gg's resources through rate limiting and caching while providing an excellent user experience through smart data management and offline support.
