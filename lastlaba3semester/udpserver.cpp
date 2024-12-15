#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::udp; //Упрощаю доступ к функциям UDP

//Базовый класс сервера
class UDPServer {
public:

    //Создаю конструктор, принимающий io_service и порт, на котором будет работать сервер
    UDPServer(boost::asio::io_service& io_service, unsigned short port)
        : socket_(io_service, udp::endpoint(udp::v4(), port)) { //Создаю UDP-сокет на указанном порту
        start_receive(); //Принятие данных от клиента
    }

private:
    // Метод для асинхронного получения данных от клиента
    void start_receive() {
        // Асинхронное ожидание данных от клиента
        socket_.async_receive_from(
            boost::asio::buffer(buffer_), remote_endpoint_, // Чтение данных в буфер
            [this](const boost::system::error_code& error, std::size_t bytes_transferred) {
                if (!error) {
                    std::string message(buffer_.data(), bytes_transferred); // Преобразуем данные в строку
                    std::cout << "Message received: " << message << std::endl;

                    auto response = std::make_shared<std::string>(message);//Ответ клиенту
                    socket_.async_send_to(boost::asio::buffer(*response), remote_endpoint_,
                        [](const boost::system::error_code&, std::size_t) {});
                        /*Пустая лямбда функция нужна, чтобы закончить async_send_to не выполняя никаких доп. действий, 
                        без нее не завершится обработка в теории сюда можно впихнуть обработку ошибок но в целом пофиг она не нужна*/

                    // Снова начинаем приём данных
                    start_receive();

                } 
                else {
                    std::cerr << "Receive error: " << error.message() << "\n";
                }
            });
    }

    udp::socket socket_; //Это чтобы каждый раз udp не писать
    udp::endpoint remote_endpoint_; //Это тоже
    std::array<char, 1024> buffer_; // Буфер для данных
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
