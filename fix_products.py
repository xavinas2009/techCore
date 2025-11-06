import random

random.seed(100)

# Read the file
with open('products.txt', 'r', encoding='utf-8') as f:
    lines = f.readlines()

new_lines = []
for line in lines:
    line = line.strip()
    if not line:
        continue
    
    # Split by pipe
    parts = line.split('|')
    
    # Take only the first 7 fields (before duplicates): id|name|desc|price|color|category|imagePath
    if len(parts) >= 6:
        # Get first 6 fields
        clean_parts = parts[:6]
        
        # Add imagePath if it exists and isn't a duplicate
        if len(clean_parts) == 6:
            clean_parts.append('thumbnails/' + clean_parts[5].lower() + '.png')  # Generate image path
        
        # Add stock status (80% in stock)
        in_stock = '1' if random.random() > 0.20 else '0'
        clean_parts.append(in_stock)
        
        # Add discount status and percentage (30% have discounts)
        if random.random() < 0.30:
            discount_percent = random.randint(10, 40)
            clean_parts.append('1')
            clean_parts.append(str(discount_percent))
        else:
            clean_parts.append('0')
            clean_parts.append('0')
        
        new_lines.append('|'.join(clean_parts) + '\n')

# Write back
with open('products.txt', 'w', encoding='utf-8') as f:
    f.writelines(new_lines)

# Count special cases
out_of_stock = len([l for l in new_lines if '|0|' in l])
on_discount = len([l for l in new_lines if l.count('|1|') >= 1 and not l.endswith('|0\n')])
both = len([l for l in new_lines if '|0|1|' in l])

print(f"Updated {len(new_lines)} products:")
print(f"- {out_of_stock} out of stock")
print(f"- {on_discount} on discount")
print(f"- {both} BOTH out of stock AND on discount (clearance!)")
