# Зачётный проект по сетевому программированию

## Студент

| | |
|---|---|
| ФИО | Катунов Максим Сергеевич |
| Группа | РПО 24/1 |
| Вариант | 9. Словарь-переводчик |
| Выполненный уровень | 3 |

## Описание

Сервер на порту 9000 принимает команды от клиента и обрабатывает их с помощью Dictionary, который хранит словарь и логику викторины. Программа поддерживает перевод слов, добавление и удаление, а также режим тренировки TEST, сохраняя изменение в файл dict.txt. Все ошибки ввода обрабатываются ответом ERROR, что не позволяет серверу аварийно завершиться

## Как собрать и запустить

Запустить два проекта (Первый - где сервер, второй - где клиент)
(В одном не мог, была ошибка)
Запустить в первую очередь сервер, во вторую очередь клиент.

## Протокол

Сервер слушает порт **<9000>**. Каждая команда — одна строка, каждый ответ — одна строка. После подключения сервер присылает приветствие.

| Команда | Что делает | Пример ответа |
|---|---|---|
| `TRANSLATE <eng>` | <перевод слова с английского на русский> | `cat -> <koshka>` |
| `WORDS` | <выводит список слов> | `<apple=yabloko; book=kniga; car=mashina; cat=koshka; city=gorod; dog=sobaka; friend=drug; house=dom; moon=luna; school=shkola; sun=solntse; table=stol; tree=derevo; water=voda; window=okno>` |
| `ADD <eng> <ru>` | <добавляет новое слово в список> | `OK` |
| `DEL <eng>` | <удаляет выбранное слово из списка> | `OK` |
| `COUNT` | <считает, сколько слов в списке> | `15` |
| `TEST` | <выводит тест для клиента, он должен ответить на 5 вопросов, переводя слова, в конце выводит сколько правильно он ответил> | `<Test finished: right 4, wrong 1>` |
| `ANSWER <ru>` | <ответ слова на русском для теста> | `RIGHT. TEST ?/5: ...` |
| `HELP` | список команд | `<Commands: TRANSLATE <eng> | WORDS | ADD <eng> <ru> | DEL <eng> | COUNT | TEST | ANSWER <ru> | HELP | QUIT>` |
| `QUIT` | завершить работу | `BYE` |

Ошибки:
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

![Работа программы](screenshots/work.png)
![netstat](screenshots/netstat.png)

## Что я изменил в шаблоне

1. Добавлены #include <fstream> и <cctype>
2. Написаны вспомогательные функции: toUpperStr, toLowerStr, trimStr
3. Логика вынесена в класс Dictionary
4. onConnect() перечисляет команды
   handle() разбирает ввод и выдает ERROR . . . на неизвестные команды
5. В main() проверка QUIT сделана через toUpperStr, чтобы работало в любом регистре
6. Сетевая часть (sendLine, recLine, сокеты) оставлена без изменений

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

    TRANSLATE <слово> -> перевод (или ERROR unknown word).

    ADD <eng> <ru> / DEL <eng> -> OK (сохраняет в файл dict.txt).

    WORDS / COUNT / HELP -> список слов / число / справка.

    TEST / ANSWER <слово> -> вопросы викторины (RIGHT/WRONG) и счет.

    QUIT -> BYE и закрытие.

    При ошибке (неизвестная команда или неверные аргументы): сервер отвечает строкой, начинающейся с ERROR ..., и не падает.
