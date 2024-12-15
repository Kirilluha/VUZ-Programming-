#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::udp;

class UDPServer {
public:
    UDPServer(boost::asio::io_service& io_service, unsigned short port)
        : socket_(io_service, udp::endpoint(udp::v4(), port)) {
        start_receive();
    }

private:
    void start_receive() {
        // Асинхронное ожидание данных от клиента
        socket_.async_receive_from(
            boost::asio::buffer(recv_buffer_), remote_endpoint_,
            [this](const boost::system::error_code& error, std::size_t bytes_transferred) {
                if (!error) {
                    std::string message(recv_buffer_.data(), bytes_transferred);
                    std::cout << "Message received: " << message << std::endl;

                    // Отправка ответа клиенту
                    auto response = std::make_shared<std::string>(message);
                    socket_.async_send_to(boost::asio::buffer(*response), remote_endpoint_,
                        [](const boost::system::error_code&, std::size_t) {});

                    // Снова начинаем приём данных
                    start_receive();
                } else {
                    std::cerr << "Receive error: " << error.message() << "\n";
                }
            });
    }

    udp::socket socket_;
    udp::endpoint remote_endpoint_;
    std::array<char, 1024> recv_buffer_; // Буфер для данных
};

int main() {
    try {
        boost::asio::io_service io_service;
        UDPServer server(io_service, 12345);
        std::cout << "UDP Server is running on port 12345...\n";
        io_service.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
