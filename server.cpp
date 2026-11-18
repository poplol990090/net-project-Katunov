// =====================================================================
//                              ВАРИАНТ 9
// =====================================================================
#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#define SEND_FLAGS 0
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR   (-1)
#define closesocket close
#define SEND_FLAGS MSG_NOSIGNAL
#endif

#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <cctype>

using namespace std;

namespace ServerApp {

    const int PORT = 9000;

    static string toUpperStr(string s) { // делает все буквы заглавными: "HeLp" -> "HELP"
        for (char& c : s) c = (char)toupper((unsigned char)c); 
        return s;
    }
    static string toLowerStr(string s) { // делает все буквы маленькими: "Cat" -> "cat"
        for (char& c : s) c = (char)tolower((unsigned char)c);
        return s;
    }
    static string trimStr(const string& s) { // удаляет лишние пробелы, табуляции (\t) и переносы строк (\r\n) в начале и в конце текста
        size_t a = s.find_first_not_of(" \t\r\n");
        if (a == string::npos) return "";
        size_t b = s.find_last_not_of(" \t\r\n");
        return s.substr(a, b - a + 1);
    }

    class Dictionary {
    private:
        map<string, string> words;
        bool   testActive = false;
        string testWord;
        int    testRight = 0;
        int    testWrong = 0;
        int    testAsked = 0;
        static const int TEST_ROUNDS = 5;

    public:
        Dictionary() { // заранее созданный словарь
            words["cat"] = "koshka";
            words["dog"] = "sobaka";
            words["house"] = "dom";
            words["book"] = "kniga";
            words["water"] = "voda";
            words["sun"] = "solntse";
            words["moon"] = "luna";
            words["friend"] = "drug";
            words["table"] = "stol";
            words["window"] = "okno";
            words["car"] = "mashina";
            words["tree"] = "derevo";
            words["city"] = "gorod";
            words["school"] = "shkola";
            words["apple"] = "yabloko";
            loadFromFile();
        }

        void loadFromFile() { // при запуске читает файл и загружает из него слова в память сервера.
            ifstream in("dict.txt");
            if (!in) return;
            string en, ru;
            while (in >> en >> ru) words[toLowerStr(en)] = ru;
        }
        void saveToFile() const { // сохраняет все текущие слова из памяти обратно в файл
            ofstream out("dict.txt");
            if (!out) return;
            for (auto& kv : words) out << kv.first << " " << kv.second << "\n";
        }

        string translate(const string& eng) const { // переводит английское слово, ищет его в словаре, если нет, то выдает ошибку
            string key = toLowerStr(trimStr(eng));
            if (key.empty()) return "ERROR empty word";
            auto it = words.find(key);
            if (it == words.end()) return "ERROR unknown word";
            return it->second;
        }

        string listWords() const { // выводит все слова через точку с запятой. команда 'WORDS'
            if (words.empty()) return "Dictionary is empty";
            string res;
            bool first = true;
            for (auto& kv : words) {
                if (!first) res += "; ";
                res += kv.first + "=" + kv.second;
                first = false;
            }
            return res;
        }

        string addWord(const string& eng, const string& ru) { // добавляет новое слово в словарь и сразу сохраняет в файл
            string k = toLowerStr(trimStr(eng));
            string v = trimStr(ru);
            if (k.empty() || v.empty()) return "ERROR usage: ADD <eng> <ru>";
            words[k] = v;
            saveToFile();
            return "OK";
        }

        string delWord(const string& eng) { // удаляет слово из словаря и сохраняет изменения в файл
            string k = toLowerStr(trimStr(eng));
            if (k.empty()) return "ERROR usage: DEL <word>";
            auto it = words.find(k);
            if (it == words.end()) return "ERROR unknown word";
            words.erase(it);
            saveToFile();
            return "OK";
        }

        string countWords() const { // просто возвращает количество слов в словаре
            return to_string(words.size());
        }

        string startTest() { // запускает режим викторины: сбрасывает старый счет и задает первый вопрос
            if (words.empty()) return "ERROR dictionary is empty";
            testActive = true;
            testRight = testWrong = testAsked = 0;
            return nextQuestion();
        }

        string nextQuestion() { // выбирает случайное слово из словаря и формирует строку вопроса 
            int idx = rand() % (int)words.size();
            auto it = words.begin();
            advance(it, idx);
            testWord = it->first;
            testAsked++;
            return "TEST " + to_string(testAsked) + "/" + to_string(TEST_ROUNDS) +
                ": translate '" + testWord + "'";
        }

        string answer(const string& ans) { // проверяет ответ игрока
            if (!testActive) return "ERROR no active test, use TEST";
            string user = toLowerStr(trimStr(ans));
            if (user.empty()) return "ERROR empty answer";

            string expectedRu = toLowerStr(words[testWord]);
            bool ok = (user == expectedRu) || (user == testWord);

            string reply = ok ? "RIGHT" : ("WRONG, correct: " + words[testWord]);
            if (ok) testRight++; else testWrong++;

            if (testAsked >= TEST_ROUNDS) {
                testActive = false;
                reply += ". Test finished: right " + to_string(testRight) +
                    ", wrong " + to_string(testWrong);
            }
            else {
                reply += ". " + nextQuestion();
            }
            return reply;
        }

        string help() const { // возвращает строку со списком всех доступных команд
            return "Commands: TRANSLATE <eng> | WORDS | ADD <eng> <ru> | DEL <eng> | "
                "COUNT | TEST | ANSWER <ru> | HELP | QUIT";
        }
    };

    static Dictionary g_dict;

    string onConnect() {
        return "Dictionary server ready. " + g_dict.help();
    }

    string handle(const string& command) {
        string trimmed = trimStr(command);
        if (trimmed.empty()) return "ERROR empty command";

        if (toUpperStr(trimmed).rfind("ANSWER", 0) == 0) {
            string arg = trimStr(trimmed.substr(6));
            return g_dict.answer(arg);
        }

        istringstream in(command);
        string name;
        in >> name;
        name = toUpperStr(name);

        if (name == "HELP") return g_dict.help();

        if (name == "TRANSLATE") {
            string eng, extra;
            if (!(in >> eng)) return "ERROR usage: TRANSLATE <word>";
            if (in >> extra)  return "ERROR too many arguments";
            return g_dict.translate(eng);
        }
        if (name == "WORDS") {
            string extra;
            if (in >> extra) return "ERROR too many arguments";
            return g_dict.listWords();
        }
        if (name == "ADD") {
            string eng, ru, extra;
            if (!(in >> eng >> ru)) return "ERROR usage: ADD <eng> <ru>";
            if (in >> extra)        return "ERROR too many arguments";
            return g_dict.addWord(eng, ru);
        }
        if (name == "DEL") {
            string eng, extra;
            if (!(in >> eng)) return "ERROR usage: DEL <word>";
            if (in >> extra)  return "ERROR too many arguments";
            return g_dict.delWord(eng);
        }
        if (name == "COUNT") {
            string extra;
            if (in >> extra) return "ERROR too many arguments";
            return g_dict.countWords();
        }
        if (name == "TEST") {
            string extra;
            if (in >> extra) return "ERROR too many arguments";
            return g_dict.startTest();
        }

        return "ERROR unknown command: " + name;
    }

    bool sendLine(SOCKET s, const string& text)
    {
        string data = text + "\n";
        size_t sent = 0;
        while (sent < data.size()) {
            int n = send(s, data.c_str() + sent, (int)(data.size() - sent), SEND_FLAGS);
            if (n <= 0) return false;
            sent += n;
        }
        return true;
    }

    bool recvLine(SOCKET s, string& line)
    {
        line.clear();
        char c;
        while (true) {
            int n = recv(s, &c, 1, 0);
            if (n <= 0) return false;
            if (c == '\n') return true;
            if (c != '\r') line += c;
        }
    }

    void runServer()
    {
#ifdef _WIN32
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            cout << "WSAStartup failed" << endl;
            return;
        }
        SetConsoleOutputCP(CP_UTF8);
#endif
        srand((unsigned)time(nullptr));

        SOCKET listenSock = socket(AF_INET, SOCK_STREAM, 0);
        if (listenSock == INVALID_SOCKET) {
            cout << "socket() error" << endl;
            return;
        }

#ifndef _WIN32
        int yes = 1;
        setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));
#endif

        sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(PORT);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(listenSock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            cout << "bind() error: port " << PORT << " is busy?" << endl;
            closesocket(listenSock);
            return;
        }

        if (listen(listenSock, 5) == SOCKET_ERROR) {
            cout << "listen() error" << endl;
            closesocket(listenSock);
            return;
        }
        cout << "Server is listening on port " << PORT << endl;

        while (true) {
            sockaddr_in clientAddr = {};
            socklen_t clientLen = sizeof(clientAddr);
            SOCKET client = accept(listenSock, (sockaddr*)&clientAddr, &clientLen);
            if (client == INVALID_SOCKET) {
                cout << "accept() error" << endl;
                continue;
            }

            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, ip, sizeof(ip));
            int clientPort = ntohs(clientAddr.sin_port);
            cout << "Client connected: " << ip << ":" << clientPort << endl;

            sendLine(client, onConnect());

            string command;
            while (recvLine(client, command)) {
                cout << "  <- " << command << endl;

                if (toUpperStr(trimStr(command)) == "QUIT") {
                    sendLine(client, "BYE");
                    break;
                }

                string answer = handle(command);
                cout << "  -> " << answer << endl;
                if (!sendLine(client, answer)) break;
            }

            closesocket(client);
            cout << "Client disconnected: " << ip << ":" << clientPort << endl;
        }

        closesocket(listenSock);
#ifdef _WIN32
        WSACleanup();
#endif
    }

} // конец namespace ServerApp

int main() {
    ServerApp::runServer();
    return 0;
}