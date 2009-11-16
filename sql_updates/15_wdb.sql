UPDATE `questcache` SET `QuestLevel`='999' WHERE `QuestLevel`='4294967295';
ALTER TABLE `questcache` CHANGE  `QuestLevel` `QuestLevel` int(12) NOT NULL DEFAULT '0';
UPDATE `questcache` SET `QuestLevel`='-1' WHERE `QuestLevel`='999';