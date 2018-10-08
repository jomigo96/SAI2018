# Programa com a fórmula de Vicenty

Esta versão usa a fórmula de Vicenty (atan em vez de acos).

## Uso

A implementação é feita na forma de uma biblioteca dinâmica `libfms.so`, para facilitar os testes. O `main` apenas recebe o ficheiro na linha de comandos e chama a função `route_distance`. Para compilar:
```
make
```
Para executar:
```
./main < path to input file >
```

E para executar com um ficheiro predifinido:
```
make run
```

## Testes

### Valgrind

O teste de memória pode ser corrido com 
```
make memcheck
```

## Unitários

TODO: usar gtests. Parece que este algoritmo tem maior erro que o outro. Também falta verificar se há problemas na longitude 180º.
