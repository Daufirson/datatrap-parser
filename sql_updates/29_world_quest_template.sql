-- Must be changed because negative values are possible!
ALTER TABLE `quest_template` CHANGE `RewSpellCast` `RewSpellCast` int(11) SIGNED NOT NULL DEFAULT 0;
