import asyncio

class AsyncUDPClientProtocol:
    def __init__(self, message, on_con_lost, server_address):
        self.message = message
        self.on_con_lost = on_con_lost
        self.server_address = server_address
        self.transport = None

    def connection_made(self, transport):
        self.transport = transport
        print(f"Sending message to {self.server_address}: {self.message}")
        self.transport.sendto(self.message.encode())

    def datagram_received(self, data, addr):
        print(f"Reply from server: {data.decode()}")
        self.on_con_lost.set_result(True)

    def error_received(self, exc):
        print(f"Error received: {exc}")

    def connection_lost(self, exc):
        asyncio.get_event_loop().stop()

async def send_message(host='127.0.0.1', port=12345):
    loop = asyncio.get_running_loop()

    while True:
        message = input("Enter message (type 'exit' to quit): ")
        if message.lower() == 'exit':
            print("Closing client.")
            break

        if not message.strip():
            print("Empty message not sent. Please enter a valid message.")
            continue

        on_con_lost = asyncio.get_event_loop().create_future()

        transport, protocol = await loop.create_datagram_endpoint(
            lambda: AsyncUDPClientProtocol(message, on_con_lost, (host, port)),
            remote_addr=(host, port)
        )

        try:
            await on_con_lost
        finally:
            transport.close()

if __name__ == "__main__":
    try:
        asyncio.run(send_message())
    except KeyboardInterrupt:
        print("\nClient shutting down.")
