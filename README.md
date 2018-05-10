# A C-Server

This is just a basic C-Server made using Berkley Sockets.

## Getting Started

These instructions will setup this server

### Cloning the repo

Open terminal and type:
(If you want to clone it over ssh)
```
git clone git@github.com:kokil87/socketsinc.git
```

Otherwise you can clone it over https:
```
git clone https://github.com/kokil87/socketsinc.git
```

### Start Server

Open the directory using `cd` command and then type:

```
make
```
The default port used is `8080`, if you want to use any other port type:

```
make PORT=<port_number>
```

### Stop Server

Just press : `Ctrl + C`