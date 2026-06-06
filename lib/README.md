# Diretório `/lib` — Bibliotecas do Projeto

## Objetivo

Este diretório é destinado a bibliotecas específicas do projeto (bibliotecas privadas). Diferente das dependências externas gerenciadas pelo `lib_deps` do `platformio.ini`, aqui ficam módulos reutilizáveis desenvolvidos internamente.

## Conteúdo

| Arquivo | Responsabilidade |
|---|---|
| `README` | Guia original do PlatformIO sobre estrutura de bibliotecas. |

> **Nota:** Atualmente não há bibliotecas privadas. As dependências externas (`ArduinoJson`, `NTPClient`) são importadas manualmente pelo usuário via PlatformIO.

## Dependências / Relações

Bibliotecas colocadas aqui seriam automaticamente detectadas pelo PlatformIO e disponibilizadas para inclusão em `/src` e `/include`. O sistema de build resolve as dependências automaticamente.
