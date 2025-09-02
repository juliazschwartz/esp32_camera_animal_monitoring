const WebSocket = require('ws');
const tf = require('@tensorflow/tfjs-node');
const { createCanvas, Image } = require('canvas');

const MODEL_PATH = 'model/model.json';
let model;

// Carrega o modelo Teachable Machine
async function loadModel() {
    model = await tf.loadLayersModel('file:///home/julia/servidor-ws/tm-my-image-model/model.json');
    console.log('Modelo carregado!');
}
loadModel();

const wss = new WebSocket.Server({ port: 8080 });
let clients = new Set();

wss.on('connection', (ws) => {
    console.log('Cliente conectado');
    clients.add(ws);

    ws.on('message', async (message, isBinary) => {
        // Repassa para todos os clientes (frame)
        for (let client of clients) {
          if (client.readyState === WebSocket.OPEN) {
            client.send(message, { binary: isBinary });
          }
        }
  
        // Se for imagem, roda predição
        if (isBinary) {
            try {
              console.log('Processando frame de tamanho', message.length);
              const imgTensor = tf.node.decodeImage(message, 3)
                .resizeBilinear([224,224])
                .expandDims(0)
                .div(255);
          
              const prediction = model.predict(imgTensor);
              const data = prediction.dataSync();
              console.log('Predição:', data);
          
              const classe = data[0] > data[1] ? 'Classe1' : 'Classe2';
              const confianca = Math.max(...data).toFixed(3);
          
              const msg = JSON.stringify({
                tipo: 'predicao',
                dados: { classe, confianca }
              });
          
              for (let client of clients) {
                if (client.readyState === WebSocket.OPEN) client.send(msg);
              }
          
            } catch(e) {
              console.error('Erro na predição:', e);
            }
          }
          
      });
  
    

    ws.on('close', () => {
        console.log('Cliente desconectado');
        clients.delete(ws);
    });
});

console.log('Servidor WebSocket rodando na porta 8080');

