const http = require('http');
const path = require('path');
const fs = require('fs');

const WebSocket = require('ws');

let savingFrames = false;

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
  server: server,
  perMessageDeflate: false
});
let clients = new Set();
wss.on('error', (err) => {
  console.error('WS client error:', err);
});
wss.on('connection', (ws) => {
    ws.on('error', (err) => {
    console.warn('WS client error (ignored):', err.message);
  });
  console.log('Cliente conectado');
  clients.add(ws);

  ws.on('message', (message, isBinary) => {
    if (isBinary) {
      if (savingFrames) {
        try {
          const framesDir = path.join(__dirname, "frames");
          fs.mkdirSync(framesDir, { recursive: true });

          const timestamp = new Date().toISOString().replace(/[:.]/g, "-");

          const filename = `frame_${timestamp}.jpg`;
          const fullPath = path.join(framesDir, filename);

          fs.writeFileSync(fullPath, message);
          console.log(framesDir)


        } catch (err) {
          console.error("Erro ao salvar frame:", err);
        }
      }
      for (let client of clients) {
        if (client !== ws && client.readyState === WebSocket.OPEN) {
          client.send(message, { binary: true });
        }
      }

    } else {
      const msgText = message.toString();

      try {
        const parsed = JSON.parse(msgText);
        const payload = JSON.stringify(parsed);
        if (parsed && typeof parsed === "object" && "type" in parsed) {
          if (parsed.type === "save") {
            savingFrames = parsed.value;
            console.log(`salvar frames ${parsed.value}`)
            return; 
          }
        }

        for (let client of clients) {
          if (client !== ws && client.readyState === WebSocket.OPEN) {
            client.send(payload);
          }
        }

      } catch (err) {
        console.log('Mensagem recebida (texto simples):', msgText);

        const payload = JSON.stringify({
          type: 'info',
          data: msgText
        });

        //for (let client of clients) {
         // if (client !== ws && client.readyState === WebSocket.OPEN) {
          //  client.send(payload);
         // }
       // }
      }
    }
  });

  ws.on('close', () => {
    console.log('Cliente desconectado');
    clients.delete(ws);
  });
});
const PORT = 8000;
server.listen(PORT, () => {
  console.log(`Servidor rodando em http://127.0.0.1:${PORT}`);
});

console.log(`Servidor WebSocket rodando na porta ${PORT}`);
