---
trigger: always_on
---

# Gerenciamento de Tarefas e Controle de Versão

## Arquivo de Rastreamento Obrigatório
O projeto deve manter um arquivo `TASKS.md` (ou `TODO.md`) na raiz do repositório. Este arquivo atua como o quadro de planejamento central para o agente e para os estudantes que acompanham a evolução do projeto.

## Planejamento e Divisão em Etapas
Antes de escrever qualquer código ou alterar a arquitetura, o agente deve:
1. Analisar o objetivo final com base no `/doc`.
2. Dividir a solução em etapas lógicas, pequenas e gerenciáveis.
3. Registrar essas etapas no `TASKS.md` utilizando listas de verificação do Markdown (ex: `- [ ] Criar a struct de configuração`).

## Ciclo de Execução e Git
A execução das tarefas deve ser estritamente iterativa e rastreável. O agente não deve acumular várias etapas para fazer um único "commit gigante".

Para **cada etapa concluída**, o fluxo obrigatório é:
1. Implementar a alteração no código ou na documentação.
2. Atualizar o `TASKS.md`, marcando a tarefa como concluída (`- [x]`).
3. Executar o gerenciamento do repositório adicionando os arquivos modificados ao *stage* (`git add`).
4. Realizar o commit (`git commit`) registrando aquela etapa isolada.

## Padronização de Commits
Para fins didáticos e organização do histórico, as mensagens de commit devem:
- Ser escritas em Português.
- Seguir o padrão *Conventional Commits* (ex: `feat:`, `fix:`, `docs:`, `refactor:`).
- Descrever claramente "o que" foi alterado e "por que" foi alterado, servindo como registro de estudo para quem analisa o histórico do repositório.

## Proibições
- O agente é estritamente proibido de iniciar a etapa seguinte sem antes realizar o *commit* da etapa atual.
- Não é permitido criar mensagens de commit genéricas como "arquivos atualizados" ou "código modificado".