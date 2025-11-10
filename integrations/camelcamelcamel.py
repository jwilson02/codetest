"""CamelCamelCamel integration for price tracking."""
import requests
import time
from bs4 import BeautifulSoup
from config import Config


class CamelCamelCamelAPI:
    """Interface for CamelCamelCamel price tracking."""

    def __init__(self, api_key=None):
        """Initialize the API client."""
        self.api_key = api_key or Config.CAMEL_API_KEY
        self.base_url = Config.CAMEL_BASE_URL
        self.session = requests.Session()
        self.session.headers.update({
            'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36'
        })

    def get_product_info(self, asin):
        """Get product price history and current deals."""
        if self.api_key:
            return self._get_via_api(asin)
        else:
            return self._scrape_product_page(asin)

    def _get_via_api(self, asin):
        """Get product info via official API (if available)."""
        # Note: CamelCamelCamel API is limited/paid
        # This is a placeholder for API implementation
        print(f"API method not fully implemented for ASIN: {asin}")
        return None

    def _scrape_product_page(self, asin):
        """Scrape CamelCamelCamel product page."""
        url = f"{self.base_url}/product/{asin}"

        try:
            print(f"Fetching data for ASIN: {asin}")
            response = self.session.get(url, timeout=10)

            if response.status_code != 200:
                print(f"Failed to fetch {url}: Status {response.status_code}")
                return None

            soup = BeautifulSoup(response.text, 'html.parser')

            product_info = {
                'asin': asin,
                'url': url,
                'title': self._extract_title(soup),
                'current_price': self._extract_current_price(soup),
                'lowest_price': self._extract_lowest_price(soup),
                'highest_price': self._extract_highest_price(soup),
                'price_history': self._extract_price_history(soup),
                'deal_score': None
            }

            # Calculate deal score
            if product_info['current_price'] and product_info['lowest_price']:
                try:
                    current = float(product_info['current_price'].replace('$', '').replace(',', ''))
                    lowest = float(product_info['lowest_price'].replace('$', '').replace(',', ''))

                    if lowest > 0:
                        discount_percent = ((current - lowest) / lowest) * 100
                        product_info['deal_score'] = round(discount_percent, 2)
                        product_info['is_good_deal'] = discount_percent < 5  # Within 5% of lowest price
                except ValueError:
                    pass

            # Add delay to be respectful to the server
            time.sleep(1)

            return product_info

        except requests.RequestException as e:
            print(f"Error fetching data for {asin}: {e}")
            return None

    def _extract_title(self, soup):
        """Extract product title."""
        title_elem = soup.find('h1') or soup.find('title')
        if title_elem:
            title = title_elem.text.strip()
            # Clean up title
            if ' - Price History' in title:
                title = title.split(' - Price History')[0]
            return title
        return None

    def _extract_current_price(self, soup):
        """Extract current Amazon price."""
        # Look for current price indicators
        price_elem = soup.find('span', class_='amazon_price') or \
                    soup.find('td', text='Amazon') or \
                    soup.select_one('.product_pane .price')

        if price_elem:
            # Try to find associated price
            if price_elem.name == 'td':
                price = price_elem.find_next_sibling('td')
                if price:
                    return price.text.strip()
            else:
                return price_elem.text.strip()

        return None

    def _extract_lowest_price(self, soup):
        """Extract lowest price from history."""
        # Look for lowest price in summary table
        lowest_elem = soup.find('td', text='Lowest') or \
                     soup.find(text='Lowest Price:')

        if lowest_elem:
            price = lowest_elem.find_next('td') if lowest_elem.name == 'td' else lowest_elem.find_next('span')
            if price:
                return price.text.strip()

        return None

    def _extract_highest_price(self, soup):
        """Extract highest price from history."""
        highest_elem = soup.find('td', text='Highest') or \
                      soup.find(text='Highest Price:')

        if highest_elem:
            price = highest_elem.find_next('td') if highest_elem.name == 'td' else highest_elem.find_next('span')
            if price:
                return price.text.strip()

        return None

    def _extract_price_history(self, soup):
        """Extract price history data."""
        # This would require parsing chart data or tables
        # Placeholder for now
        return {}

    def check_deals(self, products, threshold=10):
        """Check which products have good deals."""
        deals = []

        for product in products:
            asin = product.get('asin')
            if not asin:
                continue

            info = self.get_product_info(asin)
            if info and info.get('deal_score') is not None:
                if info['deal_score'] < threshold:
                    deals.append({
                        'product': product,
                        'camel_info': info,
                        'discount_from_lowest': info['deal_score']
                    })

        return deals

    def get_price_watch_url(self, asin):
        """Get URL for setting up price watch."""
        return f"{self.base_url}/product/{asin}"

    def batch_check_products(self, asins, delay=1.5):
        """Check multiple products with rate limiting."""
        results = {}

        for asin in asins:
            results[asin] = self.get_product_info(asin)
            time.sleep(delay)  # Be respectful to the server

        return results
