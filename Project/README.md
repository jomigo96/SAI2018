# Conexoes UDP

O programa `ils` tem 2 threads: uma ouve num socket UDP, e quando recebe uma mensagem copia os dados para uma estrutura `position_gps`. A outra é acordada quando são recebidos novos dados, e imprime-os na consola.
Usar
```
./ils <port>
```
port deve ser 8000-8999 idealmente.


O `stub` envia dados falsos para este socket. Usar
```
./stub 127.0.0.1 <port>
```
para testar no mesmo pc.
