import socket
import time

LOW_SPEED = 175
HIGH_SPEED = 255


def send_udp_command(ip, port, message):
    """Sends a single UDP packet to the given IP address and port."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)  # Create a UDP socket
    sock.sendto(message.encode(), (ip, port))  # Send the message
    sock.close()


def main():
    # IP and port of the ESP32 UDP server
    ESP_IP = "tank-t34-esp32.local"  # Replace with your ESP32's IP address
    ESP_PORT = 4210  # Replace with the UDP port your ESP32 is listening on

    # Commands to send - adjust these according to your ESP32's expected format
    commands = [
        f"L{LOW_SPEED},R{LOW_SPEED}",  # Slow speed
        "L0,R0",  # Stop motors
        f"L{HIGH_SPEED},R{HIGH_SPEED}",  # Fast speed
        "L0,R0",  # Stop motors
        f"L{-LOW_SPEED},R{-LOW_SPEED}",  # Slow speed
        "L0,R0",  # Stop motors    ]
        f"L{-HIGH_SPEED},R{-HIGH_SPEED}",  # Slow speed
        "L0,R0",  # Stop motors
    ]

    # Send commands with a delay between each
    for command in commands:
        print(f"Sending: {command}")
        send_udp_command(ESP_IP, ESP_PORT, command)
        time.sleep(2)  # Delay between commands, adjust as needed


if __name__ == "__main__":
    main()
