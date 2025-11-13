#!/usr/bin/env python3
"""
Texture Atlas Generator

Packs multiple textures into a single texture atlas for optimized rendering.
Supports multiple packing algorithms and optimization strategies.
"""

import os
import sys
import json
import argparse
from pathlib import Path
from typing import List, Tuple, Dict, Any
from dataclasses import dataclass
from enum import Enum

try:
    from PIL import Image
    HAS_PIL = True
except ImportError:
    HAS_PIL = False
    print("Warning: PIL/Pillow not installed. Image processing will be limited.")
    print("Install with: pip install Pillow")


class PackingAlgorithm(Enum):
    """Texture packing algorithms"""
    SIMPLE = "simple"
    MAXRECTS = "maxrects"
    SKYLINE = "skyline"


@dataclass
class Rect:
    """Rectangle representation"""
    x: int
    y: int
    width: int
    height: int


@dataclass
class SpriteInfo:
    """Sprite information"""
    name: str
    path: str
    image: Any  # PIL Image
    rect: Rect = None
    trimmed: bool = False
    original_size: Tuple[int, int] = None
    rotated: bool = False


class TextureAtlasGenerator:
    """Generates texture atlases from individual sprites"""

    def __init__(self, max_width: int = 2048, max_height: int = 2048,
                 padding: int = 2, power_of_two: bool = True):
        self.max_width = max_width
        self.max_height = max_height
        self.padding = padding
        self.power_of_two = power_of_two
        self.sprites: List[SpriteInfo] = []

    def add_sprite(self, name: str, image_path: str):
        """Add a sprite to the atlas"""
        if not HAS_PIL:
            print(f"Error: Cannot load image without PIL/Pillow: {image_path}")
            return False

        try:
            image = Image.open(image_path).convert('RGBA')
            sprite = SpriteInfo(
                name=name,
                path=image_path,
                image=image,
                original_size=(image.width, image.height)
            )
            self.sprites.append(sprite)
            return True
        except Exception as e:
            print(f"Error loading image {image_path}: {e}")
            return False

    def add_directory(self, directory: str, extensions: List[str] = None):
        """Add all images from a directory"""
        if extensions is None:
            extensions = ['.png', '.jpg', '.jpeg', '.bmp', '.tga']

        dir_path = Path(directory)
        if not dir_path.exists():
            print(f"Error: Directory not found: {directory}")
            return 0

        count = 0
        for ext in extensions:
            for image_path in dir_path.glob(f'**/*{ext}'):
                if image_path.is_file():
                    name = image_path.stem
                    if self.add_sprite(name, str(image_path)):
                        count += 1

        return count

    def trim_sprites(self):
        """Trim transparent pixels from sprites"""
        if not HAS_PIL:
            return

        print("Trimming transparent pixels...")
        for sprite in self.sprites:
            bbox = sprite.image.getbbox()
            if bbox:
                trimmed = sprite.image.crop(bbox)
                if trimmed.size != sprite.image.size:
                    sprite.image = trimmed
                    sprite.trimmed = True

    def generate(self, output_image: str, output_data: str,
                 algorithm: PackingAlgorithm = PackingAlgorithm.MAXRECTS,
                 trim_alpha: bool = True) -> bool:
        """Generate the texture atlas"""
        if not self.sprites:
            print("Error: No sprites to pack")
            return False

        if not HAS_PIL:
            print("Error: PIL/Pillow required for atlas generation")
            return False

        print(f"Generating texture atlas with {len(self.sprites)} sprites...")
        print(f"  Algorithm: {algorithm.value}")
        print(f"  Max size: {self.max_width}x{self.max_height}")
        print(f"  Padding: {self.padding}px")

        # Trim sprites if requested
        if trim_alpha:
            self.trim_sprites()

        # Sort sprites by height (descending) for better packing
        self.sprites.sort(key=lambda s: s.image.height, reverse=True)

        # Pack sprites
        if algorithm == PackingAlgorithm.MAXRECTS:
            success = self._pack_maxrects()
        elif algorithm == PackingAlgorithm.SIMPLE:
            success = self._pack_simple()
        else:
            success = self._pack_simple()  # Default to simple

        if not success:
            print("Error: Failed to pack sprites into atlas")
            return False

        # Calculate atlas dimensions
        atlas_width, atlas_height = self._calculate_atlas_size()

        if self.power_of_two:
            atlas_width = self._next_power_of_two(atlas_width)
            atlas_height = self._next_power_of_two(atlas_height)

        print(f"  Atlas size: {atlas_width}x{atlas_height}")

        # Create atlas image
        atlas = Image.new('RGBA', (atlas_width, atlas_height), (0, 0, 0, 0))

        # Copy sprites to atlas
        for sprite in self.sprites:
            if sprite.rect:
                if sprite.rotated:
                    # Rotate sprite 90 degrees
                    rotated_image = sprite.image.rotate(90, expand=True)
                    atlas.paste(rotated_image, (sprite.rect.x, sprite.rect.y))
                else:
                    atlas.paste(sprite.image, (sprite.rect.x, sprite.rect.y))

        # Save atlas image
        atlas.save(output_image)
        print(f"Saved atlas image: {output_image}")

        # Generate metadata
        metadata = self._generate_metadata(atlas_width, atlas_height)

        # Save metadata
        with open(output_data, 'w') as f:
            json.dump(metadata, f, indent=2)
        print(f"Saved atlas data: {output_data}")

        # Print statistics
        total_sprite_area = sum(s.image.width * s.image.height for s in self.sprites)
        atlas_area = atlas_width * atlas_height
        efficiency = (total_sprite_area / atlas_area) * 100

        print(f"\nAtlas Statistics:")
        print(f"  Sprites: {len(self.sprites)}")
        print(f"  Atlas size: {atlas_width}x{atlas_height}")
        print(f"  Packing efficiency: {efficiency:.1f}%")
        print(f"  Wasted space: {100 - efficiency:.1f}%")

        return True

    def _pack_simple(self) -> bool:
        """Simple row-based packing algorithm"""
        current_x = 0
        current_y = 0
        row_height = 0

        for sprite in self.sprites:
            sprite_width = sprite.image.width + self.padding * 2
            sprite_height = sprite.image.height + self.padding * 2

            # Check if we need a new row
            if current_x + sprite_width > self.max_width:
                current_x = 0
                current_y += row_height
                row_height = 0

            # Check if we exceed max height
            if current_y + sprite_height > self.max_height:
                return False

            # Place sprite
            sprite.rect = Rect(
                x=current_x + self.padding,
                y=current_y + self.padding,
                width=sprite.image.width,
                height=sprite.image.height
            )

            current_x += sprite_width
            row_height = max(row_height, sprite_height)

        return True

    def _pack_maxrects(self) -> bool:
        """MaxRects packing algorithm (simplified)"""
        # Start with one free rectangle
        free_rects = [Rect(0, 0, self.max_width, self.max_height)]

        for sprite in self.sprites:
            sprite_width = sprite.image.width + self.padding * 2
            sprite_height = sprite.image.height + self.padding * 2

            # Find best free rectangle
            best_rect_idx = -1
            best_score = float('inf')
            should_rotate = False

            for i, rect in enumerate(free_rects):
                # Try normal orientation
                if rect.width >= sprite_width and rect.height >= sprite_height:
                    score = min(rect.width - sprite_width, rect.height - sprite_height)
                    if score < best_score:
                        best_score = score
                        best_rect_idx = i
                        should_rotate = False

                # Try rotated orientation (90 degrees)
                if rect.width >= sprite_height and rect.height >= sprite_width:
                    score = min(rect.width - sprite_height, rect.height - sprite_width)
                    if score < best_score:
                        best_score = score
                        best_rect_idx = i
                        should_rotate = True

            if best_rect_idx == -1:
                return False  # Couldn't fit sprite

            # Place sprite in best rectangle
            rect = free_rects[best_rect_idx]

            if should_rotate:
                sprite.rect = Rect(
                    x=rect.x + self.padding,
                    y=rect.y + self.padding,
                    width=sprite.image.height,
                    height=sprite.image.width
                )
                sprite.rotated = True
                used_width = sprite_height
                used_height = sprite_width
            else:
                sprite.rect = Rect(
                    x=rect.x + self.padding,
                    y=rect.y + self.padding,
                    width=sprite.image.width,
                    height=sprite.image.height
                )
                sprite.rotated = False
                used_width = sprite_width
                used_height = sprite_height

            # Split the used rectangle
            new_rects = []

            # Right remainder
            if rect.width > used_width:
                new_rects.append(Rect(
                    x=rect.x + used_width,
                    y=rect.y,
                    width=rect.width - used_width,
                    height=used_height
                ))

            # Bottom remainder
            if rect.height > used_height:
                new_rects.append(Rect(
                    x=rect.x,
                    y=rect.y + used_height,
                    width=rect.width,
                    height=rect.height - used_height
                ))

            # Remove used rectangle and add new ones
            free_rects.pop(best_rect_idx)
            free_rects.extend(new_rects)

        return True

    def _calculate_atlas_size(self) -> Tuple[int, int]:
        """Calculate required atlas dimensions"""
        max_x = 0
        max_y = 0

        for sprite in self.sprites:
            if sprite.rect:
                max_x = max(max_x, sprite.rect.x + sprite.rect.width + self.padding)
                max_y = max(max_y, sprite.rect.y + sprite.rect.height + self.padding)

        return max_x, max_y

    def _next_power_of_two(self, value: int) -> int:
        """Get next power of two"""
        power = 1
        while power < value:
            power *= 2
        return power

    def _generate_metadata(self, atlas_width: int, atlas_height: int) -> Dict[str, Any]:
        """Generate atlas metadata"""
        metadata = {
            "width": atlas_width,
            "height": atlas_height,
            "sprites": []
        }

        for sprite in self.sprites:
            if sprite.rect:
                sprite_data = {
                    "name": sprite.name,
                    "x": sprite.rect.x,
                    "y": sprite.rect.y,
                    "width": sprite.rect.width,
                    "height": sprite.rect.height,
                    "u0": sprite.rect.x / atlas_width,
                    "v0": sprite.rect.y / atlas_height,
                    "u1": (sprite.rect.x + sprite.rect.width) / atlas_width,
                    "v1": (sprite.rect.y + sprite.rect.height) / atlas_height,
                    "rotated": sprite.rotated
                }

                if sprite.trimmed:
                    sprite_data["original_width"] = sprite.original_size[0]
                    sprite_data["original_height"] = sprite.original_size[1]
                    sprite_data["trimmed"] = True

                metadata["sprites"].append(sprite_data)

        return metadata


def main():
    parser = argparse.ArgumentParser(description='Generate texture atlas from sprites')
    parser.add_argument('input', help='Input directory containing sprites')
    parser.add_argument('output', help='Output atlas image path')
    parser.add_argument('-d', '--data', help='Output atlas data (JSON) path',
                       default=None)
    parser.add_argument('-w', '--width', type=int, default=2048,
                       help='Maximum atlas width (default: 2048)')
    parser.add_argument('-h', '--height', type=int, default=2048,
                       help='Maximum atlas height (default: 2048)', dest='atlas_height')
    parser.add_argument('-p', '--padding', type=int, default=2,
                       help='Padding between sprites (default: 2)')
    parser.add_argument('-a', '--algorithm',
                       choices=['simple', 'maxrects', 'skyline'],
                       default='maxrects',
                       help='Packing algorithm (default: maxrects)')
    parser.add_argument('--no-trim', action='store_true',
                       help='Disable alpha trimming')
    parser.add_argument('--no-pot', action='store_true',
                       help='Disable power-of-two sizing')

    args = parser.parse_args()

    # Determine data output path
    data_output = args.data
    if not data_output:
        output_path = Path(args.output)
        data_output = str(output_path.with_suffix('.json'))

    # Create generator
    generator = TextureAtlasGenerator(
        max_width=args.width,
        max_height=args.atlas_height,
        padding=args.padding,
        power_of_two=not args.no_pot
    )

    # Add sprites from directory
    sprite_count = generator.add_directory(args.input)
    if sprite_count == 0:
        print("Error: No sprites found")
        return 1

    print(f"Loaded {sprite_count} sprites")

    # Generate atlas
    algorithm = PackingAlgorithm(args.algorithm)
    success = generator.generate(
        output_image=args.output,
        output_data=data_output,
        algorithm=algorithm,
        trim_alpha=not args.no_trim
    )

    return 0 if success else 1


if __name__ == '__main__':
    sys.exit(main())
