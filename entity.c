/*
 Authors:
 Copyright 2012-2016 by Eduard Broese <ed.broese@gmx.de>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version
 2 of the License, or (at your option) any later version.
*/

/*
  entity : entities and entity metadata handling
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define LH_DECLARE_SHORT_NAMES 1
#include <lh_buffers.h>
#include <lh_bytes.h>

#include "entity.h"

// Entities

const char * METANAME[][32] = {
    [Entity] = {
        [0]  = "Flags",
        [1]  = "Air",
        [2]  = "Custom name",
        [3]  = "Name visible",
        [4]  = "Is silent",
        [5]  = "No gravity",
    },
    [Potion] = {
        [6]  = "Slot",
    },
    [FallingBlock] = {
        [6]  = "Position",
    },
    [AreaEffectCloud] = {
        [6]  = "Radius",
        [7]  = "Color",
        [8]  = "Single point",
        [9]  = "Particle ID",
        [10] = "Particle Parameter 1",
        [11] = "Particle Parameter 2",
    },
    [FishingFloat] = {
        [6]  = "Hooked entity",
    },
    [Arrow] = {
        [6]  = "Is critical",
    },
    [TippedArrow] = {
        [7]  = "Color",
    },
    [Boat] = {
        [6]  = "Time since hit",
        [7]  = "Forward direction",
        [8]  = "Damage taken",
        [9]  = "Type",
        [10] = "Right paddle turning",
        [11] = "Left paddle turning",
    },
    [EnderCrystal] = {
        [6]  = "Beam target",
        [7]  = "Show bottom",
    },
    [Fireball] = {
    },
    [WitherSkull] = {
        [6]  = "Invulnerable",
    },
    [Fireworks] = {
        [6] = "Firework info",
    },
    [Hanging] = {
    },
    [ItemFrame] = {
        [6] = "Item",
        [7] = "Rotation",
    },
    [Item] = {
        [6] = "Item",
    },
    [Living] = {
        [6]  = "Active hand",
        [7]  = "Health",
        [8]  = "Potion effect color",
        [9]  = "Potion effect ambient",
        [10] = "Number of arrows",
    },
    [Player] = {
        [11] = "Additional hearts",
        [12] = "Score",
        [13] = "Skin flags",
        [14] = "Main hand",
    },
    [ArmorStand] = {
        [11] = "Armor stand flags",
        [12] = "Head position",
        [13] = "Body position",
        [14] = "L arm position",
        [15] = "R arm position",
        [16] = "L leg position",
        [17] = "R leg position",
    },
    [Insentinent] = {
        [11] = "Insentinent flags",
    },
    [Ambient] = {
    },
    [Bat] = {
        [12] = "Is hanging",
    },
    [Creature] = {
    },
    [Ageable] = {
        [12] = "Is baby",
    },
    [Animal] = {
    },
    [Horse] = {
        [13] = "Horse flags",
        [14] = "Horse type",
        [15] = "Horse color",
        [16] = "Owner UUID",
        [17] = "Horse armor",
    },
    [Pig] = {
        [13] = "Has saddle",
    },
    [Rabbit] = {
        [13] = "Rabbit type",
    },
    [PolarBear] = {
		[13] = "Standing",
	},
    [Sheep] = {
        [13] = "Sheep color",
    },
    [TameableAnimal] = {
        [13] = "Tameable flags",
        [14] = "Owner UUID",
    },
    [Ocelot] = {
        [15] = "Ocelot type",
    },
    [Wolf] = {
        [15] = "Damage",
        [16] = "Begging",
        [17] = "Collar color",
    },
    [Villager] = {
        [13] = "Profession",
    },
    [Golem] = {
    },
    [IronGolem] = {
        [12] = "created by player",
    },
    [Snowman] = {
        [12] = "Flags",
    },
    [Shulker] = {
        [12] = "Direction",
        [13] = "Attachment position",
        [14] = "Shield height",
    },
    [Monster] = {
    },
    [Blaze] = {
        [12] = "On fire",
    },
    [Creeper] = {
        [12] = "Creeper state",
        [13] = "Charged",
        [14] = "Ignited",
    },
    [Guardian] = {
        [12] = "Flags",
        [13] = "Target EID",
    },
    [Skeleton] = {
        [12] = "Skeleton type",
        [13] = "Targeting",
    },
    [Spider] = {
        [12] = "Climbing",
    },
    [Witch] = {
        [12] = "Aggressive",
    },
    [Wither] = {
        [12] = "Target 1",
        [13] = "Target 2",
        [14] = "Target 3",
        [15] = "Invulnerable time",
    },
    [Zombie] = {
        [12] = "Baby zombie",
        [13] = "Villager zombie",
        [14] = "Converting zombie",
        [15] = "Hands up",
    },
    [Enderman] = {
        [12] = "Carried block",
        [13] = "Screaming",
    },
    [Enderdragon] = {
        [12] = "Phase",
    },
    [Flying] = {
    },
    [Ghast] = {
        [12] = "Attacking",
    },
    [Slime] = {
        [12] = "Size",
    },
    [Minecart] = {
        [6]  = "Shaking power",
        [7]  = "Shaking direction",
        [8]  = "Shaking multiplier",
        [9]  = "Block id/data",
        [10] = "Block y",
        [11] = "Show block",
    },
    [MinecartCommandBlock] = {
		[12] = "Command",
		[13] = "Last Output",
	},
    [MinecartFurnace] = {
        [12] = "Powered",
    },
    [ActivatedTNT] = {
		[6]  = "Fuse time",
	},
};

const EntityType ENTITY_HIERARCHY[] = {
    //// Superclasses
    [Entity]            = IllegalEntityType,
    [Potion]            = Entity,
    [FallingBlock]      = Entity,
    [AreaEffectCloud]   = Entity,
    [FishingFloat]      = Entity,
    [Arrow]             = Entity,
    [TippedArrow]       = Arrow,
    [Boat]              = Entity,
    [EnderCrystal]      = Entity,
    [Fireball]          = Entity,
    [WitherSkull]       = Fireball,
    [Fireworks]         = Entity,
    [Hanging]           = Entity,
    [ItemFrame]         = Hanging,
    [Item]              = Entity,
    [Living]            = Entity,
    [Player]            = Living,
    [ArmorStand]        = Living,
    [Insentinent]       = Living,
    [Ambient]           = Insentinent,
    [Bat]               = Ambient,
    [Creature]          = Insentinent,
    [Ageable]           = Creature,
    [Animal]            = Ageable,
    [Horse]             = Animal,
    [Pig]               = Animal,
    [Rabbit]            = Animal,
    [PolarBear]         = Animal,
    [Sheep]             = Animal,
    [TameableAnimal]    = Animal,
    [Ocelot]            = TameableAnimal,
    [Wolf]              = TameableAnimal,
    [Villager]          = Creature,
    [Golem]             = Creature,
    [IronGolem]         = Golem,
    [Snowman]           = Golem,
    [Shulker]           = Golem,
    [Monster]           = Creature,
    [Blaze]             = Monster,
    [Creeper]           = Monster,
    [Guardian]          = Monster,
    [Skeleton]          = Monster,
    [Spider]            = Monster,
    [CaveSpider]        = Spider,
    [Witch]             = Monster,
    [Wither]            = Monster,
    [Zombie]            = Monster,
    [ZombiePigman]      = Zombie,
    [Enderman]          = Monster,
    [Enderdragon]       = Insentinent,
    [Flying]            = Insentinent,
    [Ghast]             = Flying,
    [Slime]             = Insentinent,
    [MagmaCube]         = Slime,
    [Minecart]          = Entity,
    [MinecartCommandBlock] = Minecart,
    [MinecartFurnace]   = Minecart,
    [ActivatedTNT]      = Entity,
};

#define ENUMNAME(name) [name] = #name

const char * ENTITY_NAMES[MaxEntityType] = {
    ENUMNAME(Creeper),
    ENUMNAME(Skeleton),
    ENUMNAME(Spider),
    ENUMNAME(GiantZombie),
    ENUMNAME(Zombie),
    ENUMNAME(Slime),
    ENUMNAME(Ghast),
    ENUMNAME(ZombiePigman),
    ENUMNAME(Enderman),
    ENUMNAME(CaveSpider),
    ENUMNAME(Silverfish),
    ENUMNAME(Blaze),
    ENUMNAME(MagmaCube),
    ENUMNAME(Enderdragon),
    ENUMNAME(Wither),
    ENUMNAME(Bat),
    ENUMNAME(Witch),
    ENUMNAME(Endermite),
    ENUMNAME(Guardian),
    ENUMNAME(Shulker),
    ENUMNAME(PolarBear),

    ENUMNAME(Pig),
    ENUMNAME(Sheep),
    ENUMNAME(Cow),
    ENUMNAME(Chicken),
    ENUMNAME(Squid),
    ENUMNAME(Wolf),
    ENUMNAME(Mooshroom),
    ENUMNAME(Snowman),
    ENUMNAME(Ocelot),
    ENUMNAME(IronGolem),
    ENUMNAME(Horse),
    ENUMNAME(Rabbit),
    ENUMNAME(Villager),
};

const char * get_entity_name(char *buf, EntityType type) {
    if (type < 0 || type >= MaxEntityType ) {
        sprintf(buf, "%s", "IllegalEntityType");
    }
    else if ( ENTITY_NAMES[type] ) {
        sprintf(buf, "%s", ENTITY_NAMES[type]);
    }
    else {
        sprintf(buf, "%s", "UnknownEntity");
    }
    return buf;
}

////////////////////////////////////////////////////////////////////////////////
// Entity Metadata

const char * METATYPES[] = {
    [META_BYTE]     = "byte",
    [META_VARINT]   = "varint",
    [META_FLOAT]    = "float",
    [META_STRING]   = "string",
    [META_CHAT]     = "chat",
    [META_SLOT]     = "slot",
    [META_BOOL]     = "bool",
    [META_VEC3]     = "vector3f",
    [META_POS]      = "position",
    [META_OPTPOS]   = "optional_position",
    [META_DIR]      = "direction",
    [META_OPTUUID]  = "optional_uuid",
    [META_BID]      = "block_id",
    [META_NONE]     = "-"
};

metadata * clone_metadata(metadata *meta) {
    if (!meta) return NULL;
    lh_create_num(metadata, newmeta, 32);
    memmove(newmeta, meta, 32*sizeof(metadata));
    int i;
    for(i=0; i<32; i++)
        if (newmeta[i].type == META_SLOT)
            clone_slot(&meta[i].slot, &newmeta[i].slot);
    return newmeta;
}

void free_metadata(metadata *meta) {
    if (!meta) return;
    int i;
    for(i=0; i<32; i++)
        if (meta[i].type == META_SLOT)
            clear_slot(&meta[i].slot);
    free(meta);
}

uint8_t * read_metadata(uint8_t *p, metadata **meta) {
    assert(meta);
    assert(!*meta);
    ssize_t mc = 0;

    // allocate a whole set of 32 values
    lh_alloc_num(*meta, 32);
    metadata *m = *meta;

    int i;
    int bool;

    // mark all entries as not present - we use the same 0xff value
    // that Mojang uses as terminator
    for(i=0; i<32; i++) m[i].type = META_NONE;

    while (1) {
        uint8_t key = read_char(p);
        if (key == 0xff) break; // terminator
        assert(key < 32);

        metadata *mm = &m[key];
        mm->key = key;
        mm->type = read_char(p);

        switch (mm->type) {
            case META_BYTE:     mm->b = read_char(p);    break;
            case META_VARINT:   mm->i = read_varint(p);  break;
            case META_FLOAT:    mm->f = read_float(p);   break;
            case META_STRING:   p = read_string(p,mm->str); break;
            case META_CHAT:     p = read_string(p,mm->chat); break; //VERIFY
            case META_SLOT:     p = read_slot(p,&mm->slot); break;
            case META_BOOL:     mm->bool = read_char(p);  break; //VERIFY
            case META_VEC3:     mm->fx=read_float(p);
                                mm->fy=read_float(p);
                                mm->fz=read_float(p); break;
            case META_POS:      mm->pos.p = read_long(p); break;
            case META_OPTPOS:   bool = read_char(p); //VERIFY
                                if (bool) {
                                    mm->pos.p = read_long(p);
                                }
                                else {
                                    mm->pos.p = (uint64_t)-1;
                                }
                                break;
            case META_DIR:      mm->dir = read_varint(p);  break;
            case META_OPTUUID:  bool = read_char(p); //VERIFY
                                if (bool) {
                                    memmove(mm->uuid,p,sizeof(uuid_t));
                                    p+=sizeof(uuid_t);
                                }
                                break;
            case META_BID:      mm->block = read_char(p); break; // note- block ID only, no meta
        }
    }

    return p;
}

uint8_t * write_metadata(uint8_t *w, metadata *meta) {
    assert(meta);

    int i,j;
    char bool;
    for (i=0; i<32; i++) {
        metadata *mm = meta+i;
        if (mm->type==META_NONE) continue;

        lh_write_char(w, mm->key);
        lh_write_char(w, mm->type);
        switch (mm->type) {
            case META_BYTE:     write_char(w, mm->b);    break;
            case META_VARINT:   write_varint(w, mm->i);  break;
            case META_FLOAT:    write_float(w, mm->f);   break;
            case META_STRING:   w = write_string(w, mm->str); break;
            case META_CHAT:     w = write_string(w, mm->chat); break; //VERIFY
            case META_SLOT:     w = write_slot(w, &mm->slot); break;
            case META_BOOL:     write_char(w, mm->bool);  break;
            case META_VEC3:     write_float(w, mm->fx);
                                write_float(w, mm->fy);
                                write_float(w, mm->fz); break;
            case META_POS:      write_long(w, mm->pos.p); break;
            case META_OPTPOS:   bool = (mm->pos.p == -1);
								write_char(w, bool);
                                if (bool) {
                                    write_long(w, mm->pos.p);
                                }
                                break;
            case META_DIR:      write_varint(w, mm->dir);  break;
            case META_OPTUUID:  bool = 0;
								for(j=0; j<16; j++)
									if (mm->uuid[j])
										bool = 1;
								write_char(w, bool);
								if (bool) {
									memmove(w, mm->uuid, sizeof(mm->uuid));
									w+=16;
								}
                                break;
            case META_BID:      write_char(w, mm->block); break;
        }
	}
	lh_write_char(w, 0xff);

    return w;
}

void dump_metadata(metadata *meta, EntityType et) {
    if (!meta) return;

    int i;
    for (i=0; i<32; i++) {
        metadata *mm = meta+i;
        if (mm->type==META_NONE) continue;

        printf("\n    ");

        const char * name = NULL;
        EntityType ett = et;
        while ((!name) && (ett!=IllegalEntityType)) {
            name = METANAME[ett][mm->key];
            ett = ENTITY_HIERARCHY[ett];
        }

        printf("%2d %-24s [%-6s] = ", mm->key, name?name:"Unknown",METATYPES[mm->type]);
        switch (mm->type) {
            case META_BYTE:     printf("%d",  mm->b);   break;
            case META_VARINT:   printf("%d",  mm->i);   break;
            case META_FLOAT:    printf("%.1f",mm->f);   break;
            case META_STRING:
            case META_CHAT:     printf("\"%s\"", mm->str); break;
            case META_SLOT:     dump_slot(&mm->slot); break;
            case META_BOOL:     printf("%s", mm->bool?"true":"false"); break; //VERIFY
            case META_VEC3:     printf("%.1f,%.1f,%.1f", mm->fx, mm->fy, mm->fz); break;
            case META_POS:
            case META_OPTPOS:   printf("%d,%d,%d", mm->pos.x, mm->pos.y, mm->pos.z); break;
            case META_DIR:      printf("%d",mm->dir);  break;
            case META_OPTUUID:  hexprint(mm->uuid, sizeof(uuid_t));
            case META_BID:      printf("%2x (%d)", mm->block, mm->block); //TODO: print material name
            default:            printf("<unknown type>"); break;
        }
    }
}

