"""Amazon purchase scraper using Selenium."""
import time
import json
from datetime import datetime
from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
from selenium.common.exceptions import TimeoutException, NoSuchElementException
from bs4 import BeautifulSoup
from config import Config


class AmazonScraper:
    """Scraper for Amazon purchase history."""

    def __init__(self, email=None, password=None, headless=True):
        """Initialize the scraper."""
        self.email = email or Config.AMAZON_EMAIL
        self.password = password or Config.AMAZON_PASSWORD
        self.headless = headless
        self.driver = None

    def _init_driver(self):
        """Initialize the Selenium WebDriver."""
        options = webdriver.ChromeOptions()

        if self.headless:
            options.add_argument('--headless')

        options.add_argument('--no-sandbox')
        options.add_argument('--disable-dev-shm-usage')
        options.add_argument('--disable-blink-features=AutomationControlled')
        options.add_argument('user-agent=Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36')

        self.driver = webdriver.Chrome(options=options)
        self.driver.implicitly_wait(10)

    def login(self):
        """Log in to Amazon."""
        if not self.driver:
            self._init_driver()

        print("Navigating to Amazon login page...")
        self.driver.get(Config.AMAZON_LOGIN_URL)

        try:
            # Enter email
            email_field = WebDriverWait(self.driver, 10).until(
                EC.presence_of_element_located((By.ID, "ap_email"))
            )
            email_field.send_keys(self.email)

            # Click continue
            continue_button = self.driver.find_element(By.ID, "continue")
            continue_button.click()

            # Enter password
            password_field = WebDriverWait(self.driver, 10).until(
                EC.presence_of_element_located((By.ID, "ap_password"))
            )
            password_field.send_keys(self.password)

            # Submit
            sign_in_button = self.driver.find_element(By.ID, "signInSubmit")
            sign_in_button.click()

            # Wait for login to complete
            time.sleep(3)

            # Check if 2FA or CAPTCHA is required
            if "ap/mfa" in self.driver.current_url or "ap/cvf" in self.driver.current_url:
                print("\n⚠️  Two-factor authentication or verification required!")
                print("Please complete the verification in the browser window.")
                print("Waiting for you to complete verification...")

                # Wait up to 2 minutes for user to complete verification
                for i in range(120):
                    time.sleep(1)
                    if "ap/mfa" not in self.driver.current_url and "ap/cvf" not in self.driver.current_url:
                        break

            print("✓ Logged in successfully!")
            return True

        except Exception as e:
            print(f"Login failed: {e}")
            return False

    def scrape_orders(self, max_pages=5):
        """Scrape order history."""
        if not self.driver:
            if not self.login():
                return []

        print("Navigating to order history...")
        self.driver.get(Config.AMAZON_ORDERS_URL)
        time.sleep(2)

        all_orders = []

        for page in range(max_pages):
            print(f"Scraping page {page + 1}...")

            # Parse current page
            soup = BeautifulSoup(self.driver.page_source, 'html.parser')
            orders = self._parse_orders_page(soup)

            if not orders:
                print("No more orders found.")
                break

            all_orders.extend(orders)

            # Try to go to next page
            try:
                next_button = self.driver.find_element(By.CSS_SELECTOR, ".a-pagination .a-last a")
                if "a-disabled" in next_button.get_attribute("class"):
                    print("Reached last page.")
                    break
                next_button.click()
                time.sleep(2)
            except NoSuchElementException:
                print("No next page button found.")
                break

        print(f"✓ Scraped {len(all_orders)} orders")
        return all_orders

    def _parse_orders_page(self, soup):
        """Parse orders from a page."""
        orders = []

        # Find all order containers
        order_cards = soup.find_all('div', class_='order-card') or \
                     soup.find_all('div', class_='order')

        if not order_cards:
            # Alternative structure
            order_cards = soup.find_all('div', {'data-order-id': True})

        for card in order_cards:
            try:
                order = self._parse_order_card(card)
                if order:
                    orders.append(order)
            except Exception as e:
                print(f"Error parsing order: {e}")
                continue

        return orders

    def _parse_order_card(self, card):
        """Parse a single order card."""
        order = {}

        # Order ID
        order_id_elem = card.find('span', class_='order-info-value') or \
                       card.get('data-order-id')
        if order_id_elem:
            order['order_id'] = order_id_elem.text.strip() if hasattr(order_id_elem, 'text') else order_id_elem

        # Order date
        date_elem = card.find('span', class_='order-date-invoice-item')
        if date_elem:
            order['date'] = date_elem.text.strip()

        # Order total
        total_elem = card.find('span', class_='order-total')
        if total_elem:
            order['total'] = total_elem.text.strip()

        # Items
        items = []
        item_containers = card.find_all('div', class_='a-fixed-left-grid-inner') or \
                         card.find_all('div', class_='shipment-item')

        for item_container in item_containers:
            item = {}

            # Product title
            title_elem = item_container.find('a', class_='a-link-normal')
            if title_elem:
                item['title'] = title_elem.text.strip()
                item['url'] = 'https://www.amazon.com' + title_elem.get('href', '') if title_elem.get('href', '').startswith('/') else title_elem.get('href', '')

                # Extract ASIN from URL
                if '/dp/' in item['url']:
                    asin = item['url'].split('/dp/')[1].split('/')[0].split('?')[0]
                    item['asin'] = asin

            # Price
            price_elem = item_container.find('span', class_='a-color-price')
            if price_elem:
                item['price'] = price_elem.text.strip()

            if item:
                items.append(item)

        order['items'] = items

        return order if order.get('order_id') else None

    def save_orders(self, orders, filename='amazon_orders.json'):
        """Save orders to JSON file."""
        Config.ensure_directories()
        filepath = f"{Config.DATA_DIR}/{filename}"

        with open(filepath, 'w', encoding='utf-8') as f:
            json.dump({
                'scraped_at': datetime.now().isoformat(),
                'total_orders': len(orders),
                'orders': orders
            }, f, indent=2, ensure_ascii=False)

        print(f"✓ Saved orders to {filepath}")
        return filepath

    def close(self):
        """Close the browser."""
        if self.driver:
            self.driver.quit()
            self.driver = None
