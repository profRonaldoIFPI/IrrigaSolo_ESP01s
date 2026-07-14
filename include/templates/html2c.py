# html2c.py
try:
    import env
except ImportError:
    pass

import os
import glob

# SCons (PlatformIO) não define __file__ na execução do script.
# Como o SConscript é invocado no diretório raiz do projeto PlatformIO,
# podemos basear os caminhos a partir do diretório de trabalho atual (cwd).
BASE_DIR = os.getcwd()
TEMPLATES_DIR = os.path.join(BASE_DIR, "include", "templates")
PARTIALS_DIR = os.path.join(TEMPLATES_DIR, "partials")
INCLUDE_DIR = os.path.join(BASE_DIR, "include")

# --- Ler e Injetar Variáveis do arquivo .env como macros C++ ---
env_vars = {}
env_file_path = os.path.join(BASE_DIR, ".env")
if os.path.exists(env_file_path):
    with open(env_file_path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#") or "=" not in line:
                continue
            parts = line.split("=", 1)
            if len(parts) == 2:
                key, val = parts
                env_vars[key.strip()] = val.strip()

try:
    from SCons.Script import Import
    Import("env")
    cpp_defines = []
    if "SSID" in env_vars:
        cpp_defines.append(("ENV_WIFI_SSID", f'\\"{env_vars["SSID"]}\\"'))
    if "SENHA" in env_vars:
        cpp_defines.append(("ENV_WIFI_SENHA", f'\\"{env_vars["SENHA"]}\\"'))
    if "IP" in env_vars:
        cpp_defines.append(("ENV_MQTT_IP", f'\\"{env_vars["IP"]}\\"'))
    if "USER" in env_vars:
        cpp_defines.append(("ENV_MQTT_USER", f'\\"{env_vars["USER"]}\\"'))
    if "PASS" in env_vars:
        cpp_defines.append(("ENV_MQTT_PASS", f'\\"{env_vars["PASS"]}\\"'))
    
    if cpp_defines:
        env.Append(CPPDEFINES=cpp_defines)
        print(f"[ENV_LOAD] Injetadas macros C++: {cpp_defines}")
except Exception as e:
    pass

# Carrega os templates parciais se existirem
head_partial = ""
footer_partial = ""

head_path = os.path.join(PARTIALS_DIR, "head.html")
footer_path = os.path.join(PARTIALS_DIR, "footer.html")

if os.path.exists(head_path):
    with open(head_path, "r", encoding="utf-8") as f:
        head_partial = f.read()

if os.path.exists(footer_path):
    with open(footer_path, "r", encoding="utf-8") as f:
        footer_partial = f.read()

# Varrer todos os arquivos .html no diretório de templates
html_files = glob.glob(os.path.join(TEMPLATES_DIR, "*.html"))

for source_file in html_files:
    # Obter o nome base do arquivo (ex: 'config' de 'include/templates/config.html')
    base_name = os.path.splitext(os.path.basename(source_file))[0]
    header_file = os.path.join(INCLUDE_DIR, f"{base_name}.h")

    # Lê o conteúdo do arquivo HTML
    with open(source_file, "r", encoding="utf-8") as f:
        html_content = f.read()

    # Aplica o pré-processamento dos partials/templates
    if head_partial:
        html_content = html_content.replace("<!-- template:head -->", head_partial)
    if footer_partial:
        html_content = html_content.replace("<!-- template:footer -->", footer_partial)

    # Formata o conteúdo para um C++ raw string literal
    # O nome da variável será no formato: <nome_do_arquivo>_html
    header_content = '#pragma once\n\n'
    header_content += f'const char {base_name}_html[] PROGMEM = R"rawliteral(\n'
    header_content += html_content
    header_content += '\n)rawliteral";\n'

    # Escreve o arquivo .h na pasta include/
    with open(header_file, "w", encoding="utf-8") as f:
        f.write(header_content)

    print(f"[HTML_2_C] '{source_file}' (com partials) foi convertido para '{header_file}'")