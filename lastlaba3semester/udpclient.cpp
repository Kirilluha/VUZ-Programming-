#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::udp;

int main() {
    try {
        boost::asio::io_service io_service;

        udp::resolver resolver(io_service);
        udp::resolver::query query(udp::v4(), "127.0.0.1", "12345");
        udp::endpoint server_endpoint = *resolver.resolve(query);

        udp::socket socket(io_service);
        socket.open(udp::v4());

        std::cout << "Connected to UDP server. Type your message (type 'exit' to quit):\n";

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

            // Отправка сообщения серверу
            socket.send_to(boost::asio::buffer(message), server_endpoint);

            // Буфер для ответа сервера
            std::array<char, 1024> recv_buffer;
            udp::endpoint sender_endpoint;
            boost::system::error_code error;

            // Получение ответа от сервера
            size_t reply_length = socket.receive_from(boost::asio::buffer(recv_buffer), sender_endpoint, 0, error);

            if (!error) {
                std::cout << "Reply from server: " << std::string(recv_buffer.data(), reply_length) << "\n";
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
