# Variáveis para o compilador e flags
CC = gcc
CFLAGS = -Wall -Iinclude

# Diretórios de origem e destino
SRC_DIR = .
BUILD_DIR = build

# Arquivos fonte
UTILS_SRC = libc/utils.c
UTILS_OBJ = $(BUILD_DIR)/utils.o

LEITOR_SRC = leitor/main.c
LEITOR_OBJ = $(BUILD_DIR)/leitor.o

REMETENTE_SRC = remetente/main.c
REMETENTE_OBJ = $(BUILD_DIR)/remetente.o

SERVIDOR_SRC = Servidor/main.c
SERVIDOR_OBJ = $(BUILD_DIR)/servidor.o

# Nomes dos executáveis
LEITOR_EXEC = leitor
REMETENTE_EXEC = remetente
SERVIDOR_EXEC = servidor

# Alvo padrão (compilar tudo)
all: $(LEITOR_EXEC) $(REMETENTE_EXEC) $(SERVIDOR_EXEC)

# Regras para compilar cada executável
$(LEITOR_EXEC): $(LEITOR_OBJ) $(UTILS_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(REMETENTE_EXEC): $(REMETENTE_OBJ) $(UTILS_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(SERVIDOR_EXEC): $(SERVIDOR_OBJ) $(UTILS_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Regras para compilar os objetos
$(BUILD_DIR)/leitor.o: $(LEITOR_SRC)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/remetente.o: $(REMETENTE_SRC)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/servidor.o: $(SERVIDOR_SRC)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/utils.o: $(UTILS_SRC) include/utils.h
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Limpeza dos arquivos compilados
clean:
	rm -rf $(BUILD_DIR) $(LEITOR_EXEC) $(REMETENTE_EXEC) $(SERVIDOR_EXEC)

# Alvo de limpeza completa
.PHONY: all clean
