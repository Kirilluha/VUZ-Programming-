import asyncio

class AsyncClient:
    def __init__(self, host='127.0.0.1', port=12345):
        self.host = host
        self.port = port

    async def tcp_client(self):
        try:
            reader, writer = await asyncio.open_connection(self.host, self.port)
            print(f"Connected to TCP server at {self.host}:{self.port}. Type your message (type 'exit' to quit):")

            while True:
                message = input("Enter message: ")
                if message.lower() == 'exit':
                    print("Closing connection.")
                    writer.close()
                    await writer.wait_closed()
                    break

                if not message.strip():
                    print("Empty message not sent. Please enter a valid message.")
                    continue

                writer.write(message.encode())
                await writer.drain()

                data = await reader.read(1024)
                if not data:
                    print("Connection closed by the server.")
                    break

                print(f"Reply from server: {data.decode()}")

        except ConnectionRefusedError:
            print(f"Cannot connect to server at {self.host}:{self.port}. Is the server running?")
        except Exception as e:
            print(f"An error occurred: {e}")

def main():
    client = AsyncClient(host='127.0.0.1', port=12345)
    try:
        asyncio.run(client.tcp_client())
    except KeyboardInterrupt:
        print("\nClient shutting down.")

if __name__ == "__main__":
    main()
