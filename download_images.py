#!/usr/bin/env python3
"""
Script para baixar imagens de produtos de hardware
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
        response = requests.get(url, timeout=10)
        response.raise_for_status()
        
        # Abrir e redimensionar imagem
        img = Image.open(BytesIO(response.content))
        img = img.convert('RGB')  # Converter para RGB
        img.thumbnail(size, Image.Resampling.LANCZOS)
        
        # Criar imagem com fundo e centralizar
        thumb = Image.new('RGB', size, (40, 40, 45))
        offset = ((size[0] - img.size[0]) // 2, (size[1] - img.size[1]) // 2)
        thumb.paste(img, offset)
        
        # Salvar
        thumb.save(f"thumbnails/{filename}")
        print(f"✓ {filename} salvo com sucesso!")
        return True
    except Exception as e:
        print(f"✗ Erro ao baixar {filename}: {e}")
        return False

# Criar diretório se não existir
os.makedirs("thumbnails", exist_ok=True)

print("=" * 60)
print("DOWNLOAD DE IMAGENS DE PRODUTOS - TECHCORE")
print("=" * 60)
print()

# URLs de imagens de produtos (usando Unsplash e fontes públicas)
# Você pode substituir por URLs reais de produtos

images = {
    # CPUs
    "cpu_amd.png": "https://images.unsplash.com/photo-1591799264318-7e6ef8ddb7ea?w=200",  # AMD CPU
    "cpu_intel.png": "https://images.unsplash.com/photo-1555617981-dac3880eac6e?w=200",  # Intel CPU
    
    # GPUs
    "gpu_nvidia.png": "https://images.unsplash.com/photo-1587202372634-32705e3bf49c?w=200",  # GPU
    "gpu_amd.png": "https://images.unsplash.com/photo-1591488320449-011701bb6704?w=200",  # GPU AMD
    
    # RAM
    "ram.png": "https://images.unsplash.com/photo-1562976540-1502c2145186?w=200",  # RAM
    
    # Storage
    "storage_nvme.png": "https://images.unsplash.com/photo-1597872200969-2b65d56bd16b?w=200",  # SSD
    "storage_sata.png": "https://images.unsplash.com/photo-1531492746076-161ca9bcad58?w=200",  # SSD SATA
    
    # Motherboards
    "mobo_amd.png": "https://images.unsplash.com/photo-1591238372408-c8b02f13fd48?w=200",  # Motherboard
    "mobo_intel.png": "https://images.unsplash.com/photo-1563968743333-044cef800494?w=200",  # Motherboard
    
    # PSU
    "psu.png": "https://images.unsplash.com/photo-1607827448299-a099dc8a9f67?w=200",  # PSU
    
    # Cooling
    "cooling.png": "https://images.unsplash.com/photo-1587202372775-e229f172b9d7?w=200",  # Cooler
    
    # Case
    "case.png": "https://images.unsplash.com/photo-1587202372583-49330a15584d?w=200",  # PC Case
    
    # Peripherals
    "peripheral.png": "https://images.unsplash.com/photo-1587829741301-dc798b83add3?w=200",  # Gaming setup
}

# Baixar todas as imagens
success_count = 0
total = len(images)

for filename, url in images.items():
    if download_and_resize_image(url, filename):
        success_count += 1

print()
print("=" * 60)
print(f"Concluído! {success_count}/{total} imagens baixadas com sucesso")
print("=" * 60)
print()
print("INSTRUÇÕES:")
print("No painel Admin, ao criar produtos, use estes caminhos:")
print()
print("CPUs AMD:        thumbnails/cpu_amd.png")
print("CPUs Intel:      thumbnails/cpu_intel.png")
print("GPUs NVIDIA:     thumbnails/gpu_nvidia.png")
print("GPUs AMD:        thumbnails/gpu_amd.png")
print("RAM:             thumbnails/ram.png")
print("Storage NVMe:    thumbnails/storage_nvme.png")
print("Storage SATA:    thumbnails/storage_sata.png")
print("Mobo AMD:        thumbnails/mobo_amd.png")
print("Mobo Intel:      thumbnails/mobo_intel.png")
print("PSU:             thumbnails/psu.png")
print("Cooling:         thumbnails/cooling.png")
print("Case:            thumbnails/case.png")
print("Peripherals:     thumbnails/peripheral.png")
print()
