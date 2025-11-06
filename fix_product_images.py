#!/usr/bin/env python3
"""
Fix product image paths to match actual thumbnail files.
Intelligently assigns the correct image based on product name and category.
"""

import os

# Read products
with open('products.txt', 'r') as f:
    lines = f.readlines()

# Mapping logic for each category
def get_image_path(name, desc, category):
    """Determine the correct image path based on product details."""
    name_lower = name.lower()
    desc_lower = desc.lower()
    
    if category == "CPU":
        if "amd" in name_lower or "ryzen" in name_lower:
            return "thumbnails/cpu_amd.png"
        elif "intel" in name_lower or "core" in name_lower:
            return "thumbnails/cpu_intel.png"
        return "thumbnails/cpu_amd.png"  # default
    
    elif category == "GPU":
        if "nvidia" in name_lower or "rtx" in name_lower or "gtx" in name_lower:
            return "thumbnails/gpu_nvidia.png"
        elif "amd" in name_lower or "radeon" in name_lower or "rx" in name_lower:
            return "thumbnails/gpu_amd.png"
        return "thumbnails/gpu_nvidia.png"  # default
    
    elif category == "RAM":
        return "thumbnails/ram.png"
    
    elif category == "Storage":
        if "nvme" in desc_lower or "gen4" in desc_lower or "gen5" in desc_lower:
            return "thumbnails/storage_nvme.png"
        elif "sata" in desc_lower or "evo" in desc_lower or "ssd" in desc_lower:
            return "thumbnails/storage_sata.png"
        return "thumbnails/storage_nvme.png"  # default
    
    elif category == "Motherboard":
        if "amd" in name_lower or "x870" in name_lower or "am5" in desc_lower:
            return "thumbnails/mobo_amd.png"
        elif "intel" in name_lower or "z790" in name_lower or "lga1700" in desc_lower:
            return "thumbnails/mobo_intel.png"
        return "thumbnails/mobo_amd.png"  # default
    
    elif category == "PSU":
        return "thumbnails/psu.png"
    
    elif category == "Cooling":
        return "thumbnails/cooling.png"
    
    elif category == "Case":
        return "thumbnails/case.png"
    
    else:
        return "thumbnails/peripheral.png"  # default for peripherals

# Process each line
updated_lines = []
changes = 0

for line in lines:
    if not line.strip() or line.startswith('#'):
        updated_lines.append(line)
        continue
    
    parts = line.strip().split('|')
    if len(parts) < 7:
        updated_lines.append(line)
        continue
    
    product_id = parts[0]
    name = parts[1]
    desc = parts[2]
    price = parts[3]
    color = parts[4]
    category = parts[5]
    old_image = parts[6]
    
    # Get new image path
    new_image = get_image_path(name, desc, category)
    
    # Only update if different
    if old_image != new_image:
        print(f"[ID {product_id}] {name}")
        print(f"  Old: {old_image}")
        print(f"  New: {new_image}")
        changes += 1
        parts[6] = new_image
    
    # Reconstruct line
    updated_lines.append('|'.join(parts) + '\n')

# Write back
with open('products.txt', 'w') as f:
    f.writelines(updated_lines)

print(f"\n✓ Updated {changes} products with correct image paths!")
print(f"✓ Total products: {len([l for l in updated_lines if l.strip() and not l.startswith('#')])}")
