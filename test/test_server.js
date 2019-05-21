const http = require('http');


function mockLongDataTransform(req, res) {
  let c = 0;
  setInterval(() => {
    res.write("" + c);
    c += 1;
  }, 1000);
}

const server = http.createServer((req, res) => {
  console.log("receive request from ", req.socket.remoteAddress);
  // setTimeout(() => {
  //   res.end("hello\n");
  // }, 10000);
  mockLongDataTransform(req, res);
});

server.listen(7001, () => {
  console.log("test server listened ok");
});