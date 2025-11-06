import random

# Read products
with open('products.txt', 'r', encoding='utf-8') as f:
    lines = f.readlines()

updated_lines = []
for line in lines:
    line = line.strip()
    if not line:
        continue
    
    parts = line.split('|')
    
    # Check if rating already exists (11 fields)
    if len(parts) >= 11:
        # Already has rating
        updated_lines.append(line + '\n')
    else:
        # Generate a rating between 3.0 and 5.0
        # Higher probability for 4.0-5.0 ratings
        rating = round(random.triangular(3.0, 5.0, 4.5), 1)
        
        # Add rating field
        updated_line = line + '|' + str(rating)
        updated_lines.append(updated_line + '\n')
        print(f"Added rating {rating} to: {parts[1] if len(parts) > 1 else 'Unknown'}")

# Write back
with open('products.txt', 'w', encoding='utf-8') as f:
    f.writelines(updated_lines)

print(f"\nProcessed {len(updated_lines)} products")
