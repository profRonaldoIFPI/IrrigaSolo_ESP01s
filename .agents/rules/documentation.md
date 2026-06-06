---
trigger: always_on
---

# Padrões de Documentação de Diretórios

## Regra do README Obrigatório

Todo e qualquer diretório do projeto (incluindo subdiretórios) deve conter um arquivo `README.md`.

Sempre que o agente criar um novo diretório, ou adicionar/remover arquivos estruturais dentro de um diretório existente, ele deve criar ou atualizar o respectivo `README.md`.

## Abordagem Didática e Público-Alvo

O repositório é público e tem como público-alvo prioritário **estudantes de tecnologia**.

A linguagem da documentação deve ser técnica, porém extremamente didática, clara e acessível.

O texto deve guiar o estudante, explicando o papel daquele diretório específico dentro da arquitetura geral do sistema (o "porquê" de ele existir ali).

## Estrutura Exigida para o README.md

Cada arquivo `README.md` de diretório deve conter, no mínimo, as seguintes seções:

* **Objetivo:** Um parágrafo resumindo a responsabilidade daquele diretório.
* **Conteúdo:** Uma lista ou tabela descrevendo os arquivos principais contidos na pasta e o que cada um faz.
* **Dependências / Relações:** Uma breve explicação de como os arquivos dessa pasta interagem com o resto do sistema (ex: "Os headers desta pasta são consumidos pelo `main.cpp` na pasta `/src`").

## Uso Obrigatório de Diagramas Visuais

Para facilitar a compreensão abstrata, o agente deve incluir diagramas UML utilizando a sintaxe **Mermaid** (` ```mermaid `) sempre que for possível ilustrar um conceito.

Tipos de diagramas recomendados dependendo do contexto do diretório:

* **Fluxogramas (Flowcharts):** Excelentes para ilustrar a lógica de inicialização (ex: diretório `/src`) ou regras de negócio de agendamento.
* **Diagramas de Estado (State Diagrams):** Ideais para documentar transições de hardware, estados dos relés ou o fluxo de conexão Wi-Fi (AP/STA).
* **Diagramas de Classe ou Componentes:** Úteis para mostrar a relação entre as estruturas de dados (ex: diretório `/include` ou `/lib`).

Ao gerar um diagrama Mermaid, adicione um breve texto explicativo antes ou depois da imagem renderizada para garantir que o estudante compreenda o fluxo desenhado.