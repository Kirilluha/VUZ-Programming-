#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::udp; //Упрощаю доступ к функциям UDP

int main() {
    try {
        boost::asio::io_service io_service; //Для асинхронных операций

        udp::resolver resolver(io_service); //Инициализация resolver
        udp::resolver::query query(udp::v4(), "127.0.0.1", "12345");
        udp::endpoint server_endpoint = *resolver.resolve(query);
        /*resolve выпорлняет разрешение доменного имени в IP-адрес, 
        а значит его вообще не обязательно использовать,
        но пусть будет*/

        udp::socket socket(io_service); //Создание сокета для UDP
        socket.open(udp::v4());  //Открытие сокета на IPv4
 

        std::cout << "Connected to UDP server. Type your message (type 'exit' to quit):\n"; //Прост месседж

        while (true) {
            //Ввод сообщения с клавиатуры
            std::string message;
            std::cout << "Enter message: ";
            std::getline(std::cin, message);

            //Условие для закрытия клиента
            if (message == "exit") {
                std::cout << "Closing connection.\n";
                break;
            }

            // Отправка сообщения серверу
            socket.send_to(boost::asio::buffer(message), server_endpoint);

            // Буфер для ответа сервера
            std::array<char, 1024> buffer_;

            udp::endpoint sender_endpoint; //Эт для удобства
            boost::system::error_code error;

            // Получение ответа от сервера (кол-во байт)
            size_t reply_length = socket.receive_from(boost::asio::buffer(buffer_), sender_endpoint, 0, error);

            if (!error) { //Нет ошибки - выводим ответ
                std::cout << "Reply from server: " << std::string(buffer_.data(), reply_length) << "\n";
            } else {
                std::cerr << "Error receiving reply: " << error.message() << "\n";
                break;
            }
        }
    } catch (std::exception& e) { // Исключения
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
