# FTP

FTP server and client following standards in ***RFC 959***. Support active and passive connection mode.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

### Prerequisites

What things you need to install the software and how to install them

```
You need to have gcc and make in your PATH.
```

### Installing

Clone this repository.

```
git clone https://github.com/GlenGGG/FTP.git
cd FTP
```

Run make to build client and server.

```
cd ftp-client
make
cd ../ftp-server
make
```
### Usage

Start server

```
sudo ./server [server command port] [server-root-path]
```

Start client

```
sudo ./client [server ip] [server command port]
```

#### client side commands:

| Commands                           | Descriptions                                    |
| ---------------------------------- | :---------------------------------------------- |
| user [username]                    | login as [username]                             |
| pass                               | enter password                                  |
| pwd                                | print current working directory                 |
| list                               | run 'ls -l'                                     |
| cwd [directory]                    | change directory                                |
| cdup                               | back to upper directory equivalent to 'cd ..'   |
| retr [file path]                   | get file                                        |
| stor [file path]                   | store file                                      |
| pasv                               | enter passive mode                              |
| port [ip1,ip2,ip3,ip4,port1,port2] | enter active mode, refer to RFC 959 for details |
