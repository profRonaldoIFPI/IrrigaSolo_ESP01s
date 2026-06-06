---
trigger: always_on
---

# Regra SDD (Desenvolvimento Guiado por Especificação)

## Fonte Única de Verdade (Source of Truth)

O diretório `/doc` é a única fonte de verdade para o comportamento, escopo e arquitetura do sistema.

## Antes de Codificar

1. Leia atentamente as especificações relevantes (`specs.md` e `architecture.md`).
2. Identifique claramente os requisitos funcionais impactados.
3. Identifique os critérios de aceitação associados à tarefa.
4. Verifique e garanta que não existem especificações ou decisões arquiteturais conflitantes.

## Implementação

Implemente estritamente e apenas os requisitos documentados.

Nunca infira ou adivinhe requisitos ausentes.

Sempre que houver ambiguidade ou omissão:
- Interrompa a execução imediatamente;
- Solicite esclarecimentos ao utilizador antes de prosseguir.

## Validação e Verificação

Para cada alteração realizada no código:

- Mapeie explicitamente o código gerado para o requisito correspondente na especificação;
- Verifique se todos os critérios de aceitação aplicáveis foram atendidos;
- Relate imediatamente qualquer desvio ou limitação técnica encontrada.

## Atualização de Documentação

Se a implementação exigir alterações na especificação ou na lógica de arquitetura devido a restrições físicas do hardware, atualize primeiro o respetivo documento no diretório `/doc` antes de alterar o código.

## Proibições Estritas (Forbidden)

- Inclusão de funcionalidades não documentadas (*feature creep*);
- Suposições ou decisões de design que não estejam explicitas nos documentos;
- Alterações arquiteturais que ignorem as decisões registradas nos ADRs;
- Mudanças de comportamento do sistema que desviem dos critérios de aceitação.