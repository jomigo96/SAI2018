# Programa com a fórmula do arcos

## Uso

A implementação é feita na forma de uma biblioteca dinâmica `libfms.so`, para facilitar os testes. O `main` recebe o ficheiro de input da linha de comandos.
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

Ver diretoria `test`.
