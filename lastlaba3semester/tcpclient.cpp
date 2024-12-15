#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

using boost::asio::ip::tcp; //Упрощаю доступ к функциям TCP

int main() {
    try {
        boost::asio::io_service io_service; //Запуск асинхронных операций
        tcp::resolver resolver(io_service);
        tcp::resolver::query query("127.0.0.1", "12345"); 
        //Указываю имя и порт (в моем случае localhost и порт 12345)
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
            /*resolve выполняет разрешение доменного имени в IP-адрес, 
        а значит его вообще не обязательно использовать,
        но пусть будет*/
        tcp::socket socket(io_service); //Создаю TCP-сокет
        boost::asio::connect(socket, endpoint_iterator); //Подключаемся к серверу

        std::cout << "Connected to server. Type your message (type 'exit' to quit):\n"; //Отладочное сообщение

        while (true) {
            // Ввод сообщения с клавиатуры
            std::string message;
            std::cout << "Enter message: ";
            std::getline(std::cin, message);

            // Условие для закрытия клиента
            if (message == "exit") {
                std::cout << "Closing connection.\n";
                break;
            }

            //Отправка сообщения на сервер
            boost::asio::write(socket, boost::asio::buffer(message));

            //Буфер для ответа сервера
            char reply[1024];
            boost::system::error_code error; //Инициализирую error

            size_t reply_length = socket.read_some(boost::asio::buffer(reply), error); 
            /*Читаю кол-во байтов ответа сервера и ошибки если она возникла*/

            if (!error) { //Нет ошибки - выведется ответ
                std::cout << "Reply from server: " << std::string(reply, reply_length) << "\n";
            } 
            else { //Есть ошибка - выведется что за ошибка и цикл завершится и все переподключится
                std::cerr << "Error receiving reply: " << error.message() << "\n";
                break;
            }
        }
    } 
    catch (std::exception& e) { //Обработка исключений
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
