# Nome do executável
TARGET = pro

# Arquivo fonte
SRC = hair.cpp

# Compilador
CC = g++

# Flags de compilação
CFLAGS = -Wall -g

# Bibliotecas
LIBS = -lglut -lGLEW -lGL -lm -lGLU

# Regra padrão
all: $(TARGET)

# Como gerar o executável
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LIBS)

# Limpar arquivos compilados
clean:
	rm -f $(TARGET)
