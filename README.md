### Simplified Explanation of Epoll API
The epoll API is used to monitor multiple file descriptors to see if I/O is possible. It can be more efficient than other similar APIs, especially when dealing with a large number of file descriptors.

### Key Concepts:
>    Epoll Instance:
        - Interest List: Contains file descriptors the process is interested in.
        - Ready List: Contains file descriptors that are ready for I/O. The kernel dynamically updates this list based on I/O activity.
    System Calls:
        - epoll_create(2): Creates a new epoll instance and returns a file descriptor for it.
        - epoll_ctl(2): Adds or removes file descriptors to/from the interest list.
        - epoll_wait(2): Waits for I/O events and fetches items from the ready list, blocking if no events are available.
    Trigger Modes:
        - Edge-triggered (ET): Only notifies when changes occur. Requires non-blocking file descriptors and careful handling to avoid missing events.
        - Level-triggered (LT): Notifies as long as a file descriptor is ready, similar to poll(2).
Example Usage:
Level-triggered:

```
#define MAX_EVENTS 10
struct epoll_event ev, events[MAX_EVENTS];
int listen_sock, conn_sock, nfds, epollfd;

epollfd = epoll_create1(0);
ev.events = EPOLLIN;
ev.data.fd = listen_sock;
epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev);

for (;;) {
    nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
    for (int n = 0; n < nfds; ++n) {
        if (events[n].data.fd == listen_sock) {
            conn_sock = accept(listen_sock, (struct sockaddr *) &addr, &addrlen);
            setnonblocking(conn_sock);
            ev.events = EPOLLIN | EPOLLET;
            ev.data.fd = conn_sock;
            epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev);
        } else {
            do_use_fd(events[n].data.fd);
        }
    }
}
```

### SERVER ENTRY POINT
```
Servers::Servers(ConfigDB &configDB) : _epoll_fds(-1), _server_fds(), _domain_to_server(), _ip_to_server(), 
	_keyValues(), server_index(), server_fd_to_index(), client_to_server(), _client_amount(0),
	_client_data(), _cgi_clients_childfd(), _client_time(), configDB_(configDB)
{
	servers = this;
	stop_fd = -1;
	_cgi_clients = std::map<int, CgiClient*>();
	_keyValues = configDB_.getKeyValue();
	createServers();
	initEvents();
}

void Servers::createServers() {
    std::cout << "|" << std::string(7, ' ') << CURSIVE_GRAY << " Creating servers..." << RESET << std::string(9, ' ') << "|" << std::endl;
    std::vector<std::string> ports;
    createEpoll();
    ports = getPorts();

    const int serverNumberWidth = 15;
    const int portWidth = 20;

    for (std::vector<std::string>::iterator it2 = ports.begin(); it2 != ports.end(); it2++) {
        if (!checkSocket(*it2)) {
            if (createSocket()) {
                (!bindSocket(*it2) || !listenSocket() || !setNonBlocking(_server_fds.back()) || !combineFds(_server_fds.back()))
                    ? _server_fds.pop_back();
                    : assignDomain(*it2, _server_fds.back());
            }
        }
    }
}
```
The loop in createServers iterates over each port obtained from the configuration.
For each port, it checks if the socket is valid and available for use by calling the checkSocket function.
If the socket is available (checkSocket returns false), it proceeds to create the socket, bind it to the port, set it to listen for incoming connections, and perform other necessary setup tasks.
If any of these setup tasks fail (e.g., socket creation, binding, listening, or setting non-blocking mode), it removes the server file descriptor from _server_fds vector.
If all setup tasks succeed, it assigns domain names to the server file descriptor and prints a row indicating the server ID and its corresponding port.

### CREATING EPOLL
```
    void	Servers::createEpoll(){
        int epoll_fd = epoll_create1(0);
        this->_epoll_fds = epoll_fd;
        if (epoll_fd < 0) {
            std::cerr << "Epoll creation failed" << std::endl;
            exit(1);
        }
    }
```
The epoll_fd variable is used to store the file descriptor of the epoll instance created using epoll_create1(0). This file descriptor represents the epoll instance, which is an in-kernel data structure used for monitoring multiple file descriptors for I/O events.

Later in the code, this epoll file descriptor (epoll_fd) will be used to add file descriptors of interest (such as socket file descriptors) to the epoll instance using epoll_ctl() with the EPOLL_CTL_ADD command. This allows the program to monitor these file descriptors for specific I/O events, such as readiness for reading or writing.

Additionally, the epoll file descriptor will be used in the epoll_wait() function to wait for I/O events on the monitored file descriptors. When I/O events occur, epoll_wait() will return, providing information about the file descriptors that are ready for I/O operations. This allows the program to efficiently handle I/O events on multiple file descriptors without the need for polling or blocking on individual file descriptors.

### Check and Validating Socket

The checkSocket function is likely used to check whether a particular port is already in use by another process. It typically involves attempting to create a socket and bind it to the specified port. If the binding fails with an error indicating that the port is already in use, the function returns true, indicating that the port is unavailable.

Input: The function takes the port number as input.

Socket Creation: It attempts to create a socket using the socket() function. This creates a communication endpoint and returns a file descriptor.

Address Binding: It tries to bind the socket to the specified port using the bind() function. If the port is already in use, this step will fail, returning an error code.

Check Result: If the binding fails with an error indicating that the port is already in use, the function returns true, indicating that the port is unavailable. Otherwise, it returns false, indicating that the port is available.

```
    int Servers::checkSocket(std::string ip){
        std::string ip_string;
        std::string port_string;
        if (ip.find(":") == std::string::npos)
            return checkSocketPort(ip);
        else{
            std::stringstream ss_ip;
            std::stringstream ss_port;
            ss_ip << ip;
            getline(ss_ip, ip_string, ':');
            ss_ip >> port_string;
            if (checkSocketPort(port_string) == 1)
                return 1;
        }
        struct sockaddr_in sa;
        int result = inet_pton(AF_INET, ip_string.c_str(), &(sa.sin_addr));
        if (result == 0) {
            std::cerr << "Invalid address" << std::endl;
            return 1;
        }
        else if (result == -1) {
            std::cerr << "Address conversion failed" << std::endl;
            return 1;
        }
        return 0;
    }
```

In our implementation, The Servers::checkSocket function is designed to check whether a given IP address or IP address and port combination is already in use.

Input Handling: The function takes an IP address as a string input. If the input contains a port (i.e., it is in the format "ip
"), it extracts the IP address and port separately.

Check Port: If the input does not contain a port (i.e., it is just an IP address), the function calls checkSocketPort to check whether the port is already in use.

IP Address Validation: The function then attempts to convert the IP address string to a binary representation using inet_pton (presentation to network). The inet_pton function: is part of the POSIX socket API and is used to convert an IPv4 or IPv6 address from its presentation format (such as a string) to its network format (a binary representation).

Function Signature: The inet_pton function has the following signature:

int inet_pton(int af, const char *src, void *dst);
    - af: Address family, which can be AF_INET for IPv4 or AF_INET6 for IPv6.
    - src: A pointer to the string containing the IPv4 or IPv6 address in presentation format.
    - dst: A pointer to a buffer where the binary representation of the address will be stored.
    - Return Value:

If the conversion is successful, inet_pton returns 1.
If the input string does not contain a valid IPv4 or IPv6 address, it returns 0.
If an error occurs, it returns -1, and the errno variable is set to indicate the error.

```
    int Servers::createSocket(){
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1) {
            std::cerr << "Socket creation failed" << std::endl;
            return (0);
        }
        _server_fds.push_back(server_fd);
        return (1);
    }
```

This createSocket function is responsible for creating a new socket for the server to listen for incoming connections.

### Socket Creation:

The function calls the socket system call to create a new socket.
The socket function takes three arguments:
    - AF_INET: Specifies the address family, in this case, IPv4.
    - SOCK_STREAM: Specifies the type of socket, in this case, a stream socket for TCP communication.
    - 0 (or IPPROTO_IP): Specifies the protocol to be used. In this case, the default protocol for the given socket type and address family is used.
If the socket call fails (returns -1), an error message is printed, and the function returns 0 to indicate failure.

Server File Descriptor:

Upon successful creation of the socket, a file descriptor (server_fd) is returned, representing the socket.
This file descriptor is a unique identifier for the socket within the process and is used to refer to the socket in subsequent operations.
Saving File Descriptor:

The server_fd is then added to the _server_fds vector.
_server_fds is a member variable of the Servers class, likely declared as std::vector<int> _server_fds, where it stores the file descriptors of all created server sockets.
Return Value:

If the socket creation is successful, the function returns 1 to indicate success.
If the socket creation fails, the function returns 0 to indicate failure.
In summary, createSocket creates a new socket for the server, adds its file descriptor to a vector for later reference, and returns a status indicating whether the operation was successful or not. The file descriptor is crucial for subsequent operations on the socket, such as binding, listening, and accepting connections.


### Binding Socket

```
    int Servers::bindSocket(std::string s_port){
        if (_server_fds.back() == -1)
        {
            std::cerr << "Socket binding impossible!" << std::endl;
            return (0);
        }
        std::stringstream ss;
        ss << s_port;
        int port;
        std::string ip_string;
        const char *c_ip = NULL;
        if (s_port.find(":") == std::string::npos)
        {
            port = std::atoi(s_port.c_str());
            _ip_to_server[_server_fds.back()] = "127.0.0.1:" + s_port;
        }
        else
        {
            _ip_to_server[_server_fds.back()] = s_port;
            getline(ss, ip_string, ':');
            ss >> port;
            c_ip = ip_string.c_str();
        }
        int opt = 1;
        setsockopt(_server_fds.back(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
        struct sockaddr_in address;
        std::memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        if (c_ip != NULL)
            address.sin_addr.s_addr = inet_addr(c_ip);
        else
            address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        address.sin_port = htons(port);
        if (bind(_server_fds.back(), (struct sockaddr *)&address, sizeof(address)) == -1) {
            std::cerr << "Bind failed" << std::endl;
            return (0);
        }
        for (std::map<int, std::vector<std::string> >::iterator it = server_index.begin(); it != server_index.end(); it++){
            for (std::vector<std::string>::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++){
                if (*it2 == s_port)
                    server_fd_to_index[_server_fds.back()] = it->first;
            }
        }
        return (1);
    }
```

The bindSocket function is responsible for binding a socket to a specific IP address and port number. Let's break down how it works:

Error Checking:

The function first checks if the last server socket file descriptor _server_fds.back() is valid. If it's -1, it indicates that socket creation failed, and the function prints an error message and returns 0 to indicate failure.
Parsing IP Address and Port:

The function parses the input s_port to extract the IP address and port number. If s_port does not contain a colon (":"), it assumes that only the port number is provided, and the IP address is set to the loopback address "127.0.0.1". Otherwise, it extracts the IP address and port from the input string.
It sets c_ip to point to the extracted IP address as a C-style string.
Setting Socket Options:

It sets the SO_REUSEADDR socket option to allow reusing the same port number if the socket is in the TIME_WAIT state.
This option enables multiple sockets to bind to the same address and port number, which can be useful for server applications that need to restart quickly after a shutdown.
Preparing Address Structure:

It initializes a sockaddr_in structure address and sets its fields:
sin_family to AF_INET to indicate IPv4.
sin_addr to the parsed IP address (c_ip) or the loopback address if no IP address is provided.
sin_port to the parsed port number (port), converted to network byte order using htons().
Binding the Socket:

It calls the bind function to bind the socket to the specified address and port.
If the bind call fails, the function prints an error message and returns 0 to indicate failure.
Associating Server Index:

After successful binding, the function iterates over a map server_index, which likely maps server indexes to lists of port numbers.
For each entry in server_index, it checks if the current port number s_port matches any port number in the list. If it finds a match, it associates the server socket file descriptor _server_fds.back() with the corresponding server index.
Return Value:

If the binding is successful, the function returns 1 to indicate success.
In summary, the bindSocket function binds a server socket to a specified IP address and port number, sets socket options for reusing the address, and associates the socket with a server index




### setsockopt(_server_fds.back(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int))

The line setsockopt(_server_fds.back(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)); sets a socket option on the server socket file descriptor _server_fds.back() to enable the reuse of local addresses. Let's break down each parameter and how it works:

_server_fds.back(): This is the file descriptor of the server socket. It refers to the most recently created server socket.

SOL_SOCKET: This parameter specifies the level at which the option is defined. In this case, it indicates that the option applies to the socket layer.

SO_REUSEADDR: This parameter specifies the particular socket option being set. SO_REUSEADDR allows the reuse of local addresses. When this option is enabled, it allows other sockets to bind to the same address and port number, even if the socket that originally bound to that address is still in the TIME_WAIT state.

&opt: This is a pointer to the value that specifies whether to enable or disable the SO_REUSEADDR option. In this case, it points to the variable opt, which is presumably set to 1, indicating that the option should be enabled.

sizeof(int): This parameter specifies the size of the option value. Since opt is an int, sizeof(int) ensures that the correct size of the option value is passed to the setsockopt function.

By calling setsockopt with SO_REUSEADDR set to 1, the server socket is configured to allow the reuse of local addresses, which can be beneficial for server applications that need to quickly restart and bind to the same address and port.

inet_addr(c_ip) and htonl(INADDR_LOOPBACK) are both functions used for converting IP addresses between different formats. Here's what each does:

inet_addr(c_ip):

This function converts the string representation of an IPv4 address c_ip into its 32-bit binary representation in network byte order (big-endian).
c_ip is a pointer to a null-terminated string containing the IPv4 address in dotted-decimal notation (e.g., "127.0.0.1").
It returns the binary representation of the IPv4 address. If the input string is not a valid IPv4 address, INADDR_NONE is returned.
In the provided code, c_ip is assigned the string representation of the IP address extracted from s_port.
htonl(INADDR_LOOPBACK):

This function converts the 32-bit IPv4 address INADDR_LOOPBACK from host byte order to network byte order (big-endian).
INADDR_LOOPBACK is a constant defined in <netinet/in.h> representing the loopback address 127.0.0.1.
htonl stands for "host to network long".
It returns the 32-bit IPv4 address in network byte order.
In the provided code, INADDR_LOOPBACK is used as the address for binding the socket to the loopback interface.
In summary, inet_addr converts a string IP address to its binary representation, while htonl converts a 32-bit IPv4 address from host to network byte order. They are used here to set the IP address of the server socket.
The function htons(port) is used to convert a 16-bit unsigned short integer from host byte order to network byte order (big-endian).

port: This is the port number in host byte order that you want to convert to network byte order.

Return Value: The function returns the 16-bit port number converted to network byte order.

Purpose: Network protocols, including TCP/IP, often require data to be transmitted in network byte order, which is big-endian. The htons function ensures that port numbers are correctly represented in the required byte order when passed to network functions like bin.d, connect, etc.

### bind(_server_fds.back(), (struct sockaddr *)&address, sizeof(address))
The bind function is used to associate a socket with a specific IP address and port number. Here's how it works:

_server_fds.back(): This retrieves the file descriptor of the socket that you want to bind.

(struct sockaddr *)&address: This is a pointer to a sockaddr_in structure that contains the IP address and port number to bind to. The sockaddr_in structure is cast to a generic sockaddr structure pointer because bind expects a pointer to a sockaddr structure. This is a common practice in C and C++ to handle different types of socket addresses (sockaddr and its derivatives) using a single function.

sizeof(address): This specifies the size of the sockaddr_in structure pointed to by (struct sockaddr *)&address.

Return Value: The bind function returns 0 on success, and -1 if an error occurs.

Purpose: Binding a socket allows you to specify the local address and port that the socket will use for communication. It's typically done for server sockets to specify the address and port on which the server will listen for incoming connections.

```
int Servers::listenSocket(){
	if (listen(_server_fds.back(), SOMAXCONN) == -1) {
		std::cerr << "Listen failed" << std::endl;
		return (0);
	}
	return (1);
}
```
The listenSocket function is responsible for configuring the server socket to listen for incoming connections. Here's a breakdown of its purpose:

Socket Listening: The function calls the listen system call, which configures the socket referenced by _server_fds.back() to listen for incoming connections.

Backlog Parameter: The SOMAXCONN constant specifies the maximum length for the queue of pending connections. It's a system-defined value indicating the maximum number of pending connections that can wait to be accepted by the server.

Error Handling: If the listen call fails (returns -1), the function prints an error message indicating that listening failed, usually due to a system resource limit or another issue. Then it returns 0 to indicate failure.

Success: If the listen call succeeds, meaning the socket is successfully configured for listening, the function returns 1 to indicate success.

In summary, the purpose of the listenSocket function is to set up the server socket so that it can accept incoming connections from clients, enabling the server to communicate with them.


```
int Servers::setNonBlocking(int fd){
	int flags = fcntl(fd, F_GETFL);
	if (flags == -1)
	{
		std::cerr << "Fcntl failed" << std::endl;
		return (0);
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		std::cerr << "Fcntl failed" << std::endl;
		return (0);
	}
	if (fcntl(fd, F_SETFD, FD_CLOEXEC) == -1)
    {
        std::cerr << "Fcntl failed" << std::endl;
        return (0);
    }
	return (1);
}
```

The setNonBlocking function is responsible for setting a file descriptor to non-blocking mode. Let's break down how it works:

Getting Current Flags: The function first retrieves the current flags associated with the file descriptor fd using the fcntl function with the F_GETFL command. If this operation fails (returns -1), it prints an error message and returns 0 to indicate failure.

Setting Non-blocking Mode: If the retrieval of flags succeeds, the function then sets the O_NONBLOCK flag using bitwise OR with the retrieved flags. This flag indicates that I/O operations on the file descriptor should be non-blocking. The fcntl function with the F_SETFL command is used for this purpose. If setting the non-blocking mode fails, it prints an error message and returns 0.

Setting FD_CLOEXEC: Additionally, the function sets the FD_CLOEXEC flag using the fcntl function with the F_SETFD command. This flag ensures that the file descriptor is automatically closed when executing a new program via one of the exec family of functions. If setting this flag fails, it prints an error message and returns 0.

Success: If all operations succeed, the function returns 1 to indicate that the file descriptor has been successfully set to non-blocking mode and configured to close on execution of a new program.

In summary, the setNonBlocking function ensures that the specified file descriptor operates in non-blocking mode, which is essential for asynchronous I/O operations, and configures it to close automatically when executing a new program.


```
int Servers::combineFds(int socket_fd){
	struct epoll_event event;
	std::memset(&event, 0, sizeof(event));
	event.events = EPOLLIN;
	event.data.fd = socket_fd;
	if (epoll_ctl(this->_epoll_fds, EPOLL_CTL_ADD, socket_fd, &event) == -1) {
		std::cerr << "Epoll_ctl failed" << std::endl;
		return (0);
	}
	return (1);
}
```

The combineFds function is responsible for adding a file descriptor to the epoll instance for monitoring. Let's go through how it works:

Creating an epoll_event: The function starts by creating an epoll_event structure named event and initializes it using memset to ensure all its members are set to 0. This structure represents the event to be monitored by epoll.

Setting Event Type and File Descriptor: It then sets the events member of the event structure to EPOLLIN, indicating that the file descriptor will be monitored for incoming data readiness. Additionally, it sets the data.fd member to the specified socket_fd, indicating the file descriptor to monitor.

Adding File Descriptor to epoll Instance: The function calls epoll_ctl with the EPOLL_CTL_ADD command to add the specified socket_fd to the epoll instance associated with _epoll_fds. If this operation fails (returns -1), it prints an error message and returns 0 to indicate failure.

Success: If the addition of the file descriptor to epoll succeeds, the function returns 1 to indicate success.

In summary, the combineFds function facilitates the integration of a file descriptor into the epoll instance, allowing it to be monitored for events such as incoming data readiness.

```
void Servers::assignDomain(std::string port, int server_fd){
	if (port == "80")
		assignLocalDomain(server_fd);
	std::map<std::string, std::vector<std::string> > config = getKeyValue();
	for (std::map<std::string, std::vector<std::string> >::iterator it = config.begin(); it != config.end(); it++)
	{
		for (std::vector<std::string>::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++)
		{
			if (*it2 == port){
				std::string server_name = it->first;
				std::size_t pos = server_name.find("]");
				server_name = server_name.substr(0, pos + 1);
				std::string domain = server_name + ".server_name";
				std::map<std::string, std::vector<std::string> >::iterator it_domain = config.find(domain);
				if (it_domain != config.end()){
					for (std::vector<std::string>::iterator it3 = it_domain->second.begin(); it3 != it_domain->second.end(); it3++){
						if (std::find(_domain_to_server[server_fd].begin(), _domain_to_server[server_fd].end(), *it3) == _domain_to_server[server_fd].end())
							_domain_to_server[server_fd].push_back(*it3);
					}
				}
				else{
					if (std::find(_domain_to_server[server_fd].begin(), _domain_to_server[server_fd].end(), "localhost") == _domain_to_server[server_fd].end())
						_domain_to_server[server_fd].push_back("localhost");
				}
			}
		}
	}
}

void Servers::assignLocalDomain(int server_fd){
	std::map<std::string, std::vector<std::string> > config = getKeyValue();
	for (std::map<std::string, std::vector<std::string> >::iterator it_domain = config.begin(); it_domain != config.end(); it_domain++){
		if (it_domain->first.find("server_name") != std::string::npos){
			std::string server_name = it_domain->first;
			std::size_t pos = server_name.find("]");
			server_name = server_name.substr(0, pos + 1);
			std::string domain = server_name + ".listen";
			std::map<std::string, std::vector<std::string> >::iterator it_domain_listen = config.find(domain);
				if (it_domain_listen == config.end()){
					for (std::vector<std::string>::iterator it = it_domain->second.begin(); it != it_domain->second.end(); it++){
					if (std::find(_domain_to_server[server_fd].begin(), _domain_to_server[server_fd].end(), *it) == _domain_to_server[server_fd].end())
						_domain_to_server[server_fd].push_back(*it);
				}
			}
		}
	}
}
```

These two functions, assignLocalDomain and assignDomain, are responsible for assigning domain names to server file descriptors based on the configuration settings.

Here's how they work:

assignLocalDomain(int server_fd)
Retrieve Configuration Settings: The function retrieves the configuration settings using the getKeyValue function, which returns a map containing various server configurations.

Iterate Through Configuration: It then iterates through the configuration map to find entries related to server names (server_name).

Extract Server Name and Domain: For each server entry found, it extracts the server name and constructs a domain name from it by appending ".listen".

Check for Existing Assignment: It checks if the domain name has already been assigned to the server file descriptor. If not, it adds the domain name to the _domain_to_server map associated with the server_fd.

assignDomain(std::string port, int server_fd)
Check Port Number: If the port number is "80", it calls the assignLocalDomain function to handle the assignment of local domains.

Retrieve Configuration Settings: Similar to the previous function, it retrieves the configuration settings using getKeyValue.

Iterate Through Configuration: It iterates through the configuration settings to find entries related to the specified port.

Extract Server Name and Domain: For each port entry found, it extracts the server name and constructs a domain name from it by appending ".server_name".

Check for Existing Assignment: It checks if the domain name has already been assigned to the server file descriptor. If not, it adds the domain name to the _domain_to_server map associated with the server_fd. If no domain name is found in the configuration settings, it defaults to assigning "localhost" as the domain.

These functions essentially map domain names to server file descriptors based on the configuration settings provided. This mapping is useful for handling requests directed to specific domains on the server.