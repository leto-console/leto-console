---
name: leto-board
description: Двигать статус задачи на доске LETO Console (Project V2) через GraphQL. Использовать при взятии задачи в работу, открытии PR и закрытии issue, а также когда gh project отвечает ошибкой scope.
---

# Доска LETO Console

Проект — `LETO Console` (номер 1), поле `Status`, опции `Todo` / `In progress` /
`In review` / `Done`.

Переходы: `In progress` — при взятии задачи, `In review` — при открытии PR,
`Done` — при закрытии issue.

## Почему не `gh project`

С `GH_PROJECTS_TOKEN` ему не хватает scope `read:org`. Работает мутация
`updateProjectV2ItemFieldValue`.

## Алгоритм

Id доски, поля и опций **не хранить** ни в документации, ни в заметках: они привязаны
к объекту в конкретной доске и меняются при её пересоздании; устаревший id проявится
только при вызове (`Could not resolve to a node`). Искать по именам каждый раз.

Цепочка: issue → `projectItems` (взять item с `project.number` = 1) →
мутация на `projectId`, `itemId`, `fieldId`, `value.singleSelectOptionId`.

```sh
gh api graphql -f query='
query($owner:String!,$repo:String!,$num:Int!){
  repository(owner:$owner,name:$repo){ issue(number:$num){
    projectItems(first:10){ nodes{ id project{ number } } } } } }' \
  -F owner=leto-console -F repo=leto-console -F num=<N>
```

## Грабли

- Имя статуса писать как на доске: `In review`, а не `In Review` — опция ищется
  точным совпадением.
- `gh api --jq` не принимает `--arg`: подставлять имя статуса пайпом на
  `jq -r --arg name "$status" ...`.
- `-f`, а не `-F` для строковых id: `-F` приводит числовой id опции к числу,
  и GraphQL отвечает `Could not coerce value ... to String`.

## Отчёт

Одна строка: задача, прежний статус → новый, подтверждение из ответа мутации.
