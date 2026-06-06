# html2c.py
try:
    Import("env")
except NameError:
    pass

import os
import glob

# Certificar que a pasta 'include' existe
if not os.path.exists("include"):
    os.makedirs("include")

# Varrer todos os arquivos .html no diretório include/
html_files = glob.glob(os.path.join("include", "*.html"))

for source_file in html_files:
    # Obter o nome base do arquivo (ex: 'config' de 'include/config.html')
    base_name = os.path.splitext(os.path.basename(source_file))[0]
    header_file = os.path.join("include", f"{base_name}.h")

    # Lê o conteúdo do arquivo HTML
    with open(source_file, "r", encoding="utf-8") as f:
        html_content = f.read()

    # Formata o conteúdo para um C++ raw string literal
    # O nome da variável será no formato: <nome_do_arquivo>_html
    header_content = '#pragma once\n\n'
    header_content += f'const char {base_name}_html[] PROGMEM = R"rawliteral(\n'
    header_content += html_content
    header_content += '\n)rawliteral";\n'

    # Escreve o arquivo .h
    with open(header_file, "w", encoding="utf-8") as f:
        f.write(header_content)

    print(f"[HTML2C] '{source_file}' foi convertido com sucesso para '{header_file}'")