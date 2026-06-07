# Objetivo
Este diretório armazena os blocos reutilizáveis de código HTML/CSS (**partials**) que são dinamicamente injetados nas páginas principais durante a compilação.

# Conteúdo
* **[head.html](head.html):** Contém os metadados de viewport, conjunto de caracteres UTF-8, fontes do Google e toda a estilização CSS base (design de Glassmorphism).
* **[footer.html](footer.html):** Rodapé padrão do sistema.

# Dependências
Esses arquivos são processados exclusivamente pelo script [html2c.py](../../html2c.py) na raiz do repositório para gerar os cabeçalhos C++ correspondentes em `include/`.
