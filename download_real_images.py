#!/usr/bin/env python3
"""
Script alternativo - Baixa imagens diretamente de fontes de hardware
Requer: pip install requests pillow
"""

import requests
from PIL import Image
from io import BytesIO
import os

def download_and_resize_image(url, filename, size=(100, 100)):
    """Baixa uma imagem e redimensiona para thumbnail"""
    try:
        print(f"Baixando {filename}...")
        headers = {
            'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36'
        }
        response = requests.get(url, headers=headers, timeout=15)
        response.raise_for_status()
        
        img = Image.open(BytesIO(response.content))
        img = img.convert('RGB')
        img.thumbnail(size, Image.Resampling.LANCZOS)
        
        thumb = Image.new('RGB', size, (40, 40, 45))
        offset = ((size[0] - img.size[0]) // 2, (size[1] - img.size[1]) // 2)
        thumb.paste(img, offset)
        
        thumb.save(f"thumbnails/{filename}")
        print(f"✓ {filename}")
        return True
    except Exception as e:
        print(f"✗ Erro: {filename} - {e}")
        return False

os.makedirs("thumbnails", exist_ok=True)

# URLs de exemplo de produtos reais
# NOTA: Você pode substituir por URLs de lojas específicas
images = {
    # CPUs - Exemplos genéricos
    "cpu_amd.png": "https://m.media-amazon.com/images/I/61vGQNUEsGL._AC_SL1000_.jpg",
    "cpu_intel.png": "https://m.media-amazon.com/images/I/51vvM5CXjjL._AC_SL1000_.jpg",
    
    # GPUs
    "gpu_nvidia.png": "https://m.media-amazon.com/images/I/51vvM5CXjjL._AC_SL1000_.jpg",
    "gpu_amd.png": "https://m.media-amazon.com/images/I/81+jvBpl8mL._AC_SL1500_.jpg",
    
    # RAM
    "ram.png": "https://m.media-amazon.com/images/I/51Bja+WEIXL._AC_SL1000_.jpg",
    
    # Storage
    "storage_nvme.png": "https://m.media-amazon.com/images/I/71CcV9F6-NL._AC_SL1500_.jpg",
    "storage_sata.png": "https://m.media-amazon.com/images/I/71KJ2NWqQjL._AC_SL1500_.jpg",
    
    # Motherboards
    "mobo_amd.png": "https://m.media-amazon.com/images/I/81VN0lXSBkL._AC_SL1500_.jpg",
    "mobo_intel.png": "https://m.media-amazon.com/images/I/81l0-OzOqaL._AC_SL1500_.jpg",
    
    # PSU
    "psu.png": "https://m.media-amazon.com/images/I/71r9b2I7S6L._AC_SL1500_.jpg",
    
    # Cooling
    "cooling.png": "https://m.media-amazon.com/images/I/71Fb1i-rWUL._AC_SL1500_.jpg",
    
    # Case
    "case.png": "https://m.media-amazon.com/images/I/71yK3MsN+lL._AC_SL1500_.jpg",
    
    # Peripheral
    "peripheral.png": "https://m.media-amazon.com/images/I/61hYJqZ9YYL._AC_SL1000_.jpg",
}

print("\n" + "="*60)
print("BAIXANDO IMAGENS DE PRODUTOS REAIS")
print("="*60 + "\n")

success = sum(1 for f, u in images.items() if download_and_resize_image(u, f))

print(f"\n{'='*60}")
print(f"Concluído: {success}/{len(images)} imagens")
print("="*60)
