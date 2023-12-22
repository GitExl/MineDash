CC=cl65
SRC_FILES=src/camera.c src/entities.c src/input.c src/random.c src/level.c src/player.c src/sprites.c src/text.c src/digger.c src/gold.c src/diamond.c src/exit.c src/main.c src/level_names.c src/rock.c src/explode.c

make:
	$(CC) --static-locals --mapfile build/MINE32.MAP -Osr -Cl -o build/MINE32.PRG -t cx16 $(SRC_FILES)
	ls -l build/MINE32.PRG

run: reso make
	./x16emu/x16emu.exe -prg ./build/MINE32.PRG -run -scale 2 -quality nearest -startin ./build

reso:
	python3 ./src_res/main.py
