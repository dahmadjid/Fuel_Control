import socket

server_address = ("",6205)
server_socket = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
server_socket.bind(server_address)
server_socket.listen()
server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server_socket.settimeout(3)
while True:
    try:
        esp_socket,esp_address = server_socket.accept()
    except:
        print("server timedout")
        continue

    print("connection from: ", esp_address)
    esp_socket.settimeout(10)
    while True:
        try:
            data = esp_socket.recv(128)
            for i in range(len(data)):
                print(int(data[i]))
            esp_socket.send(b"0.05,02775 109 44,Renault,Clio 3")
            esp_socket.close()
        except Exception as e:
            print("esp_socket disconnected")
            break