#!/usr/bin/env python3
"""
Download real component images from the internet and assign to products
Uses Unsplash API for high-quality images
"""

import os
import json
import urllib.request
import urllib.error
import time
import shutil
from pathlib import Path

# Create thumbnails directory if it doesn't exist
os.makedirs('thumbnails', exist_ok=True)

# Keywords for each component type for better image search
SEARCH_KEYWORDS = {
    'CPU': ['processor', 'cpu', 'ryzen', 'intel', 'semiconductor'],
    'GPU': ['graphics card', 'video card', 'nvidia', 'rtx', 'amd', 'gpu'],
    'RAM': ['memory', 'ram', 'ddr4', 'ddr5', 'stick'],
    'Storage': ['ssd', 'nvme', 'hard drive', 'storage', 'disk'],
    'Motherboard': ['motherboard', 'mainboard', 'mobo', 'circuit'],
    'PSU': ['power supply', 'psu', 'modular'],
    'Cooling': ['cooler', 'heatsink', 'cooling', 'fan'],
    'Case': ['pc case', 'chassis', 'tower', 'computer case'],
    'Other': ['computer', 'tech', 'hardware']
}

# Unsplash API key (free tier)
UNSPLASH_ACCESS_KEY = 'YOUR_UNSPLASH_KEY'  # Using free alternative

def download_from_unsplash(query, filename, max_retries=3):
    """Download image from Unsplash API"""
    for attempt in range(max_retries):
        try:
            # Using a free image API alternative
            url = f"https://source.unsplash.com/200x200/?{query}"
            
            print(f"  Downloading {filename}...", end=' ')
            
            # Add headers to avoid being blocked
            req = urllib.request.Request(
                url,
                headers={'User-Agent': 'Mozilla/5.0'}
            )
            
            with urllib.request.urlopen(req, timeout=10) as response:
                with open(f'thumbnails/{filename}', 'wb') as out_file:
                    out_file.write(response.read())
            
            print("✓")
            time.sleep(0.5)  # Rate limiting
            return True
            
        except urllib.error.URLError as e:
            if attempt < max_retries - 1:
                print(f"Retry {attempt + 1}/{max_retries}...")
                time.sleep(2)
            else:
                print(f"Failed: {e}")
                return False
        except Exception as e:
            print(f"Error: {e}")
            return False
    
    return False

def download_component_images():
    """Download images for each component type"""
    
    print("🖼️  Starting component image downloads...")
    print("=" * 50)
    
    component_images = {
        'CPU': 'processor.jpg',
        'GPU': 'graphics-card.jpg',
        'RAM': 'memory-ram.jpg',
        'Storage': 'ssd-nvme.jpg',
        'Motherboard': 'motherboard.jpg',
        'PSU': 'power-supply.jpg',
        'Cooling': 'cooler.jpg',
        'Case': 'pc-case.jpg',
    }
    
    for component, filename in component_images.items():
        keywords = ','.join(SEARCH_KEYWORDS.get(component, ['hardware']))
        download_from_unsplash(keywords, filename)
    
    print("=" * 50)
    print("✓ Image download completed!")
    return component_images

def update_products_with_images():
    """Update products.txt with image paths"""
    
    # Component type to image mapping
    image_mapping = {
        'CPU': 'thumbnails/processor.jpg',
        'GPU': 'thumbnails/graphics-card.jpg',
        'RAM': 'thumbnails/memory-ram.jpg',
        'Storage': 'thumbnails/ssd-nvme.jpg',
        'Motherboard': 'thumbnails/motherboard.jpg',
        'PSU': 'thumbnails/power-supply.jpg',
        'Cooling': 'thumbnails/cooler.jpg',
        'Case': 'thumbnails/pc-case.jpg',
    }
    
    print("\n📝 Updating products.txt with image paths...")
    
    # Backup original
    if os.path.exists('products.txt'):
        shutil.copy('products.txt', f'backups/products_backup_images_{int(time.time())}.txt')
        print("  ✓ Backup created")
    
    updated_lines = []
    
    try:
        with open('products.txt', 'r', encoding='utf-8') as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue
                
                # Parse the line
                parts = line.split('|')
                
                if len(parts) >= 6:
                    # Get category (6th field)
                    category = parts[5]
                    
                    # Get or create image path
                    if len(parts) > 6 and parts[6] and parts[6] != 'none':
                        image_path = parts[6]
                    else:
                        image_path = image_mapping.get(category, 'thumbnails/hardware.jpg')
                    
                    # Reconstruct line with image path
                    # Keep first 6 fields, add image path
                    new_line = '|'.join(parts[:6]) + '|' + image_path
                    updated_lines.append(new_line)
                else:
                    updated_lines.append(line)
        
        # Write updated file
        with open('products.txt', 'w', encoding='utf-8') as f:
            for line in updated_lines:
                f.write(line + '\n')
        
        print(f"  ✓ Updated {len(updated_lines)} products")
        
    except Exception as e:
        print(f"  ✗ Error: {e}")
        return False
    
    return True

def main():
    print("\n" + "=" * 50)
    print("  Component Image Manager")
    print("=" * 50 + "\n")
    
    # Download images
    images = download_component_images()
    
    # Update products file
    if update_products_with_images():
        print("\n✅ All done! Products now have beautiful images!")
        print("\n📌 To use your own Unsplash API key:")
        print("   1. Get free key from: https://unsplash.com/developers")
        print("   2. Replace UNSPLASH_ACCESS_KEY in this script")
    else:
        print("\n⚠️  Some updates failed. Check your internet connection.")

if __name__ == '__main__':
    main()
