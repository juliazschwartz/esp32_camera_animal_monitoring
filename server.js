// server.js
const http = require('http');
const path = require('path');
const fs = require('fs');

const WebSocket = require('ws');
// === Servidor HTTP que entrega o HTML ===
const server = http.createServer((req, res) => {
  if (req.url === "/") {
    const filePath = path.join(__dirname, "public/index-predict.html");
    fs.readFile(filePath, (err, data) => {
      if (err) {
        res.writeHead(500);
        res.end("Erro ao carregar página");
      } else {
        res.writeHead(200, { "Content-Type": "text/html" });
        res.end(data);
      }
    });
  } else {
    res.writeHead(404);
    res.end("Página não encontrada");
  }
});
const wss = new WebSocket.Server({
  port: 8080,
  perMessageDeflate: false  // 🚫 desliga a compressão
});
let clients = new Set();
wss.on('error', (err) => {
  console.error('WS client error:', err);
});
wss.on('connection', (ws) => {
    ws.on('error', (err) => {
    console.warn('WS client error (ignored):', err.message);
  });
  console.log('📡 Cliente conectado');
  clients.add(ws);

  ws.on('message', (message, isBinary) => {
    if (isBinary) {
  
      // reenvia para todos os outros clientes
      for (let client of clients) {
        if (client !== ws && client.readyState === WebSocket.OPEN) {
          client.send(message, { binary: true });
        }
      }

    } else {
      // 📝 Texto (JSON ou string simples)
      const msgText = message.toString();

      try {
        const parsed = JSON.parse(msgText);

        // já vem no formato { type, data }, só repassar
        const payload = JSON.stringify(parsed);

        for (let client of clients) {
          if (client !== ws && client.readyState === WebSocket.OPEN) {
            client.send(payload);
          }
        }

      } catch (err) {
        console.log('📩 Mensagem recebida (texto simples):', msgText);

        const payload = JSON.stringify({
          type: 'info',
          data: msgText
        });

        for (let client of clients) {
          if (client !== ws && client.readyState === WebSocket.OPEN) {
            client.send(payload);
          }
        }
      }
    }
  });

  ws.on('close', () => {
    console.log('❌ Cliente desconectado');
    clients.delete(ws);
  });
});
// Sobe tudo na mesma porta
const PORT = 8000;
server.listen(PORT, () => {
  console.log(`🚀 Servidor rodando em http://localhost:${PORT}`);
});

console.log('🚀 Servidor WebSocket rodando na porta 8080');
