#!/usr/bin/env python3
"""
Script para adicionar automaticamente paths de imagens aos produtos existentes
baseado nas suas categorias
"""

import os

# Mapeamento de categorias para imagens
category_image_map = {
    "CPU": {
        "amd": "thumbnails/cpu_amd.png",
        "ryzen": "thumbnails/cpu_amd.png",
        "intel": "thumbnails/cpu_intel.png",
        "core": "thumbnails/cpu_intel.png",
    },
    "GPU": {
        "nvidia": "thumbnails/gpu_nvidia.png",
        "rtx": "thumbnails/gpu_nvidia.png",
        "geforce": "thumbnails/gpu_nvidia.png",
        "amd": "thumbnails/gpu_amd.png",
        "radeon": "thumbnails/gpu_amd.png",
        "rx": "thumbnails/gpu_amd.png",
    },
    "RAM": "thumbnails/ram.png",
    "Storage": {
        "nvme": "thumbnails/storage_nvme.png",
        "gen4": "thumbnails/storage_nvme.png",
        "gen3": "thumbnails/storage_nvme.png",
        "sata": "thumbnails/storage_sata.png",
        "evo": "thumbnails/storage_sata.png",
    },
    "Motherboard": {
        "amd": "thumbnails/mobo_amd.png",
        "am4": "thumbnails/mobo_amd.png",
        "am5": "thumbnails/mobo_amd.png",
        "b550": "thumbnails/mobo_amd.png",
        "x570": "thumbnails/mobo_amd.png",
        "b650": "thumbnails/mobo_amd.png",
        "x670": "thumbnails/mobo_amd.png",
        "intel": "thumbnails/mobo_intel.png",
        "lga": "thumbnails/mobo_intel.png",
        "z690": "thumbnails/mobo_intel.png",
        "z790": "thumbnails/mobo_intel.png",
    },
    "PSU": "thumbnails/psu.png",
    "Cooling": "thumbnails/cooling.png",
    "Case": "thumbnails/case.png",
    "Peripheral": "thumbnails/peripheral.png",
}

def get_image_for_product(name, description, category):
    """Determina a imagem apropriada baseado no nome, descrição e categoria"""
    
    # Converter para minúsculas para busca
    name_lower = name.lower()
    desc_lower = description.lower()
    combined = name_lower + " " + desc_lower
    
    if category not in category_image_map:
        return "none"
    
    cat_mapping = category_image_map[category]
    
    # Se for um dicionário, procura por palavras-chave
    if isinstance(cat_mapping, dict):
        for keyword, image_path in cat_mapping.items():
            if keyword.lower() in combined:
                return image_path
        # Se não encontrar, usa a primeira opção
        return list(cat_mapping.values())[0]
    else:
        # Se for string direta, retorna ela
        return cat_mapping

def process_products_file():
    """Lê products.txt e adiciona imagens automaticamente"""
    
    if not os.path.exists("products.txt"):
        print("❌ Arquivo products.txt não encontrado!")
        print("   Crie produtos primeiro no painel Admin.")
        return
    
    # Ler produtos existentes
    products = []
    with open("products.txt", "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            
            parts = line.split("|")
            if len(parts) < 6:
                continue
            
            product_id = parts[0]
            name = parts[1]
            description = parts[2]
            price = parts[3]
            color = parts[4]
            category = parts[5]
            
            # Se já tem imagem (7 partes), mantém
            if len(parts) >= 7 and parts[6] != "none" and parts[6].strip():
                image_path = parts[6]
            else:
                # Adiciona imagem automaticamente
                image_path = get_image_for_product(name, description, category)
            
            products.append({
                'id': product_id,
                'name': name,
                'desc': description,
                'price': price,
                'color': color,
                'category': category,
                'image': image_path
            })
    
    if not products:
        print("❌ Nenhum produto encontrado no arquivo.")
        return
    
    # Criar backup
    import shutil
    from datetime import datetime
    backup_name = f"products_backup_{datetime.now().strftime('%Y%m%d_%H%M%S')}.txt"
    shutil.copy("products.txt", f"backups/{backup_name}")
    print(f"✓ Backup criado: backups/{backup_name}")
    
    # Reescrever arquivo com imagens
    with open("products.txt", "w", encoding="utf-8") as f:
        for p in products:
            f.write(f"{p['id']}|{p['name']}|{p['desc']}|{p['price']}|{p['color']}|{p['category']}|{p['image']}\n")
    
    print(f"\n✓ {len(products)} produtos atualizados com imagens!")
    print("\nResumo:")
    
    # Mostrar estatísticas
    image_count = {}
    for p in products:
        img = p['image']
        if img not in image_count:
            image_count[img] = 0
        image_count[img] += 1
    
    for img, count in sorted(image_count.items()):
        if img != "none":
            print(f"  • {img}: {count} produtos")
    
    print("\n✓ Pronto! Execute o programa e veja as imagens nos produtos.")

if __name__ == "__main__":
    print("="*60)
    print("ADICIONAR IMAGENS AUTOMATICAMENTE AOS PRODUTOS")
    print("="*60)
    print()
    
    # Criar diretório de backups se não existir
    os.makedirs("backups", exist_ok=True)
    
    process_products_file()
    
    print()
    print("="*60)
