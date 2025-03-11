#!/bin/sh
# Auto-generated build script for TSL

rm tsl 2>/dev/null

gcc -DTSL_GUI  \
	main.c \
	web.c \
	dwiminv.c \
	mt19937ar.c \
	keymap.c \
	menuitem.c \
	reading.c \
	eat.c \
	select.c \
	bestiary.c \
	wingame.c \
	inscript.c \
	checks.c \
	browser.c \
	equip.c \
	itemtext.c \
	itemprop.c \
	ffield.c \
	gore.c \
	craft.c \
	identify.c \
	facet.c \
	find.c \
	pushing.c \
	burdened.c \
	swimming.c \
	memory.c \
	doors.c \
	potions.c \
	sleep.c \
	balls.c \
	breath.c \
	explode.c \
	teleport.c \
	backstab.c \
	poison.c \
	wounds.c \
	places.c \
	attrs.c \
	options.c \
	rolls.c \
	area.c \
	modbuild.c \
	missile.c \
	ability.c \
	stuff.c \
	input.c \
	losegame.c \
	message.c \
	saveload.c \
	tiles.c \
	stacks.c \
	vweapon.c \
	altitude.c \
	dungeon.c \
	level.c \
	help.c \
	creature.c \
	ui.c \
	item.c \
	game.c \
	player.c \
	debug.c \
	monster.c \
	combat.c \
	effect.c \
	treasure.c \
	actions.c \
	magic.c \
	unique.c \
	traps.c \
	ai.c \
	inventory.c \
	content.c \
	shapeshf.c \
	elements.c \
	fov.c \
	rndnames.c \
	clipbrd.c \
	anim.c \
	allui.c \
	glyph.c \
	 \
	-lm -lallegro -lallegro_image -lallegro_font \
	 \
	-o tsl

exit 0
