/**
 * Rate Limiter - Prevents overwhelming Maxroll.gg with requests
 * Respects their terms of service and prevents IP blocking
 */

class RateLimiter {
  constructor() {
    this.queues = new Map();
    this.config = {
      maxroll: {
        points: 10, // Max 10 requests
        duration: 60, // Per 60 seconds
        minDelay: 1000 // Minimum 1 second between requests
      }
    };
    this.lastRequest = new Map();
  }

  /**
   * Consume rate limit points
   * @param {string} key - Rate limiter key (e.g., 'maxroll')
   * @param {number} points - Points to consume (default: 1)
   * @returns {Promise<void>}
   */
  async consume(key, points = 1) {
    const config = this.config[key];
    if (!config) {
      console.warn(`No rate limit config for key: ${key}`);
      return;
    }

    // Initialize queue if doesn't exist
    if (!this.queues.has(key)) {
      this.queues.set(key, []);
    }

    const queue = this.queues.get(key);
    const now = Date.now();

    // Remove old entries outside the duration window
    while (queue.length > 0 && queue[0] < now - config.duration * 1000) {
      queue.shift();
    }

    // Check if we're at the limit
    if (queue.length >= config.points) {
      const oldestRequest = queue[0];
      const waitTime = oldestRequest + config.duration * 1000 - now;
      console.log(`Rate limit reached for ${key}. Waiting ${waitTime}ms`);
      await this.delay(waitTime);
      return this.consume(key, points);
    }

    // Enforce minimum delay between requests
    const lastReq = this.lastRequest.get(key) || 0;
    const timeSinceLastReq = now - lastReq;
    if (timeSinceLastReq < config.minDelay) {
      const delayTime = config.minDelay - timeSinceLastReq;
      await this.delay(delayTime);
    }

    // Add request to queue
    queue.push(Date.now());
    this.lastRequest.set(key, Date.now());
  }

  /**
   * Delay helper
   * @private
   */
  delay(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
  }

  /**
   * Get current rate limit status
   * @param {string} key - Rate limiter key
   * @returns {Object}
   */
  getStatus(key) {
    const config = this.config[key];
    if (!config) {
      return null;
    }

    const queue = this.queues.get(key) || [];
    const now = Date.now();

    // Count requests in current window
    const activeRequests = queue.filter(t => t > now - config.duration * 1000).length;

    return {
      remaining: config.points - activeRequests,
      total: config.points,
      resetIn: queue.length > 0 ? queue[0] + config.duration * 1000 - now : 0
    };
  }

  /**
   * Update rate limit configuration
   * @param {string} key - Rate limiter key
   * @param {Object} config - New configuration
   */
  updateConfig(key, config) {
    this.config[key] = { ...this.config[key], ...config };
  }
}

export const rateLimiter = new RateLimiter();
export default rateLimiter;
