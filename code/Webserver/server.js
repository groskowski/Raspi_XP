const express = require('express')
const webserver = express()
 .use((req, res) =>
   res.sendFile('/websocket-client.html', { root: __dirname })
 )
 .listen(3000, () => console.log(`Listening on ${3000}`))

const { WebSocketServer } = require('ws')
const sockserver = new WebSocketServer({ port: 443 })

sockserver.on('connection', ws => {
 console.log('New client connected!')
 ws.send('connection established')
 ws.on('close', () => console.log('Client has disconnected!'))
 const receivedMessagesArr = []
 ws.on('message', data => {
     console.log(`Server received: ${data}`)
     var receivedMessage = String(data)
     //receivedMessagesArr.push(receivedMessage)
     //console.log('All messages so far: ', receivedMessagesArr)
 })
 

 ws.onerror = function () {
   console.log('websocket error')
 }
})