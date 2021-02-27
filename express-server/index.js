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

app.use(express.json());

app.get('/', (req, res) => {
  res.sendFile(demoHtml);
})

app.post('/scan-pc', (req, res) => {
  console.log(req.body.percent);
  const { percent } = req.body;
  const msg = JSON.stringify({ percent });
  io.emit('percent', msg);
  res.sendStatus(200);
});

server.listen(5000);