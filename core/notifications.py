"""Notification system for deal alerts."""
from datetime import datetime


class NotificationManager:
    """Manage notifications for deal alerts."""

    def __init__(self):
        """Initialize notification manager."""
        self.notifications = []

    def add_notification(self, notification_type, title, message, data=None):
        """Add a notification."""
        notification = {
            'type': notification_type,
            'title': title,
            'message': message,
            'data': data or {},
            'timestamp': datetime.now().isoformat(),
            'read': False
        }

        self.notifications.append(notification)
        return notification

    def create_deal_alert(self, product_info, camel_info):
        """Create a deal alert notification."""
        title = f"Deal Alert: {product_info.get('title', 'Unknown Product')[:50]}"

        message = f"""
Product: {product_info.get('title', 'Unknown')}
Current Price: {camel_info.get('current_price', 'N/A')}
Lowest Price: {camel_info.get('lowest_price', 'N/A')}
Deal Score: {camel_info.get('deal_score', 'N/A')}% from lowest

This product is within your deal threshold!
View on CamelCamelCamel: {camel_info.get('url', 'N/A')}
        """

        return self.add_notification(
            notification_type='deal_alert',
            title=title,
            message=message.strip(),
            data={
                'asin': product_info.get('asin'),
                'product_info': product_info,
                'camel_info': camel_info
            }
        )

    def create_price_drop_alert(self, product_info, old_price, new_price):
        """Create a price drop notification."""
        title = f"Price Drop: {product_info.get('title', 'Unknown Product')[:50]}"

        message = f"""
Product: {product_info.get('title', 'Unknown')}
Old Price: {old_price}
New Price: {new_price}

Price has dropped!
        """

        return self.add_notification(
            notification_type='price_drop',
            title=title,
            message=message.strip(),
            data={
                'asin': product_info.get('asin'),
                'old_price': old_price,
                'new_price': new_price
            }
        )

    def get_unread_notifications(self):
        """Get unread notifications."""
        return [n for n in self.notifications if not n['read']]

    def get_all_notifications(self):
        """Get all notifications."""
        return self.notifications

    def mark_as_read(self, index):
        """Mark a notification as read."""
        if 0 <= index < len(self.notifications):
            self.notifications[index]['read'] = True

    def mark_all_as_read(self):
        """Mark all notifications as read."""
        for notification in self.notifications:
            notification['read'] = True

    def clear_notifications(self):
        """Clear all notifications."""
        self.notifications = []

    def print_notifications(self):
        """Print all notifications to console."""
        if not self.notifications:
            print("No notifications.")
            return

        print(f"\n{'='*80}")
        print(f"NOTIFICATIONS ({len(self.get_unread_notifications())} unread)")
        print(f"{'='*80}\n")

        for i, notif in enumerate(self.notifications):
            status = "  " if notif['read'] else "🔔"
            print(f"{status} [{notif['type'].upper()}] {notif['title']}")
            print(f"   {notif['timestamp']}")
            print(f"   {notif['message'][:100]}...")
            print()

    def send_email_notification(self, notification):
        """Send email notification (placeholder)."""
        # This would integrate with an email service
        # For now, just print
        print(f"\n[EMAIL NOTIFICATION]")
        print(f"To: {notification.get('recipient', 'user@example.com')}")
        print(f"Subject: {notification.get('title')}")
        print(f"Body: {notification.get('message')}")
