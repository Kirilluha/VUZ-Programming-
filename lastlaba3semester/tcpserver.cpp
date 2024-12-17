#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

using boost::asio::ip::tcp;     //Упрощаем доступ к функциям TCP

//Базовый класс сервера
class TCPServer {
public:
    //Создаем конструктор, принимающий io_service и порт, на котором будет работать сервер
    TCPServer(boost::asio::io_service& io_service, unsigned short port)
        : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),     //Acceptor для прослушивания
          socket_(io_service) {     // Сокет для общения с клиентом
        start_accept();     // Ожидание подключений
    }

private:
    //Ожидание новых подключений
    void start_accept() {
        acceptor_.async_accept(socket_,     //Асинхронно ждем подключения
            boost::bind(&TCPServer::handle_accept, 
            this, boost::asio::placeholders::error)); //Обработчик при успешном подключении
    }
    //Обработчик подключения
    void handle_accept(const boost::system::error_code& error) {
        if (!error) {
            std::cout << "Client connected.\n";
            start_read(); // Начать чтение данных от клиента
        }
        start_accept(); // Ожидание новых подключений
    }
    //Начинаем читать сообщения
    void start_read() {
        /*Переменная для хранения количества байт ответа от сервера используя умный указатель 
            make_shared который не даст массиву очиститься до заверщения лямбда функции*/
        auto buffer = std::make_shared<std::array<char, 1024>>();

        //Читаем количество байт ответа
        socket_.async_read_some(boost::asio::buffer(*buffer),
            [this, buffer](const boost::system::error_code& error, std::size_t bytes_transferred) {
                if (!error) {
                    //Сохранение и вывод полученного сообщения
                    std::string message_received(buffer->data(), bytes_transferred); //Конвертируем данные в строку
                    std::cout << "Message received: " << message_received << std::endl;

                    // Ответ клиенту
                    auto response = std::make_shared<std::string>(message_received);
                    boost::asio::async_write(socket_, boost::asio::buffer(*response),
                        [this](const boost::system::error_code&, std::size_t) {
                            // После отправки ответа снова читаем данные
                            start_read();
                        });
                } else {
                    std::cerr << "Error: " << error.message() << "\n";
                }
            });
    }

    tcp::acceptor acceptor_; //Создаем переменнуем типа acceptor
    tcp::socket socket_; //Создаем переменнуем типа socket
};

int main() {
    try {
        boost::asio::io_service io_service;                   //Объект для управления асинхронными операциями
        TCPServer server(io_service, 12345);                  //Создаем сервер на порту 12345
        io_service.run();                                     //Обработка асинхронных операций
    } catch (std::exception& e) {                             //Обработка исключений
        std::cerr << "Exception: " << e.what() << "\n";
    }
}
