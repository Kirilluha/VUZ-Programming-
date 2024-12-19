import asyncio

class TCPServer:
    def __init__(self, host='0.0.0.0', port=12345):
        self.host = host
        self.port = port

    async def handle_client(self, reader, writer):
        addr = writer.get_extra_info('peername')
        print(f"Client connected from {addr}")

        try:
            while True:
                # Чтение данных от клиента
                data = await reader.read(1024)
                if not data:
                    print(f"Client {addr} disconnected.")
                    break

                message = data.decode()
                print(f"Message received from {addr}: {message}")

                # Отправка ответа клиенту
                writer.write(data)
                await writer.drain()
        except asyncio.CancelledError:
            pass
        except Exception as e:
            print(f"Error with client {addr}: {e}")
        finally:
            writer.close()
            await writer.wait_closed()

    async def start_server(self):
        server = await asyncio.start_server(
            self.handle_client, self.host, self.port
        )
        addr = server.sockets[0].getsockname()
        print(f"Server started on {addr}")

        async with server:
            await server.serve_forever()

def main():
    server = TCPServer(port=12345)
    try:
        asyncio.run(server.start_server())
    except KeyboardInterrupt:
        print("\nServer shutting down.")

if __name__ == "__main__":
    main()
