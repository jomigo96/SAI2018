# Diretoria de testes

Aqui encontram-se os testes unitários à função de cálculo de distância.

É usada a framework de testes [catch2](https://github.com/catchorg/Catch2), uma biblioteca só de um header em C++. Para correr, é necessário ter instalado o g++.

O resultado do cálculo da distância é comparado com o presente na página da rota em [flightplandatabase](flightplandatabase.com).

## Executar

```
make run
```

Para especificar a tolerância desejada, editar em `tests-main.cpp` a variável `error_tolerance`. Definir esta variável com 0 fará com que todos os testes falhem, e pode ver-se o erro de cada rota.

### Especificar source

Como foram definidas várias implementações da mesma fórmula, podem ser testadas ambas apenas alterando a biblioteca que é importada. Para isso, basta editar no `Makefile` o macro `LPATH` para um de:
+ `$(shell pwd)/../acos`
+ `$(shell pwd)/../vicenty`

e recompilar tudo. 
