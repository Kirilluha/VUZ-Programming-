import asyncio

class UDPServerProtocol:
    def __init__(self):
        self.transport = None

    def connection_made(self, transport):
        self.transport = transport
        sock = transport.get_extra_info('socket')
        addr = transport.get_extra_info('sockname')
        print(f"UDP server listening on {addr}")

    def datagram_received(self, data, addr):
        message = data.decode()
        print(f"Message received from {addr}: {message}")

        if message:
            # Отправка ответа клиенту (эхо)
            self.transport.sendto(data, addr)
            print(f"Echoed back to {addr}")
        else:
            print(f"Received empty message from {addr}. Ignoring.")

    def error_received(self, exc):
        print(f"Error received: {exc}")

    def connection_lost(self, exc):
        print("Socket closed, stop the event loop")
        asyncio.get_event_loop().stop()

async def main(host='0.0.0.0', port=12345):
    print("Starting UDP server...")
    loop = asyncio.get_running_loop()

    # Создание и запуск UDP сервера
    transport, protocol = await loop.create_datagram_endpoint(
        lambda: UDPServerProtocol(),
        local_addr=(host, port)
    )

    try:
        # Сервер работает до прерывания (например, Ctrl+C)
        await asyncio.sleep(float('inf'))
    except KeyboardInterrupt:
        print("\nServer is shutting down...")
    finally:
        transport.close()

if __name__ == "__main__":
    asyncio.run(main())
