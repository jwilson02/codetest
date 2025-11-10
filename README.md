# Amazon Deal Tracker with CamelCamelCamel

An automated tool that scrapes your Amazon purchase history and uses CamelCamelCamel to find deals and price tracking opportunities for products you've already bought.

## Features

- **Amazon Purchase Scraper**: Automatically scrapes your Amazon order history
- **CamelCamelCamel Integration**: Fetches historical price data and tracks deals
- **Deal Detection**: Identifies products currently at or near their lowest prices
- **Price Tracking**: Monitors products and alerts you to good deals
- **Notifications**: Get alerts when tracked products have price drops
- **Reports**: Generate detailed deal reports in JSON format

## Prerequisites

- Python 3.8 or higher
- Chrome/Chromium browser
- ChromeDriver (for Selenium)
- Amazon account credentials

## Installation

### 1. Clone the repository

```bash
git clone <repository-url>
cd codetest
```

### 2. Install Python dependencies

```bash
pip install -r requirements.txt
```

### 3. Install ChromeDriver

**On Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install chromium-chromedriver
```

**On macOS:**
```bash
brew install chromedriver
```

**On Windows:**
Download from [ChromeDriver downloads](https://chromedriver.chromium.org/downloads) and add to PATH.

### 4. Configure credentials

Copy the example environment file and edit with your credentials:

```bash
cp .env.example .env
nano .env  # or use your preferred editor
```

Required configuration:
```env
AMAZON_EMAIL=your_email@example.com
AMAZON_PASSWORD=your_password
```

## Usage

### Quick Start

Scrape your Amazon orders and check for deals:

```bash
python main.py scrape
```

This will:
1. Log in to your Amazon account
2. Scrape your order history (default: last 5 pages)
3. Extract all purchased products
4. Check CamelCamelCamel for price history
5. Identify good deals (within 10% of lowest price)
6. Generate a deal report

### Commands

#### Scrape and Check Deals

```bash
python main.py scrape [options]
```

Options:
- `--max-pages N`: Maximum order pages to scrape (default: 5)
- `--max-products N`: Maximum products to check (default: 20)
- `--threshold N`: Deal threshold percentage (default: 10)
- `--show-browser`: Show browser window during scraping

Examples:

```bash
# Scrape more pages
python main.py scrape --max-pages 10

# Check more products
python main.py scrape --max-products 50

# Adjust deal threshold (5% from lowest price)
python main.py scrape --threshold 5

# Show browser for debugging
python main.py scrape --show-browser
```

#### View Tracked Products

```bash
python main.py view
```

Shows all products currently being tracked with their latest price information.

#### Generate Report

```bash
python main.py report
```

Generates a JSON report of all tracked products and deals.

## How It Works

### 1. Amazon Scraping

The scraper uses Selenium to:
- Authenticate with your Amazon credentials
- Navigate through your order history
- Extract product information (ASIN, title, URL, price)
- Handle pagination and dynamic content

**Note**: If Amazon requires 2FA or CAPTCHA, the scraper will pause and wait for you to complete verification in the browser window.

### 2. CamelCamelCamel Integration

For each product:
- Fetches the product page from CamelCamelCamel using the ASIN
- Extracts current price, lowest price, and highest price
- Calculates a "deal score" (percentage from lowest price)
- Marks products as "good deals" if within the threshold

### 3. Deal Tracking

The system:
- Stores all tracked products in `data/deals.json`
- Maintains price history for each product
- Generates notifications for good deals
- Creates exportable reports

## Project Structure

```
codetest/
├── main.py                     # Main CLI interface
├── config.py                   # Configuration management
├── requirements.txt            # Python dependencies
├── .env.example               # Example environment variables
├── scrapers/
│   ├── __init__.py
│   └── amazon_scraper.py      # Amazon purchase scraper
├── integrations/
│   ├── __init__.py
│   └── camelcamelcamel.py    # CamelCamelCamel integration
├── core/
│   ├── __init__.py
│   ├── deal_tracker.py        # Deal tracking system
│   └── notifications.py       # Notification manager
└── data/                       # Generated data files
    ├── amazon_orders.json     # Scraped orders
    ├── deals.json             # Tracked products
    └── deal_report.json       # Deal reports
```

## Output Files

All data files are stored in the `data/` directory:

### `amazon_orders.json`
Raw scraped order data from Amazon:
```json
{
  "scraped_at": "2025-11-10T...",
  "total_orders": 50,
  "orders": [...]
}
```

### `deals.json`
Tracked products and deal information:
```json
{
  "tracked": [...],
  "history": [...]
}
```

### `deal_report.json`
Summary report of all deals:
```json
{
  "generated_at": "2025-11-10T...",
  "total_tracked": 45,
  "good_deals": 8,
  "deals": [...]
}
```

## Security & Privacy

**Important Security Notes:**

- Your Amazon credentials are stored locally in `.env` (never committed to git)
- All data files are stored locally and not transmitted anywhere
- The scraper only reads your order history (no modifications)
- CamelCamelCamel requests are read-only
- Use strong, unique passwords for your Amazon account
- Consider using Amazon's app-specific passwords if available

## Troubleshooting

### Chrome/ChromeDriver Issues

If you get ChromeDriver errors:
```bash
# Check Chrome version
google-chrome --version

# Download matching ChromeDriver from:
# https://chromedriver.chromium.org/downloads
```

### Amazon Login Issues

- **2FA Required**: The scraper will pause and wait for you to complete 2FA in the browser
- **CAPTCHA**: Use `--show-browser` to manually solve CAPTCHAs
- **Account Lock**: Amazon may temporarily lock your account if it detects unusual activity

### Rate Limiting

CamelCamelCamel may rate limit requests:
- The scraper includes automatic delays (1-1.5 seconds between requests)
- If you get blocked, wait a few minutes and try again
- Consider using `--max-products` to limit the number of checks

## Limitations

- **Amazon Structure Changes**: Amazon may change their HTML structure, breaking the scraper
- **CamelCamelCamel Scraping**: CamelCamelCamel doesn't have a public API, so we scrape their site (use responsibly)
- **Rate Limits**: Both Amazon and CamelCamelCamel may rate limit or block automated access
- **Regional Support**: Currently designed for Amazon.com (US site)

## Future Enhancements

Potential improvements:
- [ ] Support for other Amazon regions (amazon.co.uk, amazon.de, etc.)
- [ ] Email notifications for price drops
- [ ] Web dashboard for viewing deals
- [ ] Scheduled automatic checks (cron job)
- [ ] Support for price tracking on products you haven't purchased
- [ ] Integration with other price tracking services
- [ ] Browser extension for one-click tracking

## Legal Disclaimer

This tool is for personal use only. By using this software:
- You agree to Amazon's Terms of Service
- You will not use this tool for commercial scraping
- You will respect rate limits and not overwhelm servers
- You understand this may violate Amazon's ToS regarding automated access

Use at your own risk.

## Contributing

Contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

## License

MIT License - See LICENSE file for details

## Support

For issues, questions, or suggestions:
- Open an issue on GitHub
- Check existing issues for solutions
- Review the troubleshooting section

---

**Happy deal hunting! 🎉**
