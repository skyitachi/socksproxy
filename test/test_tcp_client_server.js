const net = require('net');

const server = net.createServer((socket) => {
    console.log('connection received ', socket.remoteAddress, ': ', socket.remotePort);
    socket.write('message from server\n');
    socket.end();
});

server.listen(3000,() => {
    console.log('listened ok');
});