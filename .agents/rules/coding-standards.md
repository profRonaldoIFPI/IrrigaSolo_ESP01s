---
trigger: always_on
---

# Padrões de Codificação e Estilo

## Idioma e Nomenclatura

Todo o código-fonte gerado ou refatorado deve ser escrito em **Português**.

A nomenclatura de variáveis e funções deve utilizar estritamente o formato `camelCase`.

A nomenclatura deve ser descritiva e autoexplicativa, evitando abreviações confusas ou curtas demais.

Nomes de classes, estruturas (structs) e enumerações devem utilizar `PascalCase`.

Constantes, macros e definições de pinos de hardware devem utilizar `SCREAMING_SNAKE_CASE`.

## Práticas de Comentários e Documentação

O código deve ser altamente didático e fácil de ser compreendido por desenvolvedores em fase de aprendizado ou pesquisadores revisando a lógica.

Os comentários devem explicar a **intenção** e a **regra de negócio** (o "porquê" de uma decisão técnica) e não apenas traduzir a sintaxe (o "como" o código faz). Quando for o caso mencionar a especificação ou o "Registro de Decisões Arquiteturais" atendida.

Funções complexas ou blocos de controle de hardware devem conter um comentário de cabeçalho conciso detalhando o comportamento esperado, os parâmetros exigidos e os retornos.

Evite comentários óbvios. O código deve ser limpo o suficiente para ser lido com naturalidade.

## Regras de Submissão de Código

Antes de finalizar qualquer implementação, o agente deve validar silenciosamente se as regras de idioma, `camelCase` e as boas práticas de comentários foram rigorosamente aplicadas.

Qualquer violação deste padrão deve ser corrigida antes da entrega do código.