# ft_irc

###
<div align="center">
  <img src="https://raw.githubusercontent.com/ayogun/42-project-badges/refs/heads/main/badges/ft_irce.png?raw=true" alt="Badge du projet push_swap">
</div>

## Description

**ft_irc** is a project from the 42 school curriculum.  
It is a simplified **Internet Relay Chat (IRC) server** written in C++, allowing multiple clients to communicate in real time through channels.  

The project focuses on:

- Socket programming (TCP/IP)
- Concurrent client management
- Command parsing and response formatting
- Handling multiple channels and nicknames
- Implementing IRC protocols and numeric replies
- Proper management of empty commands and edge cases
- File transfer between clients
- Automatic bot client that tracks connected users and periodically reports nicknames

---

## Features

- Accept multiple client connections
- Parse and respond to basic IRC commands: `NICK`, `USER`, `JOIN`, `PRIVMSG`, `QUIT`
- Send numeric replies according to IRC protocol
- Handle client disconnection
- Support multiple channels
- Handle empty commands and invalid inputs
- Implement `MODE`, `KICK`, and other advanced commands
- Maintain server stability under concurrent operations
- Automatic bot connection for testing

---

## Usage

### Compilation
```bash
make
```

This will generate the executable irc_serv.

## Running the server

```bash
./irc_serv <port> <password>
```
Example: ./irc_serv 6667 mypassword

## Connecting with a client 

Use a client such as irssi or netcat:

```bash
irssi
```

```bash
/connect 127.0.0.1 6667 mypassword
```

## Algorithm & Design Choices

- **Client Representation**:  
  Each client is represented by a `Client` object containing:
  - File descriptor
  - Nickname and user information
  - Buffers for incoming and outgoing messages

- **Channel Management**:  
  Channels maintain a list of connected clients and operators.

- **Message Handling**:  
  All messages follow the IRC protocol format: nick!user@host command params

- **Nickname Collisions**:  
When a nickname is already in use, a unique suffix is automatically appended to avoid conflicts.

- **File Transfer**:  
Files are sent in chunks with headers to ensure data integrity.

- **Bot Implementation**:  
The bot is a client connected to the server that periodically sends messages displaying the nicknames of connected users.

- **Robustness**:  
Special care is taken to avoid race conditions and memory leaks.


## Author

mleineku – 42 School Student

## License

This project is open-source and reusable for personal or educational purposes.
