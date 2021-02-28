const express = require('express')
const http = require('http')
const socketIo = require('socket.io')
const { resolve } = require('path')

const demoHtml = resolve(__dirname, '..', 'dist', 'demo.html')

const app = express()
const server = http.Server(app)
const io = socketIo(server, {
  cors: {
    origin: 'http://localhost:1234',
    methods: ['GET', 'POST']
  }
})

io.on('connection', (socket) => {
  console.log('a user connected')
})

app.use(express.static('assets'))

const jsonParser = express.json({ limit: '100mb' })
const rawParser = express.raw({ limit: '1000mb', type: 'application/json' })

app.get('/', (req, res) => {
  res.sendFile(demoHtml)
})

app.post('/scan-start', jsonParser, (req, res) => {
  console.log(req.body)
  const { id, label, timestamp } = req.body
  const msg = JSON.stringify({ id, label, timestamp })
  io.emit('scan:start', msg)
  res.sendStatus(200)
})

app.post('/scan-pc', rawParser, (req, res) => {
  io.emit('percent', req.body.toString())
  res.sendStatus(200)
})

app.post('/scan-files-stats', rawParser, (req, res) => {
  io.emit('scan:stats', req.body.toString())
  res.sendStatus(200)
})

app.post('/dir-stats', rawParser, (req, res) => {
  io.emit('scan:dir-stats', req.body.toString())
  res.sendStatus(200)
})

app.post('/files', rawParser, (req, res) => {
  const files = req.body.toString()
  io.emit('files', files)
  res.sendStatus(200)
})

server.listen(5000)
