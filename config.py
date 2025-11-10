"""Configuration management for the Amazon deal tracker."""
import os
from dotenv import load_dotenv

# Load environment variables
load_dotenv()

class Config:
    """Application configuration."""

    # Amazon credentials
    AMAZON_EMAIL = os.getenv('AMAZON_EMAIL', '')
    AMAZON_PASSWORD = os.getenv('AMAZON_PASSWORD', '')

    # CamelCamelCamel settings
    CAMEL_API_KEY = os.getenv('CAMEL_API_KEY', '')

    # Notification settings
    NOTIFICATION_EMAIL = os.getenv('NOTIFICATION_EMAIL', '')
    NOTIFICATION_THRESHOLD = int(os.getenv('NOTIFICATION_THRESHOLD', '10'))

    # Browser settings
    HEADLESS = os.getenv('HEADLESS', 'true').lower() == 'true'

    # URLs
    AMAZON_ORDERS_URL = 'https://www.amazon.com/gp/your-account/order-history'
    AMAZON_LOGIN_URL = 'https://www.amazon.com/ap/signin'
    CAMEL_BASE_URL = 'https://camelcamelcamel.com'

    # Data directories
    DATA_DIR = os.path.join(os.path.dirname(__file__), 'data')
    CACHE_DIR = os.path.join(DATA_DIR, 'cache')

    @classmethod
    def validate(cls):
        """Validate required configuration."""
        errors = []

        if not cls.AMAZON_EMAIL:
            errors.append("AMAZON_EMAIL is required")
        if not cls.AMAZON_PASSWORD:
            errors.append("AMAZON_PASSWORD is required")

        return errors

    @classmethod
    def ensure_directories(cls):
        """Create necessary directories."""
        os.makedirs(cls.DATA_DIR, exist_ok=True)
        os.makedirs(cls.CACHE_DIR, exist_ok=True)
