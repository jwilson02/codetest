#!/usr/bin/env python3
"""
Amazon Deal Tracker - Main Interface

This tool scrapes your Amazon purchase history and uses CamelCamelCamel
to find deals and price tracking opportunities for products you've bought.
"""
import sys
import argparse
from scrapers.amazon_scraper import AmazonScraper
from integrations.camelcamelcamel import CamelCamelCamelAPI
from core.deal_tracker import DealTracker
from core.notifications import NotificationManager
from config import Config


def print_banner():
    """Print application banner."""
    print("""
╔═══════════════════════════════════════════════════════════╗
║         Amazon Deal Tracker with CamelCamelCamel          ║
║                                                           ║
║  Track your Amazon purchases and find the best deals!    ║
╚═══════════════════════════════════════════════════════════╝
    """)


def scrape_orders(args):
    """Scrape Amazon orders."""
    print("\n[1/3] Scraping Amazon Orders")
    print("=" * 60)

    scraper = AmazonScraper(headless=not args.show_browser)

    try:
        if not scraper.login():
            print("❌ Failed to log in to Amazon")
            return None

        orders = scraper.scrape_orders(max_pages=args.max_pages)

        if orders:
            filepath = scraper.save_orders(orders)
            print(f"\n✓ Successfully scraped {len(orders)} orders")
            return orders
        else:
            print("❌ No orders found")
            return None

    except Exception as e:
        print(f"❌ Error scraping orders: {e}")
        return None

    finally:
        scraper.close()


def extract_products(orders):
    """Extract unique products from orders."""
    products = []
    seen_asins = set()

    for order in orders:
        for item in order.get('items', []):
            asin = item.get('asin')
            if asin and asin not in seen_asins:
                products.append(item)
                seen_asins.add(asin)

    return products


def check_deals(products, args):
    """Check for deals on products."""
    print("\n[2/3] Checking CamelCamelCamel for Deals")
    print("=" * 60)

    camel = CamelCamelCamelAPI()
    tracker = DealTracker()
    notifier = NotificationManager()

    good_deals = []

    for i, product in enumerate(products[:args.max_products]):
        asin = product.get('asin')
        title = product.get('title', 'Unknown')[:60]

        print(f"\n[{i+1}/{min(len(products), args.max_products)}] Checking: {title}")

        # Track the product
        tracker.add_tracked_product(asin, product)

        # Get CamelCamelCamel info
        camel_info = camel.get_product_info(asin)

        if camel_info:
            # Update tracker
            tracker.update_deal_status(asin, camel_info)

            # Display info
            print(f"   Current Price: {camel_info.get('current_price', 'N/A')}")
            print(f"   Lowest Price:  {camel_info.get('lowest_price', 'N/A')}")

            if camel_info.get('deal_score') is not None:
                print(f"   Deal Score:    {camel_info['deal_score']}% from lowest")

                if camel_info.get('is_good_deal'):
                    print(f"   🎉 GOOD DEAL! Within {args.threshold}% of lowest price")
                    good_deals.append({'product': product, 'camel_info': camel_info})

                    # Create notification
                    notifier.create_deal_alert(product, camel_info)
        else:
            print(f"   ⚠️  Could not fetch data from CamelCamelCamel")

    print(f"\n✓ Found {len(good_deals)} good deals!")

    return good_deals, tracker, notifier


def display_deals(good_deals, tracker, notifier):
    """Display deals summary."""
    print("\n[3/3] Deal Summary")
    print("=" * 60)

    if not good_deals:
        print("\nNo exceptional deals found at the moment.")
        print("Your products are being tracked for future price drops!")
        return

    print(f"\n🎉 Found {len(good_deals)} products with great deals:\n")

    for i, deal in enumerate(good_deals):
        product = deal['product']
        camel_info = deal['camel_info']

        print(f"{i+1}. {product.get('title', 'Unknown Product')}")
        print(f"   ASIN: {product.get('asin')}")
        print(f"   Current: {camel_info.get('current_price', 'N/A')} | "
              f"Lowest: {camel_info.get('lowest_price', 'N/A')} | "
              f"Score: {camel_info.get('deal_score', 'N/A')}%")
        print(f"   Track: {camel_info.get('url', 'N/A')}")
        print()

    # Export report
    report_file = tracker.export_report()
    print(f"\n✓ Full report saved to: {report_file}")

    # Show notifications
    print()
    notifier.print_notifications()


def view_tracked(args):
    """View tracked products."""
    tracker = DealTracker()
    tracked = tracker.get_all_tracked()

    print("\n📊 Tracked Products")
    print("=" * 60)

    if not tracked:
        print("\nNo products being tracked yet.")
        print("Run 'python main.py scrape' to start tracking!")
        return

    print(f"\nTotal tracked: {len(tracked)}\n")

    for i, item in enumerate(tracked[:20]):  # Show first 20
        product_info = item.get('product_info', {})
        camel_info = item.get('camel_info', {})

        print(f"{i+1}. {product_info.get('title', 'Unknown')[:70]}")
        print(f"   ASIN: {item.get('asin')}")
        print(f"   Last checked: {item.get('last_checked', 'Never')}")

        if camel_info.get('current_price'):
            print(f"   Price: {camel_info.get('current_price')} "
                  f"(Lowest: {camel_info.get('lowest_price', 'N/A')})")

        print()


def main():
    """Main application entry point."""
    parser = argparse.ArgumentParser(
        description='Track Amazon purchases and find deals with CamelCamelCamel'
    )

    subparsers = parser.add_subparsers(dest='command', help='Commands')

    # Scrape command
    scrape_parser = subparsers.add_parser('scrape', help='Scrape Amazon orders and check for deals')
    scrape_parser.add_argument('--max-pages', type=int, default=5,
                              help='Maximum number of order pages to scrape (default: 5)')
    scrape_parser.add_argument('--max-products', type=int, default=20,
                              help='Maximum number of products to check (default: 20)')
    scrape_parser.add_argument('--threshold', type=int, default=10,
                              help='Deal threshold percentage from lowest price (default: 10)')
    scrape_parser.add_argument('--show-browser', action='store_true',
                              help='Show browser window during scraping')

    # View command
    view_parser = subparsers.add_parser('view', help='View tracked products')

    # Report command
    report_parser = subparsers.add_parser('report', help='Generate deal report')

    args = parser.parse_args()

    # Validate config
    errors = Config.validate()
    if errors and args.command == 'scrape':
        print("❌ Configuration errors:")
        for error in errors:
            print(f"   - {error}")
        print("\nPlease copy .env.example to .env and configure your credentials.")
        sys.exit(1)

    print_banner()

    if args.command == 'scrape':
        # Full scrape and check workflow
        orders = scrape_orders(args)

        if orders:
            products = extract_products(orders)
            print(f"\n✓ Extracted {len(products)} unique products")

            if products:
                good_deals, tracker, notifier = check_deals(products, args)
                display_deals(good_deals, tracker, notifier)

    elif args.command == 'view':
        view_tracked(args)

    elif args.command == 'report':
        tracker = DealTracker()
        report_file = tracker.export_report()
        print(f"\n✓ Report generated: {report_file}")

    else:
        parser.print_help()


if __name__ == '__main__':
    main()
