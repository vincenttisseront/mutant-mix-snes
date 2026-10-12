ifeq ($(strip $(PVSNESLIB_HOME)),)
$(error "Set PVSNESLIB_HOME (see docs/scripts/cloud-agent-install.sh)")
endif

include $(PVSNESLIB_HOME)/devkitsnes/snes_rules

export ROMNAME := mutant-mix

GFXCONV := $(PVSNESLIB_HOME)/devkitsnes/tools/gfx4snes

.PHONY: all bitmaps clean

all: bitmaps $(ROMNAME).sfc

gfx/.stamp: tools/make_assets.py
	python3 tools/make_assets.py
	cp -f $(PVSNESLIB_HOME)/snes-examples/hello_world/pvsneslibfont.pic gfx/pvsneslibfont.pic
	cp -f $(PVSNESLIB_HOME)/snes-examples/hello_world/pvsneslibfont.pal gfx/pvsneslibfont.pal
	$(GFXCONV) -s 64 -o 16 -u 16 -p -t png -i gfx/hero0.png
	$(GFXCONV) -s 64 -o 16 -u 16 -p -t png -i gfx/hero1.png
	$(GFXCONV) -s 64 -o 16 -u 16 -p -t png -i gfx/hero2.png
	$(GFXCONV) -s 64 -o 16 -u 16 -p -t png -i gfx/hero3.png
	$(GFXCONV) -s 32 -o 16 -u 16 -p -t png -i gfx/rat.png
	$(GFXCONV) -s 8 -o 16 -u 16 -p -m -y -M 1 -t png -i gfx/level1.png
	touch gfx/.stamp

bitmaps: gfx/.stamp

clean: cleanBuildRes cleanRom cleanGfx
	rm -f gfx/*.pic gfx/*.pal gfx/*.map src/level1_col.c
