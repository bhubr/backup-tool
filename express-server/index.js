const express = require('express');
const http = require('http');
const socketIo = require('socket.io');
const { join } = require('path');

const demoHtml = join(__dirname, 'demo.html');

const app = express();
const server = http.Server(app);
const io = socketIo(server);

io.on('connection', (socket) => {
  console.log('a user connected');
});

app.use(express.static('assets'));

const jsonParser = express.json({limit: '100mb'});
const rawParser = express.raw({limit: '100mb',type:'application/json'});

app.get('/', (req, res) => {
  res.sendFile(demoHtml);
})

app.post('/scan-pc', jsonParser, (req, res) => {
  console.log(req.body.percent);
  const { percent } = req.body;
  const msg = JSON.stringify({ percent });
  io.emit('percent', msg);
  res.sendStatus(200);
});

app.post('/files', rawParser, (req, res) => {
  const files = req.body.toString();
  console.log(files);
  // const { percent } = req.body;
  // const msg = JSON.stringify({ percent });
  io.emit('files', req.body.toString());
  res.sendStatus(200);
});

server.listen(5000);