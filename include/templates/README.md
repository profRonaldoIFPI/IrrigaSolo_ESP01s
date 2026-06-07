# Objetivo

Este diretório é responsável por armazenar os blocos de códigos reutilizáveis HTML/CSS (**partials**) que contêm a estilização CSS comum de Glassmorphism e elementos compartilhados (como o cabeçalho base e o rodapé). Esses blocos são mesclados de forma transparente nas páginas web estáticas do firmware IrrigaSolo em tempo de compilação, mantendo o desenvolvimento modular e livre de repetições redundantes (DRY).

# Conteúdo

| Arquivo | Descrição |
|---|---|
| [head.html](head.html) | Contém os metadados comuns, folhas de estilo CSS para interface e Glassmorphism, fontes externas e definições globais do tema. |
| [footer.html](footer.html) | Contém a marcação comum do rodapé do sistema. |

# Dependências / Relações

Os arquivos desta pasta não são incluídos diretamente no microcontrolador em tempo de execução. Em vez disso, o script de build [html2c.py](../../html2c.py) intercepta estes arquivos parciais e substitui os delimitadores comentados `<!-- template:head -->` e `<!-- template:footer -->` nas páginas HTML principais (`index.html`, `horarios.html`, `config.html`) antes de convertê-los em arquivos de cabeçalho C++ (`.h`) contendo matrizes `PROGMEM`.

O fluxo de processamento visual é representado no diagrama abaixo:

```mermaid
flowchart TD
    subgraph include/templates/
        Head["head.html (CSS/Fonts)"]
        Footer["footer.html (Rodapé)"]
    end

    subgraph include/ (Páginas Web)
        Index["index.html"]
        Horarios["horarios.html"]
        Config["config.html"]
    end

    Head -->|Leitura e Injeção| Python["html2c.py (Builder Script)"]
    Footer -->|Leitura e Injeção| Python
    Index -->|Mesclagem em Placeholders| Python
    Horarios -->|Mesclagem em Placeholders| Python
    Config -->|Mesclagem em Placeholders| Python

    Python -->|Geração de Arrays PROGMEM| H_Files["Arquivos Header (*.h)"]
    H_Files -->|Inclusão compilada| CPP["servidorWeb.cpp"]
```

O diagrama ilustra didaticamente como as partes visuais comuns e as páginas específicas são combinadas pelo script em Python para gerar arquivos C++ limpos prontos para a memória Flash do microcontrolador.
