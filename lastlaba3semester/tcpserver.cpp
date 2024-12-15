#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

using boost::asio::ip::tcp;

class TCPServer {
public:
    TCPServer(boost::asio::io_service& io_service, unsigned short port)
        : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
          socket_(io_service) {
        start_accept();
    }

private:
    void start_accept() {
        acceptor_.async_accept(socket_,
            boost::bind(&TCPServer::handle_accept, this, boost::asio::placeholders::error));
    }

    void handle_accept(const boost::system::error_code& error) {
        if (!error) {
            std::cout << "Client connected.\n";
            start_read(); // Начать чтение данных от клиента
        }
        start_accept(); // Ожидание новых подключений
    }

    void start_read() {
        // Буфер для хранения данных
        auto buffer = std::make_shared<std::array<char, 1024>>();

        // Асинхронное чтение данных
        socket_.async_read_some(boost::asio::buffer(*buffer),
            [this, buffer](const boost::system::error_code& error, std::size_t bytes_transferred) {
                if (!error) {
                    // Сохранение и вывод полученного сообщения
                    std::string message_received(buffer->data(), bytes_transferred);
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

    tcp::acceptor acceptor_;
    tcp::socket socket_;
};

int main() {
    try {
        boost::asio::io_service io_service;
        TCPServer server(io_service, 12345);
        io_service.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
