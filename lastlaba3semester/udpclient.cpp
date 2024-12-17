#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <memory>

using boost::asio::ip::udp; //Упрощаем доступ к функциям UDP

class AsyncUDPClient { //Класс клиента
public:
    //Создаем конструктор, принимающий io_service, хост и порт, по которому подключится клиент
    AsyncUDPClient(boost::asio::io_service& io_service, const std::string& host, const std::string& port)
        : socket_(io_service), //Создаем сокет без привязки к локальному endpoint
          resolver_(io_service)
    {
        udp::resolver::query query(udp::v4(), host, port);
        udp::resolver::iterator endpoints = resolver_.resolve(query);
         /*resolve выпорлняет разрешение доменного имени в IP-адрес, 
        а значит его вообще не обязательно использовать,
        но пусть будет для понимания структуры*/

        boost::asio::connect(socket_, endpoints); //Подключаем сокет к первому найденному endpoint

        std::cout << "Connected to UDP server at " << host << ":" << port << ". Type your message (type 'exit' to quit):\n";
        start_write(); //Запускаем функцию start_write
    }

private:
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

        //Отправка сообщения функцией async_send
        auto send_buffer = std::make_shared<std::string>(message);
        socket_.async_send(
            boost::asio::buffer(*send_buffer),
            boost::bind(&AsyncUDPClient::reply, this, boost::asio::placeholders::error));
            /* Засовываем сообщение в буфер для отправки и отправляем */
    }

    //Функция получения ответа от сервера
    void reply(const boost::system::error_code& error) { //Передаем аргумент ошибки чтобы если что ее вывести
        if (!error) {
            //После отправки сообщения начинаем чтение ответа от сервера:

            /* Переменная для хранения количества байт ответа от сервера используя умный указатель 
               make_shared который не даст массиву очиститься до завершения лямбда функции */
            auto receive_buffer = std::make_shared<std::array<char, 1024>>();

            //Читаем количество байт ответа
            socket_.async_receive(
                boost::asio::buffer(*receive_buffer),
                boost::bind(&AsyncUDPClient::handle_receive, this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred,
                            receive_buffer));
        } else {
            std::cerr << "Error sending message: " << error.message() << "\n";
            socket_.close();
        }
    }

    //Функция обработки приема ответа
    void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred,
                       std::shared_ptr<std::array<char, 1024>> receive_buffer) {
        if (!error) {
            std::string server_reply(receive_buffer->data(), bytes_transferred); //Функцией data переводим буфер в строку
            std::cout << "Reply from server: " << server_reply << "\n";
            start_write(); //После получения ответа снова переходим к отправке сообщения
        } else {
            std::cerr << "Error receiving reply: " << error.message() << "\n";
            socket_.close();
        }
    }

    udp::socket socket_; //Сокет UDP
    udp::resolver resolver_; //Резолвер для разрешения адреса
    std::string message; //Строка с сообщением клиента
};

int main() {
    try {
        boost::asio::io_service io_service; //Переменная io_service для управления асинхронностью
        AsyncUDPClient client(io_service, "127.0.0.1", "12345"); //Подключение к серверу
        io_service.run(); //Обработка асинхронных операций
    } catch (std::exception& e) { //Обработка исключений
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
