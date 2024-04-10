import socket
import threading
import sys

# Connection details
server_address = '127.0.0.1'
server_port = 3000

# Global variable to hold the message to be sent
message_to_send = ""
has_message = threading.Event()

def listen_for_messages(s):
    """Listens for messages from the server and prints them."""
    try:
        while True:
            data = s.recv(1024)
            if data:
                print("\r" + data.decode('utf-8'), end='\n> ')
                sys.stdout.flush()
            else:
                break
    except Exception as e:
        print(f"Error occurred while listening for messages: {e}")

def send_message(s):
    """Sends messages to the server when they are available."""
    global message_to_send
    while True:
        has_message.wait()  # Block until there is a message to send
        try:
            s.sendall(message_to_send.encode('utf-8'))
            has_message.clear()  # Reset the event
        except Exception as e:
            print(f"Error sending message: {e}")
            break

def user_input_handler():
    """Handles user input, setting the global message variable."""
    global message_to_send
    while True:
        message_to_send = input("> ")
        has_message.set()

# Main function to setup the client
def main():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        try:
            s.connect((server_address, server_port))
            print("Connected to the server.")

            # Send username to the server
            username = input("Enter your username: ")
            s.sendall(username.encode('utf-8'))

            # Start the listening and sending threads
            listen_thread = threading.Thread(target=listen_for_messages, args=(s,))
            send_thread = threading.Thread(target=send_message, args=(s,))
            listen_thread.start()
            send_thread.start()

            # Handle user input in the main thread
            user_input_handler()

            # Wait for threads to finish (not really happening here without additional signaling)
            listen_thread.join()
            send_thread.join()

        except Exception as e:
            print(f"Error: {e}")

if __name__ == "__main__":
    main()