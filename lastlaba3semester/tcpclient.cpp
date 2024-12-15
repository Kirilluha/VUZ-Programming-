#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

using boost::asio::ip::tcp;

int main() {
    try {
        boost::asio::io_service io_service;
        tcp::resolver resolver(io_service);
        tcp::resolver::query query("127.0.0.1", "12345");
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        tcp::socket socket(io_service);
        boost::asio::connect(socket, endpoint_iterator);

        std::cout << "Connected to server. Type your message (type 'exit' to quit):\n";

        while (true) {
            // Ввод сообщения с клавиатуры
            std::string message;
            std::cout << "Enter message: ";
            std::getline(std::cin, message);

            // Условие для выхода
            if (message == "exit") {
                std::cout << "Closing connection.\n";
                break;
            }

            // Отправка сообщения на сервер
            boost::asio::write(socket, boost::asio::buffer(message));

            // Буфер для ответа сервера
            char reply[1024];
            boost::system::error_code error;
            size_t reply_length = socket.read_some(boost::asio::buffer(reply), error);

            if (!error) {
                std::cout << "Reply from server: " << std::string(reply, reply_length) << "\n";
            } else {
                std::cerr << "Error receiving reply: " << error.message() << "\n";
                break;
            }
        }
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
