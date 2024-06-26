import socket


def send_udp_command(ip, port, message):
    """Sends a single UDP packet to the given IP address and port."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)  # Create a UDP socket
    sock.sendto(message.encode(), (ip, port))  # Send the message
    sock.close()


def main():
    # IP and port of the ESP32 UDP server
    ESP_IP = "tank-t34-esp32.local"  # Replace with your ESP32's IP address
    ESP_PORT = 4210  # Replace with the UDP port your ESP32 is listening on

    print(
        "Enter commands like 'L100', 'R100', 'L100,R100', 'T200', 'T-200', 'S' for stop."
    )
    print("Type 'exit' to quit.")

    while True:
        # Read user input
        command = input("Enter command: ").strip()

        if command.lower() == "exit":
            print("Exiting...")
            break

        if validate_command(command):
            print(f"Sending: {command}")
            send_udp_command(ESP_IP, ESP_PORT, command)
        else:
            print(
                "Invalid command format. Use 'Lx', 'Rx', 'Lx,Ry', 'Tx' or 'S', where x, y are speeds."
            )


def validate_command(command):
    """Validates the command format according to the expected motor control formats."""
    if command == "S":
        return True  # Simple stop command

    parts = command.split(",")
    if all(validate_part(part) for part in parts):
        return True

    return False


def validate_part(part):
    """Validate individual parts of a command, which could be 'Lx', 'Rx', or 'S'."""
    if not part:
        return False
    action = part[0].upper()

    KNOWN_ACTIONS = ["L", "R", "T", "S", "F"]
    KNOWN_SINGLE_LETTER_ACTIONS = ["S", "F"]
    KNOWN_NUMERIC_ACTIONS = ["L", "R", "T"]

    if action not in KNOWN_ACTIONS:
        return False
    if action in KNOWN_SINGLE_LETTER_ACTIONS:
        return len(part) == 1  # Single letter actions must be alone
    if action in KNOWN_NUMERIC_ACTIONS:
        speed = part[1:]
        if speed.isdigit() or (speed.startswith("-") and speed[1:].isdigit()):
            return True  # Speed is a valid integer, positive or negative
    return False


if __name__ == "__main__":
    main()
