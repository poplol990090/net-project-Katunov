# Зачётный проект по сетевому программированию

## Студент

| | |
|---|---|
| ФИО | <Катунов Максим Сергеевич> |
| Группа | <РПО 24/1> |
| Вариант | <9. Словарь-переводчик> |
| Выполненный уровень | <3> |

## Описание

<2–3 предложения: что делает ваш сервер и как с ним работать через клиент.>

## Как собрать и запустить

Запустить два проекта (Первый - где сервер, второй - где клиент)
(В одном не мог, была ошибка)
Запустить в первую очередь сервер, во вторую очередь клиент.

## Протокол

Сервер слушает порт **<9000>**. Каждая команда — одна строка, каждый ответ — одна строка. После подключения сервер присылает приветствие.

| Команда | Что делает | Пример ответа |
|---|---|---|
| `<TRANSLATE <eng>>` | <перевод слова с английского на русский> | `cat -> <koshka>` |
| `<WORDS>` | <выводит список слов> | `<apple=yabloko; book=kniga; car=mashina; cat=koshka; city=gorod; dog=sobaka; friend=drug; house=dom; moon=luna; school=shkola; sun=solntse; table=stol; tree=derevo; water=voda; window=okno>` |
| `<ADD <eng> <ru>>` | <добавляет новое слово в список> | `<OK>` |
| `<DEL <eng>>` | <удаляет выбранное слово из списка> | `<OK>` |
| `<COUNT>` | <считает, сколько слов в списке> | `<15>` |
| `<TEST>` | <выводит тест для клиента, он должен ответить на 5 вопросов, переводя слова, в конце выводит сколько правильно он ответил> | `<Test finished: right 4, wrong 1>` |
| `<ANSWER <ru>>` | <ответ слова на русском для теста> | `<RIGHT. TEST ?/5: ...>` |
| `HELP` | список команд | `<Commands: TRANSLATE <eng> | WORDS | ADD <eng> <ru> | DEL <eng> | COUNT | TEST | ANSWER <ru> | HELP | QUIT>` |
| `QUIT` | завершить работу | `BYE` |

Ошибки: <какие ответы присылает сервер на неправильную команду или аргументы, например `ERROR ...`>.
ERROR unknown command: ...

## Пример работы

Server: Dictionary server ready. Commands: TRANSLATE <eng> | WORDS | ADD <eng> <ru> | DEL <eng> | COUNT | TEST | ANSWER <ru> | HELP | QUIT
> words
apple=yabloko; book=kniga; car=mashina; cat=koshka; city=gorod; dog=sobaka; friend=drug; house=dom; moon=luna; school=shkola; sun=solntse; table=stol; tree=derevo; water=voda; window=okno
> count
15
> translate ccat
ERROR unknown word
> translate car
mashina
> add snowy sneg
OK

```
Server is listening on port 9000
Client connected: 127.0.0.1:64359
  <- words
  -> apple=yabloko; book=kniga; car=mashina; cat=koshka; city=gorod; dog=sobaka; friend=drug; house=dom; moon=luna; school=shkola; sun=solntse; table=stol; tree=derevo; water=voda; window=okno
  <- count
  -> 15
  <- translate ccat
  -> ERROR unknown word
  <- translate car
  -> mashina
  <- add snowy sneg
  -> OK

```

## Скриншоты

Положите картинки в папку `screenshots` и вставьте их сюда:

1. Сервер и клиент работают одновременно (два окна).
2. Вывод `netstat -an | findstr 9000` во время работы клиента (видны `LISTENING` и `ESTABLISHED`).

![Работа программы](screenshots/work.png)
![netstat](screenshots/netstat.png)

## Что я изменил в шаблоне

<Кратко: какие функции и переменные добавили, какие классы (для уровня 3), что было сложнее всего.>

## Ответы на контрольные вопросы
    1. Порядок вызовов:

    socket() — создает сокет (дескриптор для сети).

    bind() — привязывает сокет к порту 9000.

    listen() — переводит в режим ожидания клиентов.

    accept() — ждет подключения и создает новый сокет для общения с конкретным клиентом.

    2. Почему клиенту не нужны bind и listen?
    Клиент сам инициирует связь (connect). bind не нужен, так как ОС сама автоматически выдает клиенту случайный свободный порт (динамический) в момент подключения.

    3. Зачем htons(PORT)?
    Процессоры хранят числа в одном порядке (Little-Endian), а интернет-протоколы — в другом (Big-Endian). htons переворачивает байты порта, чтобы сервер и клиент поняли друг друга.

    4. Почему recvLine читает до \n?
    TCP передает данные как поток байтов. За один вызов recv можно получить половину команды или сразу две. Чтение до \n гарантирует, что мы прочитаем ровно одну команду целиком.

    5. Протокол варианта 9 (Словарь):
    Сервер шлет приветствие. Клиент шлет команду, сервер отвечает одной строкой.

    TRANSLATE <слово> → перевод (или ERROR unknown word).

    ADD <eng> <ru> / DEL <eng> → OK (сохраняет в файл dict.txt).

    WORDS / COUNT / HELP → список слов / число / справка.

    TEST / ANSWER <слово> → вопросы викторины (RIGHT/WRONG) и счет.

    QUIT → BYE и закрытие.

    При ошибке (неизвестная команда или неверные аргументы): сервер отвечает строкой, начинающейся с ERROR ..., и не падает.
