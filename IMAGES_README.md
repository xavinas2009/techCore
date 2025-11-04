# Download de Imagens de Produtos

## Opção 1: Usar imagens geradas (JÁ FEITO ✓)
As imagens já foram criadas na pasta `thumbnails/` usando gráficos procedurais.

## Opção 2: Baixar imagens reais da internet

### Passo 1: Instalar dependências Python
```powershell
pip install requests pillow
```

### Passo 2: Executar um dos scripts

**Script 1 - Imagens artísticas do Unsplash:**
```powershell
python download_images.py
```

**Script 2 - Imagens de produtos reais:**
```powershell
python download_real_images.py
```

### Passo 3: Usar as imagens

Ao criar produtos no painel Admin, use estes caminhos:

| Categoria | Caminho da Imagem |
|-----------|------------------|
| CPU AMD | `thumbnails/cpu_amd.png` |
| CPU Intel | `thumbnails/cpu_intel.png` |
| GPU NVIDIA | `thumbnails/gpu_nvidia.png` |
| GPU AMD | `thumbnails/gpu_amd.png` |
| RAM | `thumbnails/ram.png` |
| Storage NVMe | `thumbnails/storage_nvme.png` |
| Storage SATA | `thumbnails/storage_sata.png` |
| Motherboard AMD | `thumbnails/mobo_amd.png` |
| Motherboard Intel | `thumbnails/mobo_intel.png` |
| PSU | `thumbnails/psu.png` |
| Cooling | `thumbnails/cooling.png` |
| Case | `thumbnails/case.png` |
| Peripheral | `thumbnails/peripheral.png` |

## Opção 3: Adicionar suas próprias imagens

1. Coloque qualquer imagem PNG/JPG na pasta `thumbnails/`
2. No Admin, digite o caminho completo: `thumbnails/seu_arquivo.png`

**Dica:** Imagens serão automaticamente redimensionadas para 100x100 pixels.
