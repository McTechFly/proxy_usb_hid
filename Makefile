# Nom de la bibliothèque partagée
TARGET = raw_joystick


# Nom de l'exécutable Go
GOTARGET = raw_joystick_go

GIT_USERNAME = 
GIT_TOKEN = 
GIT_REPO = 

# Fichiers source C
SRC = ./src/main.c \
	  ./src/usb_descriptors.c \
      ./src/usb_raw.c \
      ./src/usb_hid.c \
      ./src/ep0.c \
      ./src/usb_debug.c \
      ./src/input_mapping.c

# Emplacement (relatif) du fichier Go
GOFILE = ./app/main.go

CC = gcc
CFLAGS = -Wall -Wextra -O2 -I/usr/include/libevdev-1.0 -I./include
LDFLAGS = -L/usr/lib/aarch64-linux-gnu -levdev -ljson-c

.PHONY: all git-update clean run

# La cible "all" exécute d'abord git-update, puis construit la lib, l'exécutable Go et enfin lance le binaire
all: git-update $(TARGET) $(GOTARGET) run

# Construction de la bibliothèque partagée
$(TARGET): $(SRC)
	# Suppression de l'exécutable précédent
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

# Construction de l'exécutable Go qui utilise la lib
$(GOTARGET): $(GOFILE) $(LIBTARGET)
	# On suppose que votre main.go a des directives cgo du type:
	#   #cgo LDFLAGS: -L. -lrawjoystick -ljson-c -levdev -lpthread
	# pour lier correctement la bibliothèque librawjoystick.so.
	go build -o $(GOTARGET) $(GOFILE)

# Lancement de l'exécutable Go, en s'assurant que la .so soit résolue dans le dossier courant
run: $(GOTARGET)
	LD_LIBRARY_PATH=. ./$(GOTARGET)

# Mise à jour du dépôt git avant chaque compilation (optionnel)
git-update:
	git pull https://$(GIT_USERNAME):$(GIT_TOKEN)@$(GIT_REPO)
	rm -f $(TARGET) $(GOTARGET)

clean:
	rm -f $(LIBTARGET) $(GOTARGET)
