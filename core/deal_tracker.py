"""Deal tracking and analysis system."""
import json
import os
from datetime import datetime
from config import Config


class DealTracker:
    """Track and analyze deals for purchased products."""

    def __init__(self, data_file='deals.json'):
        """Initialize the deal tracker."""
        Config.ensure_directories()
        self.data_file = os.path.join(Config.DATA_DIR, data_file)
        self.deals = self._load_deals()

    def _load_deals(self):
        """Load existing deals from file."""
        if os.path.exists(self.data_file):
            try:
                with open(self.data_file, 'r', encoding='utf-8') as f:
                    return json.load(f)
            except Exception as e:
                print(f"Error loading deals: {e}")
                return {'tracked': [], 'history': []}
        return {'tracked': [], 'history': []}

    def _save_deals(self):
        """Save deals to file."""
        try:
            with open(self.data_file, 'w', encoding='utf-8') as f:
                json.dump(self.deals, f, indent=2, ensure_ascii=False)
        except Exception as e:
            print(f"Error saving deals: {e}")

    def add_tracked_product(self, asin, product_info):
        """Add a product to tracking list."""
        # Check if already tracked
        for tracked in self.deals['tracked']:
            if tracked.get('asin') == asin:
                # Update existing
                tracked.update({
                    'last_checked': datetime.now().isoformat(),
                    'product_info': product_info
                })
                self._save_deals()
                return

        # Add new
        self.deals['tracked'].append({
            'asin': asin,
            'product_info': product_info,
            'added_at': datetime.now().isoformat(),
            'last_checked': datetime.now().isoformat()
        })
        self._save_deals()

    def update_deal_status(self, asin, camel_info):
        """Update deal status for a product."""
        timestamp = datetime.now().isoformat()

        # Update tracked product
        for tracked in self.deals['tracked']:
            if tracked.get('asin') == asin:
                tracked['last_checked'] = timestamp
                tracked['camel_info'] = camel_info

                # Add to history if it's a good deal
                if camel_info.get('is_good_deal'):
                    self.deals['history'].append({
                        'asin': asin,
                        'timestamp': timestamp,
                        'deal_score': camel_info.get('deal_score'),
                        'current_price': camel_info.get('current_price'),
                        'lowest_price': camel_info.get('lowest_price')
                    })

        self._save_deals()

    def get_good_deals(self, threshold=10):
        """Get products with good deals."""
        good_deals = []

        for tracked in self.deals['tracked']:
            camel_info = tracked.get('camel_info', {})
            deal_score = camel_info.get('deal_score')

            if deal_score is not None and deal_score < threshold:
                good_deals.append(tracked)

        return good_deals

    def get_all_tracked(self):
        """Get all tracked products."""
        return self.deals['tracked']

    def get_deal_history(self):
        """Get history of deals."""
        return self.deals['history']

    def generate_report(self):
        """Generate a deal report."""
        report = {
            'generated_at': datetime.now().isoformat(),
            'total_tracked': len(self.deals['tracked']),
            'good_deals': len(self.get_good_deals()),
            'deals': []
        }

        for tracked in self.deals['tracked']:
            camel_info = tracked.get('camel_info', {})
            product_info = tracked.get('product_info', {})

            deal = {
                'asin': tracked['asin'],
                'title': product_info.get('title') or camel_info.get('title'),
                'current_price': camel_info.get('current_price'),
                'lowest_price': camel_info.get('lowest_price'),
                'deal_score': camel_info.get('deal_score'),
                'is_good_deal': camel_info.get('is_good_deal', False),
                'camel_url': camel_info.get('url')
            }

            report['deals'].append(deal)

        return report

    def export_report(self, filename='deal_report.json'):
        """Export deal report to file."""
        report = self.generate_report()
        filepath = os.path.join(Config.DATA_DIR, filename)

        with open(filepath, 'w', encoding='utf-8') as f:
            json.dump(report, f, indent=2, ensure_ascii=False)

        print(f"✓ Report saved to {filepath}")
        return filepath
