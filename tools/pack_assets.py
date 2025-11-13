#!/usr/bin/env python3
"""
Asset Packing Script

Packs game assets for distribution, including:
- Compression
- Validation
- Bundle creation
- Manifest generation
"""

import os
import sys
import json
import argparse
import hashlib
import zlib
import struct
from pathlib import Path
from typing import Dict, List, Any
import shutil

class AssetPacker:
    """Packs and compresses game assets"""

    def __init__(self, source_dir: str, output_dir: str, compression_level: int = 6):
        self.source_dir = Path(source_dir)
        self.output_dir = Path(output_dir)
        self.compression_level = compression_level
        self.manifest = {
            "version": "1.0",
            "assets": [],
            "total_size": 0,
            "compressed_size": 0
        }

    def pack(self, asset_types: List[str] = None, create_bundle: bool = True):
        """Pack assets into optimized format"""
        print(f"Packing assets from: {self.source_dir}")
        print(f"Output directory: {self.output_dir}")

        if not self.source_dir.exists():
            print(f"Error: Source directory does not exist: {self.source_dir}")
            return False

        # Create output directory
        self.output_dir.mkdir(parents=True, exist_ok=True)

        # Asset type filters
        type_extensions = {
            'textures': ['.png', '.jpg', '.jpeg', '.bmp', '.tga'],
            'audio': ['.wav', '.mp3', '.ogg'],
            'shaders': ['.glsl', '.vert', '.frag', '.shader'],
            'models': ['.obj', '.fbx', '.gltf', '.glb'],
            'data': ['.json', '.xml', '.dat'],
            'maps': ['.tmx', '.json']
        }

        # Collect assets
        assets = []
        for asset_type, extensions in type_extensions.items():
            if asset_types and asset_type not in asset_types:
                continue

            assets.extend(self._collect_assets(asset_type, extensions))

        print(f"Found {len(assets)} assets to pack")

        # Process each asset
        for asset in assets:
            self._process_asset(asset)

        # Generate manifest
        manifest_path = self.output_dir / "asset_manifest.json"
        with open(manifest_path, 'w') as f:
            json.dump(self.manifest, indent=2, fp=f)

        print(f"\nPacking complete!")
        print(f"  Total assets: {len(self.manifest['assets'])}")
        print(f"  Original size: {self._format_size(self.manifest['total_size'])}")
        print(f"  Compressed size: {self._format_size(self.manifest['compressed_size'])}")
        compression_ratio = (1 - self.manifest['compressed_size'] / max(self.manifest['total_size'], 1)) * 100
        print(f"  Compression ratio: {compression_ratio:.1f}%")

        # Create bundle if requested
        if create_bundle:
            self._create_bundle()

        return True

    def _collect_assets(self, asset_type: str, extensions: List[str]) -> List[Dict[str, Any]]:
        """Collect assets of a specific type"""
        assets = []
        search_dir = self.source_dir / asset_type

        if not search_dir.exists():
            return assets

        for ext in extensions:
            for file_path in search_dir.rglob(f"*{ext}"):
                if file_path.is_file():
                    rel_path = file_path.relative_to(self.source_dir)
                    assets.append({
                        'path': str(file_path),
                        'relative_path': str(rel_path),
                        'type': asset_type,
                        'extension': ext
                    })

        return assets

    def _process_asset(self, asset: Dict[str, Any]):
        """Process and compress a single asset"""
        input_path = Path(asset['path'])
        rel_path = asset['relative_path']

        # Read asset data
        with open(input_path, 'rb') as f:
            data = f.read()

        original_size = len(data)

        # Calculate hash
        file_hash = hashlib.sha256(data).hexdigest()

        # Compress data
        compressed_data = zlib.compress(data, level=self.compression_level)
        compressed_size = len(compressed_data)

        # Output path
        output_path = self.output_dir / rel_path
        output_path.parent.mkdir(parents=True, exist_ok=True)

        # Decide whether to use compressed version
        use_compressed = compressed_size < original_size * 0.95  # Only if 5%+ savings

        if use_compressed:
            # Write compressed with header
            with open(str(output_path) + '.zpk', 'wb') as f:
                # Write header: magic number (4 bytes) + original size (8 bytes)
                f.write(b'ZPAK')
                f.write(struct.pack('<Q', original_size))
                f.write(compressed_data)

            final_size = compressed_size + 12  # Header size
            final_path = str(rel_path) + '.zpk'
        else:
            # Copy uncompressed
            shutil.copy2(input_path, output_path)
            final_size = original_size
            final_path = rel_path

        # Add to manifest
        asset_entry = {
            'path': final_path,
            'original_path': rel_path,
            'type': asset['type'],
            'size': final_size,
            'original_size': original_size,
            'compressed': use_compressed,
            'hash': file_hash
        }

        self.manifest['assets'].append(asset_entry)
        self.manifest['total_size'] += original_size
        self.manifest['compressed_size'] += final_size

        status = "compressed" if use_compressed else "copied"
        print(f"  {status}: {rel_path} ({self._format_size(original_size)} -> {self._format_size(final_size)})")

    def _create_bundle(self):
        """Create a single bundle file containing all assets"""
        bundle_path = self.output_dir / "assets.bundle"

        print(f"\nCreating bundle: {bundle_path}")

        with open(bundle_path, 'wb') as bundle:
            # Write bundle header
            bundle.write(b'ASSETBUN')  # Magic number
            bundle.write(struct.pack('<I', len(self.manifest['assets'])))  # Asset count

            # Write asset table offset (will update later)
            table_offset_pos = bundle.tell()
            bundle.write(struct.pack('<Q', 0))

            # Write assets
            asset_offsets = []
            for asset_info in self.manifest['assets']:
                asset_path = self.output_dir / asset_info['path']

                if not asset_path.exists():
                    print(f"Warning: Asset not found: {asset_path}")
                    continue

                offset = bundle.tell()
                asset_offsets.append(offset)

                with open(asset_path, 'rb') as f:
                    data = f.read()
                    bundle.write(data)

            # Write asset table
            table_offset = bundle.tell()

            for i, asset_info in enumerate(self.manifest['assets']):
                # Write asset path (null-terminated)
                path_bytes = asset_info['original_path'].encode('utf-8') + b'\0'
                bundle.write(struct.pack('<I', len(path_bytes)))
                bundle.write(path_bytes)

                # Write offset and size
                bundle.write(struct.pack('<Q', asset_offsets[i]))
                bundle.write(struct.pack('<Q', asset_info['size']))

            # Update table offset in header
            bundle.seek(table_offset_pos)
            bundle.write(struct.pack('<Q', table_offset))

        bundle_size = os.path.getsize(bundle_path)
        print(f"Bundle created: {self._format_size(bundle_size)}")

    def _format_size(self, size: int) -> str:
        """Format size in human-readable format"""
        for unit in ['B', 'KB', 'MB', 'GB']:
            if size < 1024.0:
                return f"{size:.2f} {unit}"
            size /= 1024.0
        return f"{size:.2f} TB"

    def validate(self, manifest_path: str) -> bool:
        """Validate packed assets against manifest"""
        print(f"Validating assets from manifest: {manifest_path}")

        with open(manifest_path, 'r') as f:
            manifest = json.load(f)

        errors = 0
        for asset in manifest['assets']:
            asset_path = self.output_dir / asset['path']

            if not asset_path.exists():
                print(f"  ERROR: Missing asset: {asset['path']}")
                errors += 1
                continue

            # Verify size
            actual_size = os.path.getsize(asset_path)
            if actual_size != asset['size']:
                print(f"  ERROR: Size mismatch for {asset['path']}: "
                      f"expected {asset['size']}, got {actual_size}")
                errors += 1

        if errors == 0:
            print(f"Validation successful! All {len(manifest['assets'])} assets verified.")
            return True
        else:
            print(f"Validation failed with {errors} errors.")
            return False


def main():
    parser = argparse.ArgumentParser(description='Pack game assets for distribution')
    parser.add_argument('source', help='Source assets directory')
    parser.add_argument('output', help='Output directory for packed assets')
    parser.add_argument('-c', '--compression', type=int, default=6,
                       help='Compression level (0-9, default: 6)')
    parser.add_argument('-t', '--types', nargs='+',
                       choices=['textures', 'audio', 'shaders', 'models', 'data', 'maps'],
                       help='Asset types to pack (default: all)')
    parser.add_argument('-b', '--bundle', action='store_true',
                       help='Create single bundle file')
    parser.add_argument('-v', '--validate', action='store_true',
                       help='Validate packed assets')

    args = parser.parse_args()

    packer = AssetPacker(args.source, args.output, args.compression)

    if args.validate:
        manifest_path = Path(args.output) / "asset_manifest.json"
        if not manifest_path.exists():
            print(f"Error: Manifest not found: {manifest_path}")
            return 1

        success = packer.validate(str(manifest_path))
        return 0 if success else 1

    success = packer.pack(asset_types=args.types, create_bundle=args.bundle)
    return 0 if success else 1


if __name__ == '__main__':
    sys.exit(main())
