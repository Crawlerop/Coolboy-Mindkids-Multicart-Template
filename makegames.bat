@echo off

python multitools\create_multi.py CAT_GAMES
python multitools\gen_names.py game_names.txt names.bin
rem python multitools\create_multigames.py 0 0 .\GAMES 0 0 > games.inc

:end
pause