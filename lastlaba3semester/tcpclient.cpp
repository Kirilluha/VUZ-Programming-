#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <memory>

using boost::asio::ip::tcp; //Упрощаем доступ к функциям TCP

class AsyncClient { //Класс клиента
public:
    //Создаем конструктор, принимающий io_service, хост и порт, по которому подключится клиент
    AsyncClient(boost::asio::io_service& io_service, const std::string& host, const std::string& port) 
        : socket_(io_service), 
        resolver_(io_service) { 

        auto endpoints = resolver_.resolve(host, port);
        /*resolve выпорлняет разрешение доменного имени в IP-адрес, 
        а значит его вообще не обязательно использовать,
        но пусть будет для понимания структуры*/
        boost::asio::async_connect(socket_, endpoints,
            boost::bind(&AsyncClient::connect, this, boost::asio::placeholders::error, host, port)); //Начинаем подключение и после подключения стартуем connect

    }

private:
    //Фунция для проверки, успешное ли подключние, если да, то стартуем метод start_write
    void connect(const boost::system::error_code& error, std::string host, std::string port) { //Передаем аргумент ошибки чтобы если что ее вывести
        if (!error) {
            std::cout << "Connected to TCP server at " << host << ":" << port << ". Type your message (type 'exit' to quit):\n";
            start_write();
        } else {
            std::cerr << "Connection failed: " << error.message() << "\n";
        }
    }
    //Функция записи сообщения в консоли
    void start_write() {
        std::cout << "Enter message: ";
        std::getline(std::cin, message); //Считываем сообщение с консоли

        if (message == "exit") { //Если пользователь вводит "exit", завершаем соединение
            std::cout << "Closing connection.\n";
            socket_.close();
            return;
        }

        if (message.empty()) { //Не отправляем пустые сообщения иначе сервер зависает
        
            std::cout << "Empty message not sent. Please enter a valid message.\n";
            start_write(); //Запросить ввод снова
            return;
        }

        //Отправка сообщения функцией acync_write
        boost::asio::async_write(socket_, boost::asio::buffer(message),
            boost::bind(&AsyncClient::reply, this, boost::asio::placeholders::error)); 
        /* Засовываем сообщение в буфер для отправки и отправляем */

    }

    //Функция получения ответа от сервера
    void reply(const boost::system::error_code& error) { //Передаем аргумент ошибки чтобы если что ее вывести
        if (!error) {
            //После отправки сообщения начинаем чтение ответа от сервера:

            /*Переменная для хранения количества байт ответа от сервера используя умный указатель 
            make_shared который не даст массиву очиститься до заверщения лямбда функции*/
            auto buffer = std::make_shared<std::array<char, 1024>>(); 

            //Читаем количество байт ответа
            socket_.async_read_some(boost::asio::buffer(*buffer),
                [this, buffer](const boost::system::error_code& error, std::size_t bytes_transferred) {
                    if (!error) {
                        std::cout << "Reply from server: "
                                  << std::string(buffer->data(), bytes_transferred) << "\n"; //Функцией data переводим буфер в строку
                        start_write(); //После получения ответа снова переходим к отправке сообщения
                    } else {
                        std::cerr << "Error reading reply: " << error.message() << "\n";
                        socket_.close();
                    }
                });
        } else {
            std::cerr << "Error sending message: " << error.message() << "\n";
            socket_.close();
        }
    }

    tcp::socket socket_; //Создаем переменную типа сокет
    tcp::resolver resolver_; //Создаем переменную типа ресолвер
    std::string message; //Строка с сообщением клиента
};

int main() {
    try {
        boost::asio::io_service io_service; //Переменная io_service для управления асинхронностью
        AsyncClient client(io_service, "127.0.0.1", "12345"); //Подключение к серверу
        io_service.run(); //Обработка асинхронных операций
    } catch (std::exception& e) { //Обработка исключений
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
