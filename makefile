# makefile for nesdoug example code, for Linux
# this version won't move the final files to BUILD folder
# also won't rebuild on changes to neslib/nesdoug/famitone files

CC65 = cc65
CA65 = ca65
LD65 = ld65
NAME = multicart
CFG = MindKids_Template.cfg

EXT = nes

.PHONY: default clean run

default: $(NAME).$(EXT)


#target: dependencies

$(NAME).$(EXT): $(NAME).o crt0.o $(CFG)
	$(LD65) -C $(CFG) -o $(NAME).$(EXT) crt0.o $(NAME).o nes.lib -Ln labels.txt --dbgfile dbg.txt
	rm *.o
	@echo $(NAME).$(EXT) created

crt0.o: crt0.s MUSIC/TestMusic3.s MUSIC/SoundFx.s games.inc	names.bin cart_data.inc
	$(CA65) crt0.s

$(NAME).o: $(NAME).s
	$(CA65) $(NAME).s -g

$(NAME).s: $(NAME).c Sprites.h games.h settings.h
	$(CC65) -Oirs $(NAME).c --add-source

clean:
	rm $(NAME).$(EXT)

run: $(NAME).$(EXT)
	VTEmu $(NAME).$(EXT)